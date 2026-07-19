#include "simulation.h"
#include "../constants/constants.h"
#include "../permutations/permutations.h"
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

GameState runDimboost(GameState st) {
    return runDimboost(st, true);
}

GameState runDimboost(GameState st, bool verbose) {
    vector<GameState> gameStates;
    long startTime = st.realTimePlayed();
    gameStates = purchaseRun(st, [](GameState& st) {return st.canBuyNextDimboost();}, verbose);
    if (st.dimensionBoosts() < 5) {
        GameState bestState = gameStates[0].copy();
        bestState.requestDimboost();
        return bestState;
    }
    bool isSacrificeIteration = true;
    long bestTime = gameStates[0].realTimePlayed();
    long currentIterationTime = bestTime;
    vector<vector<double>> winnerPurchaseStrategies;
    for (GameState& gst : gameStates) {
        winnerPurchaseStrategies.push_back(gst.getCompletedPurchases());
    }
    gameStates = sacrificeRun(st, [](GameState& st) {return st.canBuyNextDimboost();}, winnerPurchaseStrategies, verbose);
    return gameStates[0];
    // while (bestTime != currentIterationTime) {
    //     if (isSacrificeIteration) {
    //         vector<vector<double>> winnerPurchaseStrategies;
    //         for (GameState& gst : gameStates) {
    //             winnerPurchaseStrategies.push_back(gst.getCompletedPurchases());
    //         }
    //         gameStates = sacrificeRun(st, [](GameState& st) {return st.canBuyNextDimboost();}, winnerPurchaseStrategies, verbose);
    //     }
    // }
}

vector<GameState> purchaseRun(GameState st, function<bool(GameState&)> stopCondition, bool verbose) {
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
    GameState bestState;
    Decimal bestAntimatter = DC::D1;
    Timer timer;
    Timer branchTimer;
    Timer checkTimer;
    Timer tickTimer;
    Timer buyTimer;
    Timer purgeTimer;
    double branchTime;
    double checkTime;
    double tickTime;
    double buyTime;
    double purgeTime;
    bool isFinished = false;
    int ticks = 0;
    int maxStates = 0;
    gameStates.push_back(st.copy());
    while (true) {
        ticks++;
        // Branching
        branchTimer.silentReset();
        for (GameState& gst : gameStates) {
            // Savestate will branch if isn't currently busy with an instruction and can afford at least one purchase
            while (gst.canBranch() && !gst.hasNextPurchaseInstruction()) {
                Decimal priceRange = gst.getPriceRange();
                // Gets all possible purchases at current price range and populates the game states accordingly
                vector<double> variants;
                if (gst.tickspeed().canPurchase(priceRange)) {
                    variants.push_back(9);
                }
                for (int i = 1; i <= 8; i++) {
                    int purchases = gst.AD()[i].getPurchases();
                    if (purchases < 10 || i == min(gst.dimensionBoosts() + 4, 8)) {
                        if (gst.AD()[i].canPurchase(priceRange)) {
                            variants.push_back(i);
                        }
                    } else {
                        if (gst.AD()[i].canPurchase(priceRange / DC::D10)) {
                            variants.push_back(i * 10);
                        }
                    }
                }
                if (variants.empty()) {
                    continue;
                } else if (variants.size() == 1) {
                    gst.addInstructions(variants);
                } else {
                    for (int i = 1; i < variants.size(); i++) {
                        GameState newGst = gst.copy();
                        newGst.addInstructions({variants[i]});
                        newGameStates.push_back(newGst);
                    }
                    gst.addInstructions({variants[0]});
                }
                gst.runNextInstructions();
            }
        }
        for (GameState& gst : newGameStates) {
            gameStates.push_back(gst);
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
                    cout << "buy: " << buyTime << " ms" << endl;
                    cout << "purge: " << purgeTime << " ms" << endl;
                }
                isFinished = true;
            }
            if (ticks == 100 && verbose) {
                if (gst.antimatter() > bestAntimatter) {
                    bestAntimatter = Decimal::max(bestAntimatter, gst.antimatter());
                    bestState = gst;
                }
            }
        }
        if (isFinished) break;
        if (ticks == 100 && verbose) {
            Decimal currLog = Decimal::max(bestAntimatter, DC::D1).log10();
            Decimal goalLog = bestState.getAntimatterGoalForDimboost().log10();
            renderProgressBar(Decimal::toNumber(currLog / goalLog));
        }
        checkTime += checkTimer.silentReset();
        // Tick and purchases
        for (GameState& gst : gameStates) {
            tickTimer.silentReset();
            gst.tick(0.033);
            tickTime += tickTimer.silentReset();
            buyTimer.silentReset();
            gst.runNextInstructions();
            buyTime += buyTimer.silentReset();
        }
        // Purge and updating progress
        if (ticks == 100) {
            purgeTimer.silentReset();
            ticks = 0;
            int beforeSize = gameStates.size();
            maxStates = max(maxStates, beforeSize);
            gameStates = purge(gameStates, verbose);
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

vector<GameState> sacrificeRun(GameState st, function<bool(GameState&)> stopCondition, vector<vector<double>> purchaseStrategies, bool verbose) {
    vector<GameState> gameStates;
    vector<GameState> newGameStates;
    GameState bestState;
    Decimal bestAntimatter = DC::D1;
    Timer timer;
    Timer branchTimer;
    Timer checkTimer;
    Timer tickTimer;
    Timer buyTimer;
    Timer purgeTimer;
    double branchTime;
    double checkTime;
    double tickTime;
    double buyTime;
    double purgeTime;
    bool isFinished = false;
    int ticks = 0;
    int maxStates = 0;
    for (int i = 0; i < purchaseStrategies.size(); i++) {
        gameStates.push_back(st.copy());
        gameStates[i].addInstructions(purchaseStrategies[i]);
    }
    while (true) {
        ticks++;
        // Branching
        branchTimer.silentReset();
        for (GameState& gst : gameStates) {
            if (gst.canSacrifice()) {
                GameState newGst = gst.copy();
                newGst.addInstructions({108.0, Decimal::toNumber(newGst.nextSacrificeBoost().log10())});
                newGameStates.push_back(newGst);
            }
        }
        for (GameState& gst : newGameStates) {
            gameStates.push_back(gst);
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
                    cout << "buy: " << buyTime << " ms" << endl;
                    cout << "purge: " << purgeTime << " ms" << endl;
                }
                isFinished = true;
            }
            if (ticks % 10 == 0 && verbose) {
                if (gst.antimatter() > bestAntimatter) {
                    bestAntimatter = Decimal::max(bestAntimatter, gst.antimatter());
                    bestState = gst;
                }
            }
        }
        if (isFinished) break;
        if (ticks % 10 == 0 && verbose) {
            Decimal currLog = Decimal::max(bestAntimatter, DC::D1).log10();
            Decimal goalLog = bestState.getAntimatterGoalForDimboost().log10();
            renderProgressBar(Decimal::toNumber(currLog / goalLog));
        }
        checkTime += checkTimer.silentReset();
        // Tick and purchases
        for (GameState& gst : gameStates) {
            tickTimer.silentReset();
            gst.tick(0.033);
            tickTime += tickTimer.silentReset();
            buyTimer.silentReset();
            gst.runNextInstructions();
            buyTime += buyTimer.silentReset();
        }
        // Purge and updating progress
        if (ticks == 100) {
            purgeTimer.silentReset();
            ticks = 0;
            int beforeSize = gameStates.size();
            maxStates = max(maxStates, beforeSize);
            gameStates = purge(gameStates, verbose);
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

int compare(vector<Decimal>& st1, vector<Decimal>& st2) {
    // Compare the two game states together.
    // If a first game state has a property bigger, score variable shifts positive.
    // Otherwise it shifts negative. This value is then used to determine the return value.
    // If st1 is strictly better than st2, this function should return 1.
    // If st2 is strictly better than st2, this function should return -1.
    // Otherwise, return 0.
    int score1 = 0;
    int score2 = 0;
    for (int i = 0; i < st1.size(); i++) {
        Decimal v1 = st1[i];
        Decimal v2 = st2[i];
        if (v1 > v2) score1++;
        if (v1 < v2) score2++;
        if (score1 > 0 && score2 > 0) return 0;
    }
    if (score1 > 0 && score2 == 0) {
        return 1;
    } else if (score2 > 0 && score1 == 0) {
        return -1;
    } else {
        return 0;
    }
}

vector<GameState> purge(vector<GameState>& gamestates, bool verbose) {
    vector<vector<Decimal>> values;
    for (GameState& gst : gamestates) {
        vector<Decimal> valueRow;
        valueRow.push_back(gst.antimatter());
        for (int i = 1; i <= 8; i++) {
            valueRow.push_back(gst.AD()[i].getAmount());
            valueRow.push_back(Decimal(gst.AD()[i].getPurchases()));
        }
        valueRow.push_back(gst.getSacrificeBonus());
        valueRow.push_back(Decimal(gst.tickspeed().getPurchases()));
        valueRow.push_back(gst.tickspeed().perSecond());
        valueRow.push_back(gst.getAchievementBonus());
        values.push_back(valueRow);
    }
    int canBeRemoved[values.size()];
    for (int i = 0; i < values.size(); i++) {
        canBeRemoved[i] = 0;
    }
    for (int i = 0; i < values.size(); i++) {
        if (canBeRemoved[i] == 1) continue;
        for (int j = i + 1; j < values.size(); j++) {
            if (i == j) continue;
            if (canBeRemoved[j] == 1) continue;
            int cmp = compare(values[i], values[j]);
            if (cmp > 0) {
                canBeRemoved[j] = 1;
            } else if (cmp < 0) {
                canBeRemoved[i] = 1;
            }
        }
    }
    vector<GameState> result;
    int i = 0;
    for (auto stIter = gamestates.begin(); stIter != gamestates.end(); stIter++) {
        if (canBeRemoved[i] == 0) {
            result.push_back(*stIter);
        }
        i++;
    }
    return result;
}
