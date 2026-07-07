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
        st.addInstructions({11, 130, 13});
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
    bool hasWinner;
    int maxInstructions = st.instructionsExecuted();
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
                    purchases[i] = 10 - p;
                }
            } else {
                if (bestState.AD()[i].getCost() == priceRange / DC::D10) {
                    purchases[i * 10] = 1;
                }
            }
        }
        permutations = getPermutations(purchases);
        if (verbose) {
            cout << "Got all possible variants for available purchases | ";
            timer.reset();
        }
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
        if (verbose) {
            cout << "Populated game states; currently " << aliveGameStates.size() << " | ";
            timer.reset();
        }
        // Runs all savestate branches until a winner is found.
        // A winner is a savestate that managed to complete all queued purchases the earliest.
        hasWinner = false;
        while (!hasWinner) {
            int currInstructions = 0;
            for (GameState& gst : aliveGameStates) {
                gst.tick(0.033);
                gst.runNextInstructions();
                if (stopCondition(gst)) {
                    if (verbose) cout << "Finished! | ";
                    else cout << "Total time elapsed: ";
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
                if (gst.instructionsExecuted() > currInstructions) {
                    currInstructions = gst.instructionsExecuted();
                }
            }
            if (currInstructions > maxInstructions) {
                maxInstructions = currInstructions;
                aliveGameStates = purge(aliveGameStates, verbose);
            }
        }
        if (verbose) {
            cout << "Simulated all gamestates | ";
            timer.reset();
        }
        // Removes all savestates that are behind at least one other savestate
        // in all comparable values.
        int beforeSize = aliveGameStates.size();
        aliveGameStates = purge(aliveGameStates, verbose);
        if (verbose) {
            cout << "Purged " << beforeSize - aliveGameStates.size() << " gamestates; " << aliveGameStates.size() << " remain | ";
            timer.reset();
        }
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
    if (totalFeatures == 0) {
        return 0;
    } else if (score == totalFeatures) {
        return 1;
    } else if (score == -totalFeatures) {
        return -1;
    } else {
        return 0;
    }
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

vector<GameState> purge(vector<GameState>& gamestates, bool verbose) {
    set<int> toBeRemoved;
    for (int i = 0; i < gamestates.size(); i++) {
        for (int j = i + 1; j < gamestates.size(); j++) {
            if (i == j) continue;
            int cmp = compare(gamestates[i], gamestates[j]);
            if (cmp > 0) {
                toBeRemoved.emplace(j);
            } else if (cmp < 0) {
                toBeRemoved.emplace(i);
            }
        }
    }
    vector<GameState> result;
    int i = 0;
    auto removedIter = toBeRemoved.begin();
    for (auto stIter = gamestates.begin(); stIter != gamestates.end(); stIter++) {
        if (i == *removedIter && removedIter != toBeRemoved.end()) {
            removedIter++;
        } else {
            result.push_back(*stIter);
        }
        i++;
    }
    return result;
}
