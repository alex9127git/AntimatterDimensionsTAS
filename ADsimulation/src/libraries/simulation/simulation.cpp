#include "instructions.h"
#include "simulation.h"
#include "simconst.h"
#include "ioutils.h"
#include "errors.h"
#include "purge.h"
#include <algorithm>
#include <iostream>
#include <cstdint>
#include <string>
#include <format>
#include <cmath>
#include <set>
using namespace std;


Iteration::Iteration(
    const Progress& _progress, bool _verbose, const Strategies& _projectedSacrifices
)   :   progress(_progress),
        verbose(_verbose),
        projectedSacrifices(_projectedSacrifices)
    {}

void Iteration::appendNewStates() {
    if (!newNodes.empty()) adds++;
    states += newNodes.size();
    for (SearchNode& node : newNodes) nodes.push_back(move(node));
    newNodes.clear();
}

void Iteration::checkProgress() {
    stats.resetCheckTimer();
    bool doRender = updateTimer.peek() > RENDER_INTERVAL_MS && verbose;
    if (doRender) updateTimer.reset();
    bool anyFinished = false;
    #pragma omp parallel for if(nodes.size() > PARALLEL_THRESHOLD) reduction(||:anyFinished)
    for (SearchNode& node : nodes) {
        node.runNextInstructions();
        anyFinished = anyFinished || progress.isFinished(node.game());
    }
    if (anyFinished) {
        if (verbose && !isFinished) {
            progress.finish();
            renderProgressDisplay();
            stats.printStats(maxStates);
        }
        isFinished = true;
    }
    if (!isFinished && doRender) {
        SearchNode* best = &nodes[0];
        for (SearchNode& node : nodes) {
            if (node.game().antimatterThisReset() > best->game().antimatterThisReset()) {
                best = &node;
            }
        }
        progress.update(best->game());
        renderProgressDisplay();
    }
    stats.updateCheckTime();
}

void Iteration::tickAllStates() {
    stats.resetTickTimer();
    #pragma omp parallel for if(nodes.size() > PARALLEL_THRESHOLD) 
    for (SearchNode& node : nodes) {
        node.tick(0.033);
        node.runNextPurchases();
    }
    stats.updateTickTime();
}

void Iteration::purgeObsoleteStates() {
    if (adds >= 300 || states >= statesAfterPurge * 1.5) {
        stats.resetPurgeTimer();
        adds = 0;
        statesBeforePurge = nodes.size();
        if ((size_t) statesBeforePurge > MAX_STATES) {
            throw StateLimitExceeded(format("Iteration stopped: more than {} states", MAX_STATES));
        }
        maxStates = max(maxStates, statesBeforePurge);
        purge(nodes, [this](int purged, double progressFraction) {
            if (!verbose || updateTimer.peek() < RENDER_INTERVAL_MS) return;
            updateTimer.reset();
            statesPurged = purged;
            purgeProgress = progressFraction;
            renderProgressDisplay(true);
        });
        statesAfterPurge = nodes.size();
        states = statesAfterPurge;
        stats.updatePurgeTime();
        renderProgressDisplay();
    }
}

void Iteration::renderProgressDisplay(bool purgeOngoing) {
    if (!(verbose && isTerminal(stderr))) return;
    vector<string> lines = {
        "Iteration progress:",
        renderProgressBar(progress.getFraction()),
        format("Running for {}; simulated {} game ticks", renderTime(stats.getTimeRunning()), ticks),
        format("{} states currently alive; peak {} states", states, maxStates),
    };
    if (purgeOngoing) {
        lines.push_back("Purge progress:");
        lines.push_back(renderProgressBar(purgeProgress));
        lines.push_back(format("Purged {} states out of {}", statesPurged, statesBeforePurge));
    } else {
        lines.push_back(format("Last purge: {} states -> {} states", statesBeforePurge, statesAfterPurge));
        lines.push_back("");
        lines.push_back("");
    }
    if (linesRendered > 0) cerr << "\033[" << linesRendered << "F";
    cerr << "\033[J";
    for (const string& line : lines) cerr << line << '\n';
    linesRendered = lines.size();
    cerr << flush;
}

// Adds all sacrifices from list, from "fromIndex" to the first sacrifice that has a window.
// If no such sacrifice exists, all remaining sacrifices are added.
// The sacrifice with window is added as a branching sacrifice
void Iteration::prepareSacrifices(SearchNode& node, size_t routeIndex, size_t fromIndex) const {
    const vector<double>& plan = projectedSacrifices[routeIndex];
    for (size_t index = fromIndex; index < plan.size(); index++) {
        if (hasSacrificeWindow(routeIndex, index)) {
            node.setNextSacBranching(
                Decimal(1, plan[index]) / sacrificeWindow(routeIndex, index).boundLow);
            return;
        }
        node.addInstructions({Instruction::Sacrifice, plan[index]});
    }
}

void Iteration::addSacrificeWindowBranches() {
    for (SearchNode& node : nodes) {
        size_t routeIndex = node.getProjRouteIndex();
        const vector<double>& plan = projectedSacrifices[routeIndex];
        size_t sacrificesDone = node.game().sacrifice().getSacrificesDone();
        if (sacrificesDone >= plan.size() || !hasSacrificeWindow(routeIndex, sacrificesDone)) continue;

        if (node.game().canSacrifice() && node.canSacBranch()) {
            Decimal currSac = node.game().getBoostAfterSacrifice();
            Decimal prevSac = node.game().sacrifice().getSacrificeBonus() * 
                max(node.game().sacrifice().sacrificeIncrease(node.getPrevAD1()), DC::D1);
            Decimal target = (currSac + prevSac) / DC::D2;
            double diff = currSac.log10value() - prevSac.log10value();
            SacrificeWindow sacWindow = sacrificeWindow(node.getProjRouteIndex(), sacrificesDone);
            if (currSac > Decimal(1, 
                projectedSacrifices[node.getProjRouteIndex()][sacrificesDone]
            ) * sacWindow.boundHigh) {
                scheduleSacrifice(node, target, diff);
            } else {
                SearchNode newNode = node;
                scheduleSacrifice(newNode, target, diff);
                Decimal nextSac = currSac * sacWindow.precision;
                node.setNextSacBranching(nextSac);
                newNodes.push_back(move(newNode));
            }
        }
    }
}

void Iteration::scheduleSacrifice(SearchNode& node, const Decimal& target, double diff) const {
    node.addInstructions({Instruction::Sacrifice, target.log10value()});
    node.appendSacrificeDiff(diff);
    prepareSacrifices(node, node.getProjRouteIndex(), node.game().sacrifice().getSacrificesDone() + 1);
}

vector<SearchNode> Iteration::run() {
    while (true) {
        stats.resetBranchTimer();
        addBranches();
        stats.updateBranchTime();
        checkProgress();
        if (isFinished) break;
        tickAllStates();
        purgeObsoleteStates();
        ticks++;
        if (ticks >= MAX_TICKS) {
            renderProgressDisplay();
            throw TickLimitExceeded(format("Strategy search stopped: took more than {} ticks", MAX_TICKS));
        }
    }
    vector<SearchNode> finishedStates;
    for (SearchNode& node : nodes) {
        if (progress.isFinished(node.game())) {
            finishedStates.push_back(move(node));
        }
    }
    return finishedStates;
}

PurchaseIteration::PurchaseIteration(
    GameState st, const Progress& progress,
    const Strategies& sacrificeStrategies, bool verbose,
    const Strategies& _sacrificeDiffs, int _windowIndex
)   :   Iteration(progress, verbose, sacrificeStrategies),
        sacrificeDiffs(_sacrificeDiffs),
        windowIndex(_windowIndex)
{
    SearchNode node = SearchNode(st);
    // If Konami code exploit isn't used, assume this is the starting game state 
    // and add some default instructions.
    // Otherwise, buy the first dimension - this is likely an unavoidable purchase.
    if (node.game().canUseKonami()) {
        node.addInstructions({Instruction::bulk(1, 1), Instruction::KonamiCode, Instruction::bulk(1, 3)});
    } else {
        node.addInstructions({Instruction::bulk(1, 1)});
    }
    node.runNextInstructions();
    for (size_t i = 0; i < sacrificeStrategies.size(); i++) {
        nodes.push_back(node);
        if (windowIndex >= 0) nodes[i].initProjRoute(i);
        prepareSacrifices(nodes[i], i, 0);
    }
    states = nodes.size();
}

SacrificeWindow PurchaseIteration::sacrificeWindow(size_t routeIndex, size_t sacrificeIndex) const {
    const vector<double>& plan = projectedSacrifices[routeIndex];
    double diff = sacrificeDiffs[routeIndex][sacrificeIndex];
    double maxDistBelow = sacrificeIndex > 0 
        ? plan[sacrificeIndex] - plan[sacrificeIndex - 1]
        : plan[sacrificeIndex];
    double maxDistAbove = sacrificeIndex + 1 < plan.size() 
        ? plan[sacrificeIndex + 1] - plan[sacrificeIndex] 
        : maxDistBelow;
    double boundLowLog = min({WINDOW_TICKS * diff, log10(1.01), maxDistBelow});
    double boundHighLog = min({WINDOW_TICKS * diff, log10(1.1), maxDistAbove});
    Decimal precision = min(Decimal(1, STEP_TICKS * diff), maxPrecision);
    return {Decimal(1, boundLowLog), Decimal(1, boundHighLog), precision};
}

// Checks if the sacrifice with the given index has a window (is able to be moved) in this route.
// Last condition is a check that prevents branching if the sacrifice diff is not written
bool PurchaseIteration::hasSacrificeWindow(size_t routeIndex, size_t sacrificeIndex) const {
    size_t planSize = projectedSacrifices[routeIndex].size();
    return windowIndex >= 0 && (size_t) windowIndex < planSize
        && sacrificeIndex == planSize - 1 - windowIndex
        && sacrificeIndex < sacrificeDiffs[routeIndex].size();
}

void PurchaseIteration::addBranches() {
    size_t begin = 0;
    size_t end = nodes.size();
    while (begin < end) {
        for (size_t i = begin; i < end; i++) {
            nodes[i].runNextPurchases();
            computePurchaseBranches(nodes[i], newNodes);
        }
        appendNewStates();
        begin = end;
        end = nodes.size();
    }
    if (windowIndex >= 0) {
        addSacrificeWindowBranches();
        appendNewStates();
    }
}

void PurchaseIteration::computePurchaseBranches(SearchNode& node, vector<SearchNode>& branches) {
    while (node.canBranch() && !node.queue().hasNextPurchase()) {
        vector<uint16_t> variants = node.getPurchaseBranches();
        for (size_t i = 1; i < variants.size(); i++) {
            SearchNode newNode = node;
            newNode.addInstructions({(double) variants[i]});
            newNode.runNextPurchases();
            branches.push_back(move(newNode));
        }
        node.addInstructions({(double) variants[0]});
        node.runNextPurchases();
    }
}

ApproxSacrificeIteration::ApproxSacrificeIteration(
    GameState st, const Progress& progress,
    const Strategies& purchaseStrategies, bool verbose
)   :   Iteration(progress, verbose)
{
    SearchNode node = SearchNode(st);
    for (size_t i = 0; i < purchaseStrategies.size(); i++) {
        nodes.push_back(node);
        nodes[i].addInstructions(purchaseStrategies[i]);
    }
    states = nodes.size();
}

void ApproxSacrificeIteration::addBranches() {
    for (SearchNode& node : nodes) {
        if (node.game().canSacrifice() && node.canSacBranch()) {
            SearchNode newNode = node;
            Decimal currSac = newNode.game().getBoostAfterSacrifice();
            Decimal prevSac = newNode.game().sacrifice().getSacrificeBonus() * 
                max(newNode.game().sacrifice().sacrificeIncrease(newNode.getPrevAD1()), DC::D1);
            Decimal target = (currSac + prevSac) / DC::D2;
            newNode.addInstructions({Instruction::Sacrifice, target.log10value()});
            Decimal nextSac = currSac * precision;
            newNode.setNextSacBranching(DC::D1);
            node.setNextSacBranching(nextSac);
            newNodes.push_back(move(newNode));
        }
    }
    appendNewStates();
}

SacrificeRefinementIteration::SacrificeRefinementIteration(
    GameState st, const Progress& progress,
    const Strategies& purchaseStrategies, 
    const Strategies& sacrificeStrategies, bool verbose
)   :   Iteration(progress, verbose, sacrificeStrategies)
{
    SearchNode node = SearchNode(st);
    for (size_t i = 0; i < purchaseStrategies.size(); i++) {
        nodes.push_back(node);
        nodes[i].addInstructions(purchaseStrategies[i]);
        nodes[i].initProjRoute(i);
        prepareSacrifices(nodes[i], i, 0);
    }
    states = nodes.size();
}

void SacrificeRefinementIteration::addBranches() {
    addSacrificeWindowBranches();
    appendNewStates();
}

namespace SimulationHelpers {
    struct RouteStrategies {
        Strategies purchases = {{}};
        Strategies sacrifices = {{}};
        Strategies sacrificeDiffs = {{}};
    };

    struct SegmentSearch {
        const GameState& st;
        const Progress& progress;
        bool verbose;
        int64_t startTime = st.realTimePlayed();
        int iterationCounter = 0;
        GameInstance best = GameInstance();
        int64_t bestTime = INT64_MAX;
        RouteStrategies bestStrategies = {};
    };

    // Collects purchase strategies. Removes duplicates if such exist
    void collectPurchaseStrategies(const vector<SearchNode>& nodes, Strategies& purchaseStrategies) {
        purchaseStrategies.clear();
        set<vector<double>> seenStrategies;
        for (const SearchNode& node : nodes) {
            vector<double> strat = node.queue().getCompletedPurchases();
            if (seenStrategies.insert(strat).second) purchaseStrategies.push_back(strat);
        }
    }

    // Collects sacrifice strategies. Removes duplicates if such exist
    void collectSacrificeStrategies(const vector<SearchNode>& nodes, Strategies& sacrificeStrategies,
                                    Strategies& sacrificeDiffs)
    {
        sacrificeStrategies.clear();
        sacrificeDiffs.clear();
        set<vector<double>> seenStrategies;
        for (const SearchNode& node : nodes) {
            vector<double> strat = node.queue().getCompletedSacrifices();
            if (seenStrategies.insert(strat).second) {
                sacrificeStrategies.push_back(strat);
                sacrificeDiffs.push_back(node.getSacrificeDiffs());
            }
        }
    }

    // Collects strategy pairs (purchase + sacrifice). Removes duplicates if such exist
    void collectStrategyPairs(const vector<SearchNode>& nodes, Strategies& purchaseStrategies,
                            Strategies& sacrificeStrategies, bool addFinalTarget) 
    {
        purchaseStrategies.clear();
        sacrificeStrategies.clear();
        set<pair<vector<double>, vector<double>>> seenStrategyPairs;
        for (const SearchNode& node : nodes) {
            vector<double> purchStrat = node.queue().getCompletedPurchases();
            vector<double> sacStrat = node.queue().getCompletedSacrifices();
            if (addFinalTarget) {
                sacStrat.push_back(
                    (node.game().getBoostAfterSacrifice() * DC::D2).log10value()
                );
            }
            if (seenStrategyPairs.insert(pair(purchStrat, sacStrat)).second) {
                purchaseStrategies.push_back(purchStrat);
                sacrificeStrategies.push_back(sacStrat);
            }
        }
    }

    void startIteration(SegmentSearch& search) {
        search.iterationCounter++;
        cout << "============================================================" << endl;
        cout << "Running iteration " << search.iterationCounter << endl;
    }

    void printIterationResults(const SegmentSearch& search, int64_t iterationTime, size_t strategiesSize) {
        cout << "Iteration " << search.iterationCounter << " results:" << endl;
        cout << "Segment time: " << renderTime(iterationTime - search.startTime) << endl;
        cout << "Cumulative time: " << renderTime(iterationTime) << endl;
        cout << "Winning strategies left: " << strategiesSize << endl;
    }

    void refineSacrifices(
        const SegmentSearch& search, vector<SearchNode>& resultNodes, RouteStrategies& strategies
    ) {
        cout << "============================================================" << endl;
        cout << "Refining sacrifices" << endl;
        resultNodes = SacrificeRefinementIteration(
            search.st, search.progress, strategies.purchases, strategies.sacrifices, search.verbose).run();
        collectSacrificeStrategies(resultNodes, strategies.sacrifices, strategies.sacrificeDiffs);
    }

    void updateBest(
        SegmentSearch& search, const GameInstance& iterationBest, 
        int64_t iterationTime, const RouteStrategies& strategies
    ) {
        search.best = iterationBest;
        search.bestTime = iterationTime;
        search.bestStrategies = strategies;
    }

    void initialIterationCycle(SegmentSearch& search, bool sacrificeAllowed) {
        vector<SearchNode> resultNodes;

        int64_t prevIterationTime = INT64_MAX;
        int64_t iterationTime = INT64_MAX;
        RouteStrategies strategies;

        set<Strategies> seenPurchaseInputs, seenSacrificeInputs;

        bool isImproving = true;
        bool repeated = false;
        bool isPurchasing = false;

        // Switch between purchases and sacrifices until stuck in a local optimum
        do {
            startIteration(search);
            isPurchasing = search.iterationCounter % 2 == 1;
            if (isPurchasing) {
                prevIterationTime = iterationTime;
                cout << "Finding purchases" << endl;
                resultNodes = PurchaseIteration(
                    search.st, search.progress, strategies.sacrifices, search.verbose).run();
                collectPurchaseStrategies(resultNodes, strategies.purchases);
                if (!seenPurchaseInputs.insert(strategies.purchases).second) repeated = true;
            } else {
                cout << "Finding sacrifices" << endl;
                resultNodes = ApproxSacrificeIteration(
                    search.st, search.progress, strategies.purchases, search.verbose).run();
                collectStrategyPairs(resultNodes, strategies.purchases, strategies.sacrifices, true);
                refineSacrifices(search, resultNodes, strategies);
                if (!seenSacrificeInputs.insert(strategies.sacrifices).second) repeated = true;
            }
            const GameInstance& iterationBest = resultNodes[0].gameInstance();
            iterationTime = iterationBest.game().realTimePlayed();
            if (!isPurchasing) isImproving = iterationTime < prevIterationTime;
            if (iterationTime < search.bestTime) updateBest(search, iterationBest, iterationTime, strategies);
            printIterationResults(search, iterationTime,
                isPurchasing ? strategies.purchases.size() : strategies.sacrifices.size());
        } while (sacrificeAllowed && isImproving && !repeated);
    }

    void polishSacrifices(SegmentSearch& search) {
        vector<SearchNode> resultNodes;

        int64_t iterationTime = INT64_MAX;
        RouteStrategies strategies;

        size_t maxPlanSize = 0;
        for (const vector<double>& plan : search.bestStrategies.sacrifices) maxPlanSize = max(maxPlanSize, plan.size());
        size_t iterationsLeft = maxPlanSize;
        int windowIndex = -1;

        // Attempt all purchase iterations again, but with ability to move sacrifices; repeat for each sacrifice
        while (iterationsLeft > 0) {
            windowIndex = (windowIndex + 1) % maxPlanSize;
            startIteration(search);
            cout << "Polishing sacrifice, " << iterationsLeft << " iterations left" << endl;
            cout << "Optimizing sacrifice #" << maxPlanSize - windowIndex << endl;
            iterationsLeft--;
            try {
                resultNodes = PurchaseIteration(search.st, search.progress, search.bestStrategies.sacrifices, 
                    search.verbose, search.bestStrategies.sacrificeDiffs, windowIndex).run();
            } catch (const StateLimitExceeded& e) {
                cout << e.what() << ", skipped" << endl;
                continue;
            }
            collectStrategyPairs(resultNodes, strategies.purchases, strategies.sacrifices, false);
            refineSacrifices(search, resultNodes, strategies);
            const GameInstance& iterationBest = resultNodes[0].gameInstance();
            iterationTime = iterationBest.game().realTimePlayed();
            if (iterationTime < search.bestTime) {
                cout << "Recent iteration found better time; ";
                cout << maxPlanSize - iterationsLeft << " more iterations added" << endl;
                iterationsLeft = maxPlanSize;
                updateBest(search, iterationBest, iterationTime, strategies);
            }
            printIterationResults(search, iterationTime, strategies.sacrifices.size());
        }
    }
}
using namespace SimulationHelpers;

GameInstance runDimboost(GameState st, bool useSacrifice, bool verbose) {
    GameInstance result;
    if (st.antimatterGalaxies() == 1 && st.dimensionBoosts() == 9) {
        result = runUntil(
            st, Progress::untilAch23(), useSacrifice, verbose
        );
    } else {
        result = runUntil(
            st, Progress::untilDimboost(), useSacrifice, verbose
        );
    }
    result.addInstructions({Instruction::DimensionBoost});
    result.runNextInstructions();
    return result;
}

GameInstance runGalaxy(GameState st, bool useSacrifice, bool verbose) {
    GameInstance result = runUntil(
        st, Progress::untilGalaxy(), useSacrifice, verbose
    );
    result.addInstructions({Instruction::AntimatterGalaxy});
    result.runNextInstructions();
    return result;
}

GameInstance runUntil(
    GameState st, const Progress& progress, bool useSacrifice, bool verbose
) {
    SegmentSearch search = {st, progress, verbose};
    initialIterationCycle(search, st.dimensionBoosts() >= 5 && useSacrifice);
    polishSacrifices(search);
    search.best.clearAllInstructions();
    return search.best;
}
