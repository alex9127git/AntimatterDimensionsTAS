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
    // The function will have a list of all game states that are currently running;
    // at the start there should be only the starting game state passed to the function
    vector<GameState> gameStates;
    vector<GameState> newGameStates;
    Timer timer;
    Timer branchTimer;
    Timer tickTimer;
    Timer purgeTimer;
    double branchTime;
    double tickTime;
    double purgeTime;
    gameStates.push_back(st.copy());
    int ticks = 0;
    while (true) {
        branchTimer.silentReset();
        for (GameState& gst : gameStates) {
            if (!gst.canBranch() || gst.hasNextInstruction()) continue;
            Decimal priceRange = gst.getPriceRange();
            vector<int> variants;
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
        }
        for (GameState& gst : newGameStates) {
            gameStates.push_back(gst);
        }
        branchTime += branchTimer.silentReset();
        newGameStates.clear();
        tickTimer.silentReset();
        for (GameState& gst : gameStates) {
            gst.runNextInstructions();
            if (stopCondition(gst)) {
                cout << "Finished! Total time elapsed: ";
                timer.reset();
                cout << "branch: " << branchTime << " ms" << endl;
                cout << "tick: " << tickTime << " ms" << endl;
                cout << "purge: " << purgeTime << " ms" << endl;
                return gst;
            }
        }
        for (GameState& gst : gameStates) {
            gst.tick(0.033);
            gst.runNextInstructions();
        }
        tickTime += tickTimer.silentReset();
        ticks++;
        if (ticks == 100) {
            purgeTimer.silentReset();
            ticks = 0;
            int beforeSize = gameStates.size();
            gameStates = purge(gameStates, verbose);
            purgeTime += purgeTimer.silentReset();
        }
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
    if (toBeRemoved.size() > 0) {
        //cout << "lol\n";
    }
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
