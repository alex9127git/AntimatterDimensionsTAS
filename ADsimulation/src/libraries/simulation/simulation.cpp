#include "simulation.h"
#include "../ioutils/ioutils.h"
#include "../constants/constants.h"
#include <cstdint>
#include <vector>
#include <numeric>
#include <string>
#include <format>
using namespace std;


GameState runDimboost(GameState st, bool useSacrifice, bool verbose) {
    GameState result = runUntil(
        st, Progress::untilDimboost(), useSacrifice, verbose
    );
    result.addInstructions({100});
    result.runNextInstructions();
    return result;
}

GameState runUntil(
    GameState st, const Progress& progress, bool useSacrifice, bool verbose
) {
    vector<GameState> gameStates;
    GameState bestState;
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
                gameStates = PurchaseIteration(st, progress, sacrificeStrategies, verbose).run();
                purchaseStrategies.clear();
                for (GameState& gst : gameStates) {
                    purchaseStrategies.push_back(gst.queue().getCompletedPurchases());
                }
                break;
            case 0:
                gameStates = ApproxSacrificeIteration(st, progress, purchaseStrategies, verbose).run();
                purchaseStrategies.clear();
                sacrificeStrategies.clear();
                for (GameState& gst : gameStates) {
                    purchaseStrategies.push_back(gst.queue().getCompletedPurchases());
                    vector<double> sacStrat = gst.queue().getCompletedSacrifices();
                    sacStrat.push_back(
                        (gst.sacrifice().getSacrificeBonus() * gst.nextSacrificeBoost() * DC::D2).log10value()
                    );
                    sacrificeStrategies.push_back(sacStrat);
                }
                cout << "============================================================" << endl;
                cout << "Optimizing sacrifices" << endl;
                gameStates = PreciseSacrificeIteration(
                    st, progress, purchaseStrategies, sacrificeStrategies, verbose).run();
                sacrificeStrategies.clear();
                for (GameState& gst : gameStates) {
                    sacrificeStrategies.push_back(gst.queue().getCompletedSacrifices());
                }
                break;
        }
        const GameState& iterationBestState = gameStates[0];
        iterationTime = iterationBestState.realTimePlayed();
        if (iterationTime < currentIterationTime) bestState = iterationBestState;
        if (verbose) {
            cout << "Iteration " << iterationCounter << " results:" << endl;
            cout << "Segment time: " << renderTime(iterationTime - startTime) << endl;
            cout << "Cumulative time: " << renderTime(iterationTime) << endl;
            cout << "Winning strategies left: " << purchaseStrategies.size() << endl;
        }
    } while (st.dimensionBoosts() >= 5 && useSacrifice && iterationTime < currentIterationTime);
    bestState.clearAllInstructions();
    return bestState;
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
    if (!newGameStates.empty()) adds++;
    states += newGameStates.size();
    for (GameState& gst : newGameStates) gameStates.push_back(move(gst));
    newGameStates.clear();
}

void Iteration::checkProgress() {
    stats.resetCheckTimer();
    bool doRender = updateTimer.peek() > RENDER_INTERVAL_MS && verbose;
    if (doRender) updateTimer.silentReset();
    bool anyFinished = false;
    #pragma omp parallel for if(gameStates.size() > PARALLEL_THRESHOLD) reduction(||:anyFinished)
    for (GameState& gst : gameStates) {
        gst.runNextInstructions();
        anyFinished = anyFinished || progress.isFinished(gst);
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
        GameState* bestState = &gameStates[0];
        for (GameState& gst : gameStates) {
            if (gst.antimatterThisReset() > bestState->antimatterThisReset()) {
                bestState = &gst;
            }
        }
        progress.update(*bestState);
        renderProgressDisplay();
    }
    stats.updateCheckTime();
}

void Iteration::tickAllStates() {
    stats.resetTickTimer();
    #pragma omp parallel for if(gameStates.size() > PARALLEL_THRESHOLD) 
    for (GameState& gst : gameStates) {
        gst.tick(0.033);
        gst.runNextInstructions();
    }
    stats.updateTickTime();
}

void Iteration::purgeObsoleteStates() {
    if (adds >= 300 || states >= statesAfterPurge * 1.5) {
        stats.resetPurgeTimer();
        adds = 0;
        statesBeforePurge = gameStates.size();
        maxStates = max(maxStates, statesBeforePurge);
        purge(gameStates, [this](int purged, double progressFraction) {
            if (!verbose || updateTimer.peek() < RENDER_INTERVAL_MS) return;
            updateTimer.silentReset();
            statesPurged = purged;
            purgeProgress = progressFraction;
            renderProgressDisplay(true);
        });
        statesAfterPurge = gameStates.size();
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

vector<GameState> Iteration::run() {
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
    vector<GameState> finishedStates;
    for (GameState& gst : gameStates) {
        if (progress.isFinished(gst)) {
            finishedStates.push_back(move(gst));
        }
    }
    return finishedStates;
}

PurchaseIteration::PurchaseIteration(
    GameState st, const Progress& progress,
    const vector<vector<double>>& sacrificeStrategies, bool verbose
)   :   Iteration(progress, verbose)
{
    // If Konami code exploit isn't used, assume this is the starting game state 
    // and add some default instructions.
    // Otherwise, buy the first dimension - this is likely an unavoidable purchase.
    if (st.canUseKonami()) {
        st.addInstructions({Instruction::bulk(1, 1), Instruction::KonamiCode, Instruction::bulk(1, 3)});
    } else {
        st.addInstructions({Instruction::bulk(1, 1)});
    }
    st.runNextInstructions();
    for (int i = 0; i < sacrificeStrategies.size(); i++) {
        gameStates.push_back(st);
        for (double instruction : sacrificeStrategies[i]) {
            gameStates[i].addInstructions({108, instruction});
        }
    }
    states = gameStates.size();
}

void PurchaseIteration::addBranches() {
    size_t begin = 0;
    size_t end = gameStates.size();
    while (begin < end) {
        for (size_t i = begin; i < end; i++) {
            gameStates[i].runNextInstructions();
            computePurchaseBranches(gameStates[i], newGameStates);
        }
        appendNewStates();
        begin = end;
        end = gameStates.size();
    }
}

ApproxSacrificeIteration::ApproxSacrificeIteration(
    GameState st, const Progress& progress,
    const vector<vector<double>>& purchaseStrategies, bool verbose
)   :   Iteration(progress, verbose)
{
    for (int i = 0; i < purchaseStrategies.size(); i++) {
        gameStates.push_back(st);
        gameStates[i].addInstructions(purchaseStrategies[i]);
    }
    states = gameStates.size();
}

void ApproxSacrificeIteration::addBranches() {
    for (GameState& gst : gameStates) {
        if (gst.canSacrifice() && gst.canSacBranch()) {
            GameState newGst = gst;
            Decimal currSac = newGst.sacrifice().getSacrificeBonus() * newGst.nextSacrificeBoost();
            Decimal prevSac = newGst.sacrifice().getSacrificeBonus() * 
                Decimal::max(newGst.sacrifice().nextSacrificeBoost(newGst.getPrevAD1()), DC::D1);
            Decimal target = (currSac + prevSac) / DC::D2;
            newGst.addInstructions({Instruction::Sacrifice, target.log10value()});
            Decimal nextSac = currSac * precision;
            newGst.setNextSacBranching(DC::D1);
            gst.setNextSacBranching(nextSac);
            newGameStates.push_back(move(newGst));
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
    projectedSacrifices = sacrificeStrategies[0];
    for (int i = 0; i < purchaseStrategies.size(); i++) {
        gameStates.push_back(st);
        gameStates[i].addInstructions(purchaseStrategies[i]);
        gameStates[i].setNextSacBranching(Decimal(1, projectedSacrifices[0]) / tolerance);
    }
    states = gameStates.size();
}

void PreciseSacrificeIteration::addBranches() {
    for (GameState& gst : gameStates) {
        if (gst.canSacrifice() && gst.canSacBranch()) {
            Decimal currSac = gst.sacrifice().getSacrificeBonus() * gst.nextSacrificeBoost();
            Decimal prevSac = gst.sacrifice().getSacrificeBonus() * 
                Decimal::max(gst.sacrifice().nextSacrificeBoost(gst.getPrevAD1()), DC::D1);
            Decimal target = (currSac + prevSac) / DC::D2;
            if (currSac > Decimal(1, projectedSacrifices[gst.sacrifice().getSacrificesDone()]) * tolerance) {
                scheduleSacrifice(gst, target);
            } else {
                GameState newGst = gst;
                scheduleSacrifice(newGst, target);
                Decimal nextSac = currSac * precision;
                gst.setNextSacBranching(nextSac);
                newGameStates.push_back(move(newGst));
            }
        }
    }
    appendNewStates();
}

void PreciseSacrificeIteration::scheduleSacrifice(GameState& st, const Decimal& target) const {
    st.addInstructions({Instruction::Sacrifice, target.log10value()});
    st.setNextSacBranching(
        Decimal(1, projectedSacrifices[st.sacrifice().getSacrificesDone() + 1]) / tolerance);
}

void computePurchaseBranches(GameState& gameState, vector<GameState>& branches) {
    while (gameState.canBranch() && !gameState.queue().hasNextPurchase()) {
        vector<uint16_t> variants = gameState.getPurchaseBranches();
        for (int i = 1; i < variants.size(); i++) {
            GameState newGst = gameState;
            newGst.addInstructions({(double) variants[i]});
            newGst.runNextInstructions();
            branches.push_back(move(newGst));
        }
        gameState.addInstructions({(double) variants[0]});
        gameState.runNextInstructions();
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

void purge(vector<GameState>& gamestates, const function<void(int, double)>& onProgress) {
    int size = gamestates.size();
    vector<int> indices(size);
    iota(indices.begin(), indices.end(), 0);
    sort(indices.begin(), indices.end(), [&gamestates](int i, int j) {
        return gamestates[i].AD()[1].getAmount() > gamestates[j].AD()[1].getAmount();
    });
    vector<double> values;
    int width = 0;
    for (int i = 0; i < size; i++) {
        GameState& gst = gamestates[indices[i]];
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
    vector<GameState> result;
    int i = 0;
    for (auto stIter = gamestates.begin(); stIter != gamestates.end(); stIter++) {
        if (removedOrig[i] == 0) {
            result.push_back(move(*stIter));
        }
        i++;
    }
    gamestates.swap(result);
}
