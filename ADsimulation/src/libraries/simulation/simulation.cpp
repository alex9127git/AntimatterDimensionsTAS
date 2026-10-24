#include "simulation.h"
#include "../ioutils/ioutils.h"
#include "../constants/constants.h"
#include "../permutations/permutations.h"
#include <cstdint>
#include <vector>
#include <list>
#include <set>
using namespace std;


void renderProgressBar(double percentage) {
    cout << format("{:>7.3f}%", percentage * 100);
    int completedWidth = floor(percentage * 50);
    cout << " [" << string(completedWidth, '#') << string(50 - completedWidth, ' ') << "]";
    cout << "\r" << flush;
}

GameState runDimboost(GameState st, bool useSacrifice) {
    return runUntil(
        st, [](GameState& st) {return st.canBuyNextDimboost();}, 
        [](GameState& st) {
            return st.antimatterThisReset().log10value() / st.getAntimatterGoalForDimboost().log10value();
        }, useSacrifice, true
    );
}

GameState runDimboost(GameState st, bool useSacrifice, bool verbose) {
    return runUntil(
        st, [](GameState& st) {return st.canBuyNextDimboost();}, 
        [](GameState& st) {
            return st.antimatterThisReset().log10value() / st.getAntimatterGoalForDimboost().log10value();
        }, useSacrifice, verbose
    );
}

GameState runUntil(
    GameState st, function<bool(GameState&)> stopCondition, 
    function<double(GameState&)> progressFunction,
    bool useSacrifice, bool verbose
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
        cout << "Running iteration " << iterationCounter << endl;
        switch (iterationCounter % 2) {
            case 1:
                gameStates = purchaseRun(
                    st, stopCondition, progressFunction, sacrificeStrategies, verbose
                );
                purchaseStrategies.clear();
                for (GameState& gst : gameStates) {
                    purchaseStrategies.push_back(gst.getCompletedPurchases());
                }
                break;
            case 0:
                gameStates = sacrificeRun(
                    st, stopCondition, progressFunction, purchaseStrategies, verbose
                );
                purchaseStrategies.clear();
                sacrificeStrategies.clear();
                for (GameState& gst : gameStates) {
                    purchaseStrategies.push_back(gst.getCompletedPurchases());
                    vector<double> sacStrat = gst.getCompletedSacrifices();
                    sacStrat.push_back(
                        (gst.getSacrificeBonus() * gst.nextSacrificeBoost() * DC::D2).log10value()
                    );
                    sacrificeStrategies.push_back(sacStrat);
                }
                cout << "Optimizing sacrifices" << endl;
                gameStates = deepSacRun(
                    st, stopCondition, progressFunction, purchaseStrategies, sacrificeStrategies, verbose
                );
                sacrificeStrategies.clear();
                for (GameState& gst : gameStates) {
                    sacrificeStrategies.push_back(gst.getCompletedSacrifices());
                }
                break;
        }
        GameState iterationBestState = gameStates[0].copy();
        iterationTime = iterationBestState.realTimePlayed();
        if (iterationTime < currentIterationTime) bestState = iterationBestState;
        if (verbose) {
            cout << "Iteration " << iterationCounter << endl;
            cout << "Segment time: " << renderTime(iterationTime - startTime) << endl;
            cout << "Cumulative time: " << renderTime(iterationTime) << endl;
            cout << "Winning strategies left: " << purchaseStrategies.size() << endl;
        }
    } while (st.dimensionBoosts() >= 5 && useSacrifice && iterationTime < currentIterationTime);
    bestState.clearAllInstructions();
    bestState.addInstructions({100});
    bestState.runNextInstructions();
    return bestState;
}

Progress::Progress(function<double(GameState&)> _func) 
    :   progressFunction(_func) 
    {};

Iteration::Iteration(
    function<double(GameState&)> progressFunction,
    function<bool(GameState&)> _stopCondition, bool _verbose
)   :   stopCondition(_stopCondition),
        verbose(_verbose),
        progress(Progress(progressFunction))
    {}

void Iteration::appendNewStates() {
    if (!newGameStates.empty()) adds++;
    states += newGameStates.size();
    for (GameState& gst : newGameStates) gameStates.push_back(move(gst));
    newGameStates.clear();
}

void Iteration::checkProgress() {
    checkTimer.silentReset();
    GameState* bestState = &gameStates[0];
    for (GameState& gst : gameStates) {
        gst.runNextInstructions();
        if (stopCondition(gst)) {
            if (verbose && !isFinished) {
                renderProgressBar(1);
                cout << endl;
                cout << "Finished! Total time elapsed: ";
                timer.reset();
                cout << "Max States: " << maxStates << endl;
                cout << "branch: " << branchTime << " ms" << endl;
                cout << "check: " << checkTime << " ms" << endl;
                cout << "tick: " << tickTime << " ms" << endl;
                cout << "purge: " << purgeTime << " ms" << endl;
            }
            isFinished = true;
        }
        if (ticks % 50 == 0 && verbose) {
            if (gst.antimatterThisReset() > bestState->antimatterThisReset()) {
                bestState = &gst;
            }
        }
    }
    if (!isFinished && ticks % 50 == 0 && verbose) {
        renderProgressBar(progress.calculate(*bestState));
    }
    checkTime += checkTimer.silentReset();
}

void Iteration::tickAllStates() {
    tickTimer.silentReset();
    // Tick and purchases
    for (GameState& gst : gameStates) {
        gst.tick(0.033);
        gst.runNextInstructions();
    }
    tickTime += tickTimer.silentReset();
}

void Iteration::purgeObsoleteStates() {
    if (adds >= 300 || states >= statesAfterPurge * 1.5) {
        purgeTimer.silentReset();
        adds = 0;
        int beforeSize = gameStates.size();
        maxStates = max(maxStates, beforeSize);
        purge(gameStates, verbose);
        statesAfterPurge = gameStates.size();
        states = statesAfterPurge;
        purgeTime += purgeTimer.silentReset();
    }
}

vector<GameState> Iteration::run() {
    while (true) {
        ticks++;
        // Branching
        branchTimer.silentReset();
        addBranches();
        branchTime += branchTimer.silentReset();
        checkProgress();
        if (isFinished) break;
        tickAllStates();
        purgeObsoleteStates();
    }
    vector<GameState> finishedStates;
    for (GameState& gst : gameStates) {
        if (stopCondition(gst)) {
            finishedStates.push_back(gst);
        }
    }
    return finishedStates;
}

PurchaseIteration::PurchaseIteration(
    GameState st,
    function<bool(GameState&)> _stopCondition,
    function<double(GameState&)> progressFunction,
    vector<vector<double>> sacrificeStrategies, bool _verbose
)   :   Iteration(progressFunction, _stopCondition, _verbose)
{
    // If Konami code exploit isn't used, assume this is the starting game state 
    // and add some default instructions.
    // Otherwise, buy the first dimension - this is likely an unavoidable purchase.
    if (st.canUseKonami()) {
        st.addInstructions({11, 130, 13});
    } else {
        st.addInstructions({11});
    }
    st.runNextInstructions();
    for (int i = 0; i < sacrificeStrategies.size(); i++) {
        gameStates.push_back(st.copy());
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

vector<GameState> purchaseRun(
    GameState st, function<bool(GameState&)> stopCondition, 
    function<double(GameState&)> progressFunction,
    vector<vector<double>> sacrificeStrategies, bool verbose
) {
    return PurchaseIteration(
        st, stopCondition, progressFunction, sacrificeStrategies, verbose
    ).run();
}

ApproxSacrificeIteration::ApproxSacrificeIteration(
    GameState st,
    function<bool(GameState&)> _stopCondition,
    function<double(GameState&)> progressFunction,
    vector<vector<double>> purchaseStrategies, bool _verbose
)   :   Iteration(progressFunction, _stopCondition, _verbose)
{
    for (int i = 0; i < purchaseStrategies.size(); i++) {
        gameStates.push_back(st.copy());
        gameStates[i].addInstructions(purchaseStrategies[i]);
    }
    states = gameStates.size();
}

void ApproxSacrificeIteration::addBranches() {
    for (GameState& gst : gameStates) {
        if (gst.canSacrifice() && gst.canSacBranch()) {
            GameState newGst = gst.copy();
            Decimal currSac = newGst.getSacrificeBonus() * newGst.nextSacrificeBoost();
            Decimal prevSac = newGst.getSacrificeBonus() * 
                Decimal::max(newGst.nextSacrificeBoost(newGst.getPrevAD1()), DC::D1);
            Decimal target = (currSac + prevSac) / DC::D2;
            newGst.addInstructions({108.0, target.log10value()});
            Decimal nextSac = currSac * precision;
            newGst.setNextSacBranching(DC::D1);
            gst.setNextSacBranching(nextSac);
            newGameStates.push_back(move(newGst));
        }
    }
    appendNewStates();
}

vector<GameState> sacrificeRun(
    GameState st, function<bool(GameState&)> stopCondition, 
    function<double(GameState&)> progressFunction,
    vector<vector<double>> purchaseStrategies, bool verbose
) {
    return ApproxSacrificeIteration(
        st, stopCondition, progressFunction, purchaseStrategies, verbose
    ).run();
}

PreciseSacrificeIteration::PreciseSacrificeIteration(
    GameState st,
    function<bool(GameState&)> _stopCondition,
    function<double(GameState&)> progressFunction,
    vector<vector<double>> purchaseStrategies, 
    vector<vector<double>> sacrificeStrategies, bool _verbose
)   :   Iteration(progressFunction, _stopCondition, _verbose)
{
    projectedSacrifices = sacrificeStrategies[0];
    for (int i = 0; i < purchaseStrategies.size(); i++) {
        gameStates.push_back(st.copy());
        gameStates[i].addInstructions(purchaseStrategies[i]);
        gameStates[i].setNextSacBranching(Decimal(1, projectedSacrifices[0]) / tolerance);
    }
    states = gameStates.size();
}

void PreciseSacrificeIteration::addBranches() {
    for (GameState& gst : gameStates) {
        if (gst.canSacrifice() && gst.canSacBranch()) {
            Decimal currSac = gst.getSacrificeBonus() * gst.nextSacrificeBoost();
            Decimal prevSac = gst.getSacrificeBonus() * 
                Decimal::max(gst.nextSacrificeBoost(gst.getPrevAD1()), DC::D1);
            Decimal target = (currSac + prevSac) / DC::D2;
            if (currSac > Decimal(1, projectedSacrifices[gst.getSacrificesDone()]) * tolerance) {
                gst.addInstructions({108.0, target.log10value()});
                gst.setNextSacBranching(
                    Decimal(1, projectedSacrifices[gst.getSacrificesDone() + 1]) / tolerance);
            } else {
                GameState newGst = gst.copy();
                newGst.addInstructions({108.0, target.log10value()});
                newGst.setNextSacBranching(
                    Decimal(1, projectedSacrifices[gst.getSacrificesDone() + 1]) / tolerance);
                Decimal nextSac = currSac * precision;
                gst.setNextSacBranching(nextSac);
                newGameStates.push_back(move(newGst));
            }
        }
    }
    appendNewStates();
}

vector<GameState> deepSacRun(
    GameState st, function<bool(GameState&)> stopCondition, 
    function<double(GameState&)> progressFunction,
    vector<vector<double>> purchaseStrategies, 
    vector<vector<double>> sacrificeStrategies, bool verbose
) {
    return PreciseSacrificeIteration(
        st, stopCondition, progressFunction,
        purchaseStrategies, sacrificeStrategies, verbose
    ).run();
}

void computePurchaseBranches(GameState& gameState, vector<GameState>& branches) {
    while (gameState.canBranch() && !gameState.hasNextPurchaseInstruction()) {
        vector<uint16_t> variants = gameState.getPurchaseBranches();
        for (int i = 1; i < variants.size(); i++) {
            GameState newGst = gameState.copy();
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

void purge(vector<GameState>& gamestates, bool verbose) {
    int size = gamestates.size();
    Timer timer;
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
        values.push_back(gst.getSacrificeBonus().log10value());
        values.push_back(gst.tickspeed().getPurchases());
        values.push_back(gst.tickspeed().perSecond().log10value());
        values.push_back(gst.getAchievementBonus().log10value());
        if (i == 0) {
            width = values.size();
            values.reserve(size * width);
        }
    }
    vector<char> removedCopy(size);
    for (int i = 0; i < size; i++) {
        removedCopy[i] = 0;
    }
    double createArrayTimer = timer.silentReset();
    for (int i = 0; i < size; i++) {
        if (removedCopy[i] == 1) continue;
        for (int j = i + 1; j < size; j++) {
            if (removedCopy[j] == 1) continue;
            if (compare(&values[i * width], &values[j * width], width)) {
                removedCopy[j] = 1;
            }
        }
    }
    vector<char> removedOrig(size);
    for (int i = 0; i < size; i++) {
        removedOrig[indices[i]] = removedCopy[i];
    }
    double compareTimer = timer.silentReset();
    vector<GameState> result;
    int i = 0;
    for (auto stIter = gamestates.begin(); stIter != gamestates.end(); stIter++) {
        if (removedOrig[i] == 0) {
            result.push_back(move(*stIter));
        }
        i++;
    }
    double recreateTimer = timer.silentReset();
    vector<double>().swap(values);
    //cout << createArrayTimer << " " << compareTimer << " " << recreateTimer << endl;
    gamestates.swap(result);
}
