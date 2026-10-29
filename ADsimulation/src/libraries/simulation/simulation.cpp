#include "instructions.h"
#include "simulation.h"
#include "constants.h"
#include "ioutils.h"
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string>
#include <format>
using namespace std;


GameInstance runDimboost(GameState st, bool useSacrifice, bool verbose) {
    GameInstance result = runUntil(
        st, Progress::untilDimboost(), useSacrifice, verbose
    );
    result.addInstructions({100});
    result.runNextInstructions();
    return result;
}

GameInstance runUntil(
    GameState st, const Progress& progress, bool useSacrifice, bool verbose
) {
    vector<SearchNode> resultNodes;
    GameInstance best;
    int64_t currentIterationTime = INT64_MAX;
    int64_t iterationTime = INT64_MAX;
    int64_t startTime = st.realTimePlayed();
    vector<vector<double>> purchaseStrategies = {{}};
    vector<vector<double>> sacrificeStrategies = {{}};
    int iterationCounter = 0;
    do {
        iterationCounter++;
        currentIterationTime = iterationTime;
        cout << "============================================================" << endl;
        cout << "Running iteration " << iterationCounter << endl;
        switch (iterationCounter % 2) {
            case 1:
                resultNodes = PurchaseIteration(st, progress, sacrificeStrategies, verbose).run();
                purchaseStrategies.clear();
                for (SearchNode& node : resultNodes) {
                    purchaseStrategies.push_back(node.queue().getCompletedPurchases());
                }
                break;
            case 0:
                resultNodes = ApproxSacrificeIteration(st, progress, purchaseStrategies, verbose).run();
                purchaseStrategies.clear();
                sacrificeStrategies.clear();
                for (SearchNode& node : resultNodes) {
                    purchaseStrategies.push_back(node.queue().getCompletedPurchases());
                    vector<double> sacStrat = node.queue().getCompletedSacrifices();
                    sacStrat.push_back(
                        (node.game().sacrifice().getSacrificeBonus() * 
                         node.game().nextSacrificeBoost() * DC::D2).log10value()
                    );
                    sacrificeStrategies.push_back(sacStrat);
                }
                cout << "============================================================" << endl;
                cout << "Optimizing sacrifices" << endl;
                resultNodes = PreciseSacrificeIteration(
                    st, progress, purchaseStrategies, sacrificeStrategies, verbose).run();
                sacrificeStrategies.clear();
                for (SearchNode& node : resultNodes) {
                    sacrificeStrategies.push_back(node.queue().getCompletedSacrifices());
                }
                break;
        }
        const GameInstance& iterationBest = resultNodes[0].gameInstance();
        iterationTime = iterationBest.game().realTimePlayed();
        if (iterationTime < currentIterationTime) best = iterationBest;
        if (verbose) {
            cout << "Iteration " << iterationCounter << " results:" << endl;
            cout << "Segment time: " << renderTime(iterationTime - startTime) << endl;
            cout << "Cumulative time: " << renderTime(iterationTime) << endl;
            cout << "Winning strategies left: " << purchaseStrategies.size() << endl;
        }
    } while (st.dimensionBoosts() >= 5 && useSacrifice && iterationTime < currentIterationTime);
    best.clearAllInstructions();
    return best;
}

Progress::Progress(function<bool(const GameState&)> stopFunc, function<double(const GameState&)> progressFunc)
    :   stopCondition(stopFunc),
        progressFunction(progressFunc) 
    {};

Iteration::Iteration(
    const Progress& _progress, bool _verbose
)   :   progress(_progress),
        verbose(_verbose)
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
    if (doRender) updateTimer.silentReset();
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
        node.runNextInstructions();
    }
    stats.updateTickTime();
}

void Iteration::purgeObsoleteStates() {
    if (adds >= 300 || states >= statesAfterPurge * 1.5) {
        stats.resetPurgeTimer();
        adds = 0;
        statesBeforePurge = nodes.size();
        maxStates = max(maxStates, statesBeforePurge);
        purge(nodes, [this](int purged, double progressFraction) {
            if (!verbose || updateTimer.peek() < RENDER_INTERVAL_MS) return;
            updateTimer.silentReset();
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
    const vector<vector<double>>& sacrificeStrategies, bool verbose
)   :   Iteration(progress, verbose)
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
    for (int i = 0; i < sacrificeStrategies.size(); i++) {
        nodes.push_back(node);
        for (double instruction : sacrificeStrategies[i]) {
            nodes[i].addInstructions({108, instruction});
        }
    }
    states = nodes.size();
}

void PurchaseIteration::addBranches() {
    size_t begin = 0;
    size_t end = nodes.size();
    while (begin < end) {
        for (size_t i = begin; i < end; i++) {
            nodes[i].runNextInstructions();
            computePurchaseBranches(nodes[i], newNodes);
        }
        appendNewStates();
        begin = end;
        end = nodes.size();
    }
}

ApproxSacrificeIteration::ApproxSacrificeIteration(
    GameState st, const Progress& progress,
    const vector<vector<double>>& purchaseStrategies, bool verbose
)   :   Iteration(progress, verbose)
{
    SearchNode node = SearchNode(st);
    for (int i = 0; i < purchaseStrategies.size(); i++) {
        nodes.push_back(node);
        nodes[i].addInstructions(purchaseStrategies[i]);
    }
    states = nodes.size();
}

void ApproxSacrificeIteration::addBranches() {
    for (SearchNode& node : nodes) {
        if (node.game().canSacrifice() && node.canSacBranch()) {
            SearchNode newNode = node;
            Decimal currSac = newNode.game().sacrifice().getSacrificeBonus() * newNode.game().nextSacrificeBoost();
            Decimal prevSac = newNode.game().sacrifice().getSacrificeBonus() * 
                Decimal::max(newNode.game().sacrifice().nextSacrificeBoost(newNode.getPrevAD1()), DC::D1);
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

PreciseSacrificeIteration::PreciseSacrificeIteration(
    GameState st, const Progress& progress,
    const vector<vector<double>>& purchaseStrategies, 
    const vector<vector<double>>& sacrificeStrategies, bool verbose
)   :   Iteration(progress, verbose)
{
    SearchNode node = SearchNode(st);
    projectedSacrifices = sacrificeStrategies[0];
    for (int i = 0; i < purchaseStrategies.size(); i++) {
        nodes.push_back(node);
        nodes[i].addInstructions(purchaseStrategies[i]);
        nodes[i].setNextSacBranching(Decimal(1, projectedSacrifices[0]) / tolerance);
    }
    states = nodes.size();
}

void PreciseSacrificeIteration::addBranches() {
    for (SearchNode& node : nodes) {
        if (node.game().canSacrifice() && node.canSacBranch()) {
            Decimal currSac = node.game().sacrifice().getSacrificeBonus() * node.game().nextSacrificeBoost();
            Decimal prevSac = node.game().sacrifice().getSacrificeBonus() * 
                Decimal::max(node.game().sacrifice().nextSacrificeBoost(node.getPrevAD1()), DC::D1);
            Decimal target = (currSac + prevSac) / DC::D2;
            if (currSac > Decimal(1, projectedSacrifices[node.game().sacrifice().getSacrificesDone()]) * tolerance) {
                scheduleSacrifice(node, target);
            } else {
                SearchNode newNode = node;
                scheduleSacrifice(newNode, target);
                Decimal nextSac = currSac * precision;
                node.setNextSacBranching(nextSac);
                newNodes.push_back(move(newNode));
            }
        }
    }
    appendNewStates();
}

void PreciseSacrificeIteration::scheduleSacrifice(SearchNode& node, const Decimal& target) const {
    node.addInstructions({Instruction::Sacrifice, target.log10value()});
    node.setNextSacBranching(
        Decimal(1, projectedSacrifices[node.game().sacrifice().getSacrificesDone() + 1]) / tolerance);
}

void computePurchaseBranches(SearchNode& node, vector<SearchNode>& branches) {
    while (node.canBranch() && !node.queue().hasNextPurchase()) {
        vector<uint16_t> variants = node.getPurchaseBranches();
        for (int i = 1; i < variants.size(); i++) {
            SearchNode newNode = node;
            newNode.addInstructions({(double) variants[i]});
            newNode.runNextInstructions();
            branches.push_back(move(newNode));
        }
        node.addInstructions({(double) variants[0]});
        node.runNextInstructions();
    }
}

bool compare(const double* st1, const double* st2, size_t width) {
    // Compare the two game states together.
    // Returns true if all st1's values are bigger or equal than that of st2,
    // and at least one value is strictly bigger.
    // Otherwise, return false.
    bool strictlyBetter = false;
    for (size_t i = 0; i < width; i++) {
        if (st1[i] < st2[i]) return false;
        if (st1[i] > st2[i]) strictlyBetter = true;
    }
    return strictlyBetter;
}

void purge(vector<SearchNode>& nodes, const function<void(int, double)>& onProgress) {
    int size = nodes.size();
    vector<int> indices(size);
    iota(indices.begin(), indices.end(), 0);
    sort(indices.begin(), indices.end(), [&nodes](int i, int j) {
        return nodes[i].game().AD()[1].getAmount() > nodes[j].game().AD()[1].getAmount();
    });
    vector<double> values;
    int width = 0;
    for (int i = 0; i < size; i++) {
        const GameState& gst = nodes[indices[i]].game();
        values.push_back(gst.antimatter().log10value());
        for (int d = 1; d <= 8; d++) {
            values.push_back(gst.AD()[d].getAmount().log10value());
            values.push_back(gst.AD()[d].getPurchases());
        }
        values.push_back(gst.sacrifice().getSacrificeBonus().log10value());
        values.push_back(gst.tickspeed().getPurchases());
        values.push_back(gst.tickspeed().perSecond().log10value());
        values.push_back(gst.getAchievementBonus().log10value());
        if (i == 0) {
            width = values.size();
            values.reserve(size * width);
        }
    }
    vector<char> removedCopy(size);
    int statesPurged = 0;
    double iterationsTotal = (double) (size) * (size - 1) / 2.0;
    #pragma omp parallel if(size > PARALLEL_THRESHOLD)
    for (int i = 0; i < size; i++) {
        #pragma omp single
        if (onProgress) {
            double iterationsDone = (double) i * size - (double) i * (i + 1) / 2;
            onProgress(statesPurged, iterationsDone / iterationsTotal);
        }
        if (removedCopy[i] == 1) continue;
        #pragma omp for reduction(+:statesPurged)
        for (int j = i + 1; j < size; j++) {
            if (removedCopy[j] == 1) continue;
            if (compare(&values[i * width], &values[j * width], width)) {
                removedCopy[j] = 1;
                statesPurged++;
            }
        }
    }
    vector<char> removedOrig(size);
    for (int i = 0; i < size; i++) {
        removedOrig[indices[i]] = removedCopy[i];
    }
    vector<SearchNode> result;
    int i = 0;
    for (auto stIter = nodes.begin(); stIter != nodes.end(); stIter++) {
        if (removedOrig[i] == 0) {
            result.push_back(move(*stIter));
        }
        i++;
    }
    nodes.swap(result);
}
