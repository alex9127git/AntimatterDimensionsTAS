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
    return runUntil(st, [](GameState& st) {return st.canBuyNextDimboost();}, useSacrifice, 1000, true);
}

GameState runDimboost(GameState st, bool useSacrifice, int precision) {
    return runUntil(st, [](GameState& st) {return st.canBuyNextDimboost();}, useSacrifice, precision, true);
}

GameState runDimboost(GameState st, bool useSacrifice, int precision, bool verbose) {
    return runUntil(st, [](GameState& st) {return st.canBuyNextDimboost();}, useSacrifice, precision, verbose);
}

GameState runUntil(GameState st, function<bool(GameState&)> stopCondition, bool useSacrifice, int precision, bool verbose) {
    vector<GameState> gameStates;
    GameState bestState;
    int64_t currentIterationTime = INT64_MAX;
    int64_t iterationTime = INT64_MAX;
    int64_t startTime = st.realTimePlayed();
    vector<vector<double>> winnerStrategies = {{}};
    int iterationCounter = 0;
    do {
        iterationCounter++;
        currentIterationTime = iterationTime;
        cout << "Running iteration " << iterationCounter << endl;
        if (iterationCounter % 2 == 1) {
            gameStates = purchaseRun(st, stopCondition, winnerStrategies, verbose);
            winnerStrategies.clear();
            for (GameState& gst : gameStates) {
                winnerStrategies.push_back(gst.getCompletedPurchases());
            }
        } else {
            gameStates = sacrificeRun(st, stopCondition, precision, winnerStrategies, verbose);
            winnerStrategies.clear();
            for (GameState& gst : gameStates) {
                winnerStrategies.push_back(gst.getCompletedSacrifices());
            }
        }
        GameState iterationBestState = gameStates[0].copy();
        iterationTime = iterationBestState.realTimePlayed();
        if (iterationTime < currentIterationTime) bestState = iterationBestState;
        if (verbose) {
            cout << "Iteration " << iterationCounter << endl;
            cout << "Segment time: " << renderTime(iterationTime - startTime) << endl;
            cout << "Cumulative time: " << renderTime(iterationTime) << endl;
            cout << "Winning strategies left: " << winnerStrategies.size() << endl;
        }
    } while (st.dimensionBoosts() >= 5 && useSacrifice && iterationTime < currentIterationTime);
    bestState.clearAllInstructions();
    bestState.addInstructions({100});
    bestState.runNextInstructions();
    return bestState;
}

vector<GameState> purchaseRun(GameState st, function<bool(GameState&)> stopCondition, vector<vector<double>> sacrificeStrategies, bool verbose) {
    Decimal priceRange = DC::D10;
    // If Konami code exploit isn't used, assume this is the starting game state 
    // and add some default instructions.
    // Otherwise, buy the first dimension - this is likely an unavoidable purchase.
    if (st.canUseKonami()) {
        st.addInstructions({11, 130, 13});
    } else {
        st.addInstructions({11});
    }
    st.runNextInstructions();
    // The function will have a list of all game states that are currently running;
    // at the start there should be only the starting game state passed to the function
    vector<GameState> gameStates;
    vector<GameState> newGameStates;
    newGameStates.clear();
    GameState bestState;
    Decimal bestAntimatter = DC::D1;
    Timer timer;
    Timer branchTimer;
    Timer checkTimer;
    Timer tickTimer;
    Timer purgeTimer;
    double branchTime = 0;
    double checkTime = 0;
    double tickTime = 0;
    double purgeTime = 0;
    bool isFinished = false;
    int ticks = 0;
    int statesAfterPurge = 1;
    int adds = 0;
    int maxStates = 0;
    for (int i = 0; i < sacrificeStrategies.size(); i++) {
        gameStates.push_back(st.copy());
        for (double instruction : sacrificeStrategies[i]) {
            gameStates[i].addInstructions({108, instruction});
        }
    }
    int states = gameStates.size();
    while (true) {
        ticks++;
        // Branching
        branchTimer.silentReset();
        size_t begin = 0;
        size_t end = gameStates.size();
        while (begin < end) {
            for (size_t i = begin; i < end; i++) {
                gameStates[i].runNextInstructions();
                computePurchaseBranches(gameStates[i], newGameStates);
            }
            if (!newGameStates.empty()) adds++;
            states += newGameStates.size();
            for (GameState& gst : newGameStates) gameStates.push_back(move(gst));
            newGameStates.clear();
            begin = end;
            end = gameStates.size();
        }
        branchTime += branchTimer.silentReset();
        // Checking for condition
        checkTimer.silentReset();
        for (GameState& gst : gameStates) {
            gst.runNextInstructions();
            if (stopCondition(gst)) {
                if (verbose) {
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
                break;
            }
            if (ticks % 50 == 0 && verbose) {
                if (gst.antimatterThisReset() > bestAntimatter) {
                    bestAntimatter = Decimal::max(bestAntimatter, gst.antimatterThisReset());
                    bestState = gst;
                }
            }
        }
        if (isFinished) break;
        if (ticks % 50 == 0 && verbose) {
            Decimal currLog = Decimal::max(bestAntimatter, DC::D1).log10();
            Decimal goalLog = bestState.getAntimatterGoalForDimboost().log10();
            renderProgressBar(Decimal::toNumber(currLog / goalLog));
        }
        checkTime += checkTimer.silentReset();
        tickTimer.silentReset();
        // Tick and purchases
        for (GameState& gst : gameStates) {
            gst.tick(0.033);
            gst.runNextInstructions();
        }
        tickTime += tickTimer.silentReset();
        // Purge and updating progress
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
    vector<GameState> finishedStates;
    for (GameState& gst : gameStates) {
        if (stopCondition(gst)) {
            finishedStates.push_back(gst);
        }
    }
    return finishedStates;
}

vector<GameState> sacrificeRun(GameState st, function<bool(GameState&)> stopCondition, int precision, vector<vector<double>> purchaseStrategies, bool verbose) {
    vector<GameState> gameStates;
    vector<GameState> newGameStates;
    GameState bestState;
    Decimal bestAntimatter = DC::D1;
    Timer timer;
    Timer branchTimer;
    Timer checkTimer;
    Timer tickTimer;
    Timer purgeTimer;
    double branchTime = 0;
    double checkTime = 0;
    double tickTime = 0;
    double purgeTime = 0;
    bool isFinished = false;
    int ticks = 0;
    int statesAfterPurge = 1;
    int adds = 0;
    int maxStates = 0;
    for (int i = 0; i < purchaseStrategies.size(); i++) {
        gameStates.push_back(st.copy());
        gameStates[i].addInstructions(purchaseStrategies[i]);
    }
    int states = gameStates.size();
    while (true) {
        ticks++;
        // Branching
        branchTimer.silentReset();
        for (GameState& gst : gameStates) {
            if (gst.canSacrifice() && gst.canSacBranch()) {
                GameState newGst = gst.copy();
                double sacValue = Decimal::toNumber(newGst.nextSacrificeBoost());
                newGst.addInstructions({108.0, log10(sacValue)});
                double nextSacValue = (floor(sacValue * precision) + 1) / precision;
                gst.setNextSacBranching(nextSacValue);
                newGameStates.push_back(move(newGst));
            }
        }
        if (!newGameStates.empty()) {
            adds += 1;
        }
        for (GameState& gst : newGameStates) {
            gameStates.push_back(move(gst));
            states++;
        }
        branchTime += branchTimer.silentReset();
        newGameStates.clear();
        // Checking for condition
        checkTimer.silentReset();
        for (GameState& gst : gameStates) {
            gst.runNextInstructions();
            if (stopCondition(gst)) {
                if (verbose) {
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
                if (gst.antimatter() > bestAntimatter) {
                    bestAntimatter = Decimal::max(bestAntimatter, gst.antimatter());
                    bestState = gst;
                }
            }
        }
        if (isFinished) break;
        if (ticks % 50 == 0 && verbose) {
            Decimal currLog = Decimal::max(bestAntimatter, DC::D1).log10();
            Decimal goalLog = bestState.getAntimatterGoalForDimboost().log10();
            renderProgressBar(Decimal::toNumber(currLog / goalLog));
        }
        checkTime += checkTimer.silentReset();
        tickTimer.silentReset();
        // Tick and purchases
        for (GameState& gst : gameStates) {
            gst.tick(0.033);
            gst.runNextInstructions();
        }
        tickTime += tickTimer.silentReset();
        // Purge and updating progress
        if (adds >= 300 || states >= statesAfterPurge * 1.5) {
            purgeTimer.silentReset();
            adds = 0;
            int beforeSize = gameStates.size();
            maxStates = max(maxStates, beforeSize);
            purge(gameStates, verbose);
            statesAfterPurge = gameStates.size();
            states = statesAfterPurge;
            purgeTime += purgeTimer.silentReset();
            //cout << "tick: " << ticks << "; before: " << beforeSize << "; after: " << statesAfterPurge << string(100, ' ') << endl;
        }
    }
    vector<GameState> finishedStates;
    for (GameState& gst : gameStates) {
        if (stopCondition(gst)) {
            finishedStates.push_back(gst);
        }
    }
    return finishedStates;
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
