#include "simulation.h"
#include "../constants/constants.h"
#include "../permutations/permutations.h"
#include <vector>
#include <list>
#include <set>
using namespace std;


GameState run(GameState st, function<bool(GameState&)> stopCondition) {
    return run(st, stopCondition, false);
}

GameState run(GameState st, function<bool(GameState&)> stopCondition, bool verbose) {
    Decimal priceRange = DC::D10;
    // If Konami code exploit isn't used, assume this is the starting game state 
    // and add some default instructions.
    // Otherwise, buy the first dimension - this is likely an unavoidable purchase.
    if (st.canUseKonami()) {
        st.addInstructions({11, 130, 19});
        priceRange = DC::D100;
    } else {
        st.addInstructions({11});
    }
    st.runNextInstructions();
    while (st.hasNextInstruction()) {
        st.tick(0.033);
        st.runNextInstructions();
    }
    // The function will have a list of all game states that are currently running;
    // at the start there should be only the starting game state passed to the function
    GameState& bestState = st;
    vector<GameState> aliveGameStates;
    aliveGameStates.push_back(st.copy());
    vector<GameState> newGameStates;
    map<int, int> purchases;
    vector<vector<int>> permutations;
    set<int> toBeRemoved;
    bool hasWinner;
    Timer timer;
    while (true) {
        if (verbose) cout << "Simulating price range " << priceRange << endl;
        // Creates all possible permutations of purchases accessible at this price range.
        purchases.clear();
        if (bestState.tickspeed().getCost() == priceRange) {
            purchases[9] = 1;
        }
        for (int i = 1; i <= 8; i++) {
            if (!bestState.AD()[i].isUnlocked()) continue;
            int p = bestState.AD()[i].getPurchases();
            if (p < 10) {
                if (bestState.AD()[i].getCost() == priceRange) {
                    purchases[i] = 10;
                }
            } else {
                if (bestState.AD()[i].getCost() == priceRange / DC::D10) {
                    purchases[i * 10] = 1;
                }
            }
        }
        permutations = getPermutations(purchases);
        if (verbose) timer.reset();
        // For each already present save state creates branches for every permutation
        // of possible purchases.
        newGameStates.clear();
        for (GameState& ref : aliveGameStates) {
            for (vector<int> perm : permutations) {
                GameState gst = ref.copy();
                gst.addInstructions(perm);
                newGameStates.push_back(gst);
            }
        }
        aliveGameStates = newGameStates;
        if (verbose) timer.reset();
        // Runs all savestate branches until a winner is found.
        // A winner is a savestate that managed to complete all queued purchases the earliest.
        hasWinner = false;
        while (!hasWinner) {
            for (GameState& gst : aliveGameStates) {
                gst.tick(0.033);
                gst.runNextInstructions();
                if (stopCondition(gst)) {
                    timer.reset();
                    return gst;
                }
                if (!gst.hasNextInstruction()) {
                    if (!hasWinner) {
                        bestState = gst;
                    } else if (hasWinner && gst.realTimePlayed() <= bestState.realTimePlayed()) {
                        bestState = gst;
                    }
                    hasWinner = true;
                }
            }
        }
        if (verbose) timer.reset();
        // Removes all savestates that are behind at least one other savestate
        // in all comparable values.
        toBeRemoved.clear();
        for (int i = 0; i < aliveGameStates.size(); i++) {
            for (int j = i + 1; j < aliveGameStates.size(); j++) {
                if (i == j) continue;
                int cmp = compare(aliveGameStates[i], aliveGameStates[j]);
                if (cmp > 0) {
                    toBeRemoved.emplace(j);
                } else if (cmp < 0) {
                    toBeRemoved.emplace(i);
                }
            }
        }
        if (verbose) timer.reset();
        // for (auto iter = toBeRemoved.rbegin(); iter != toBeRemoved.rend(); iter++) {
        //     aliveGameStates.erase(aliveGameStates.begin() + (*iter));
        // }
        newGameStates.clear();
        int i = 0;
        auto removedIter = toBeRemoved.begin();
        for (auto stIter = aliveGameStates.begin(); stIter != aliveGameStates.end(); stIter++) {
            if (i == *removedIter && removedIter != toBeRemoved.end()) {
                removedIter++;
            } else {
                newGameStates.push_back(*stIter);
            }
            i++;
        }
        aliveGameStates = newGameStates;
        if (verbose) timer.reset();
        priceRange *= DC::D10;
        if (verbose) cout << endl;
    }
}

int compare(GameState& st1, GameState& st2) {
    // Compare the two game states together.
    // If a first game state has a property bigger, score variable shifts positive.
    // Otherwise it shifts negative. This value is then used to determine the return value.
    // If st1 is strictly better than st2, this function should return 1.
    // If st2 is strictly better than st2, this function should return -1.
    // Otherwise, return 0.
    int score = 0;
    int totalFeatures = 28;
    compareValues(st1.antimatter(), st2.antimatter(), score, totalFeatures);
    for (int i = 1; i <= 8; i++) {
        Dimension dim1 = st1.AD()[i];
        Dimension dim2 = st2.AD()[i];
        compareValues(dim1.getAmount(), dim2.getAmount(), score, totalFeatures);
        compareValues(dim1.getPurchases(), dim2.getPurchases(), score, totalFeatures);
        compareValues(dim1.productionPerSecond(), dim2.productionPerSecond(), score, totalFeatures);
    }
    compareValues(st1.tickspeed().getPurchases(), st2.tickspeed().getPurchases(), score, totalFeatures);
    compareValues(st1.tickspeed().perSecond(), st2.tickspeed().perSecond(), score, totalFeatures);
    compareValues(st1.getAchievementBonus(), st2.getAchievementBonus(), score, totalFeatures);
    if (score == totalFeatures) {
        return 1;
    } else if (score == -totalFeatures) {
        return -1;
    }
    return 0;
}

void compareValues(Decimal v1, Decimal v2, int& score, int& totalFeatures) {
    if (v1 > v2) {
        score += 1;
    } else if (v1 < v2) {
        score -= 1;
    } else {
        totalFeatures -= 1;
    }
}

void compareValues(int v1, int v2, int& score, int& totalFeatures) {
    if (v1 > v2) {
        score += 1;
    } else if (v1 < v2) {
        score -= 1;
    } else {
        totalFeatures -= 1;
    }
}