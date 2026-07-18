#include <iostream>
#include <cassert>
#include "../libraries/constants/constants.h"
#include "../libraries/decimal/decimal.h"
#include "../libraries/gamestate/gamestate.h"
#include "../libraries/permutations/permutations.h"
#include "../libraries/simulation/simulation.h"
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;


int main() {
    Decimal a = DC::D0;
    Decimal b = DC::D0;
    Decimal c = DC::D0;

    cout << "Started equivalence tests" << endl;
    a = Decimal(5);
    b = Decimal(5, 0LL);
    assert(a == b);
    a = Decimal(37);
    b = Decimal(3.7, 1LL);
    assert(a == b);
    a = Decimal(69420);
    b = Decimal(6.942, 4LL);
    c = Decimal(694.2, 2LL);
    assert(a == b);
    assert(b == c);
    assert(a == c);
    cout << "All equivalence tests passed" << endl << endl;

    cout << "Started comparison tests" << endl;
    a = Decimal(5);
    b = Decimal(4);
    assert(a > b);
    a = Decimal(10);
    b = DC::D10;
    assert(a >= b);
    a = Decimal(694201337);
    b = Decimal(694201336);
    assert(a > b);
    a = Decimal(0.003);
    b = Decimal(0.1);
    c = Decimal(1.5);
    assert(a < b);
    assert(a < c);
    assert(b < c);
    a = Decimal(-1);
    b = Decimal(1);
    assert(a < b);
    a = Decimal(-10);
    b = Decimal(1);
    assert(a < b);
    a = Decimal(-100);
    b = Decimal(1);
    assert(a < b);
    a = Decimal(0.69);
    b = Decimal(-0.69);
    c = Decimal(0);
    assert(a > c);
    assert(b < c);
    cout << "All comparison tests passed" << endl << endl;

    cout << "Started math tests" << endl;
    a = Decimal(77);
    b = Decimal(33);
    assert(a + b == Decimal(110));
    assert(a - b == Decimal(44));
    assert(a * b == Decimal(77 * 33));
    assert(-a == Decimal(-77));
    a = Decimal(69420);
    b = Decimal(1337);
    assert(a * b == Decimal(69420 * 1337));
    a = Decimal(1, 100LL);
    b = Decimal(1, 150LL);
    assert(a + b == Decimal(1, 150LL));
    assert(a * b == Decimal(1, 250LL));
    a = Decimal(69);
    b = DC::D1E5;
    assert(Decimal::invert(a) == Decimal(1.0/69));
    assert(Decimal::invert(b) == Decimal(1, -5LL));
    a = Decimal(1001);
    b = Decimal(7);
    assert((a / b) == Decimal(143));
    a = Decimal(9);
    assert(Decimal::pow(a, a) == Decimal(387420489));
    a = Decimal(10);
    b = Decimal(0.5);
    //Decimal::pow(a, b).repr();
    assert(Decimal::pow(a, b) > Decimal(3.1622776));
    assert(Decimal::pow(a, b) < Decimal(3.1622777));
    a = Decimal(2);
    assert(Decimal::pow(a, 1024) > Decimal(1, 308LL));
    cout << "All math tests passed" << endl << endl;

    cout << "Started permutation tests" << endl;
    map<int, int> permMap1;
    permMap1[1] = 1;
    permMap1[2] = 1;
    permMap1[3] = 1;
    vector<vector<int>> permutations = getPermutations(permMap1);
    for (vector<int> permutation : permutations) {
        for (int elem : permutation) {
            cout << elem << " ";
        }
        cout << endl;
    }
    assert(permutations.size() == 6);

    map<int, int> permMap2;
    permMap2[15] = 1;
    permMap2[16] = 1;
    permMap2[17] = 1;
    permMap2[8] = 10;
    permMap2[9] = 1;
    permutations = getPermutations(permMap2);
    assert(permutations.size() == 24024);

    map<int, int> permMap3;
    permMap3[13] = 1;
    permMap3[14] = 1;
    permMap3[15] = 1;
    permMap3[16] = 1;
    permMap3[17] = 1;
    permMap3[8] = 10;
    permMap3[9] = 1;
    permutations = getPermutations(permMap3);
    assert(permutations.size() == 5765760);

    map<int, int> permMap4;
    permMap4[1] = 10;
    permutations = getPermutations(permMap4);
    assert(permutations.size() == 1);
    cout << "All permutation tests passed" << endl << endl;

    cout << "Started game state tests" << endl;
    GameState gameState = GameState();

    cout << "Checking default state assertions" << endl;
    gameState.buyDimUntil10(2);
    gameState.buyTickspeed();

    assert(gameState.AD()[2].getPurchases() == 0);
    assert(gameState.tickspeed().getPurchases() == 0);
    assert(gameState.getAchievementBonus() == Decimal::pow(DC::D1_03, 4));
    cout << "Assertions checked; entering fixed order check" << endl;

    gameState = GameState();
    gameState.addInstructions({
        11, 130, 19, 20, 91, 32, 91, 38, 10, 91, 41, 91, 49, 91, 20, 91, 10, 91, 30, 91, 20, 10, 92, 40
    });
    gameState.runNextInstructions();

    while (!gameState.canBuyNextDimboost()) {
        gameState.tick(0.033);
        gameState.runNextInstructions();
    }
    cout << endl << gameState << endl;
    assert(gameState.realTimePlayed() == 1080816);
    cout << "Fixed order check done; time taken is optimal" << endl << endl;

    json save = {
        {"achievementState", {
            {"unlockedAchievements", {11,12,13,14,15,16,17,18,22,35,44,76}},
        }},
        {"antimatter", {{"exponent",1},{"mantissa",1.0}}},
        {"antimatterDimensionState", {{
            "dims", {
                {
                    {"amount",{{"exponent",0},{"mantissa",0.0}}},
                    {"cost",{{"exponent",1},{"mantissa",1.0}}},
                    {"purchases",0},
                    {"scaling",{{"exponent",3},{"mantissa",1.0}}},
                    {"tier",1},
                    {"unlocked",true}
                },
                {
                    {"amount",{{"exponent",0},{"mantissa",0.0}}},
                    {"cost",{{"exponent",2},{"mantissa",1.0}}},
                    {"purchases",0},
                    {"scaling",{{"exponent",4},{"mantissa",1.0}}},
                    {"tier",2},
                    {"unlocked",false}
                },
                {
                    {"amount",{{"exponent",0},{"mantissa",0.0}}},
                    {"cost",{{"exponent",4},{"mantissa",1.0}}},
                    {"purchases",0},
                    {"scaling",{{"exponent",5},{"mantissa",1.0}}},
                    {"tier",3},
                    {"unlocked",false}
                },
                {
                    {"amount",{{"exponent",0},{"mantissa",0.0}}},
                    {"cost",{{"exponent",6},{"mantissa",1.0}}},
                    {"purchases",0},{"scaling",{{"exponent",6},{"mantissa",1.0}}},
                    {"tier",4},
                    {"unlocked",false}
                },
                {
                    {"amount",{{"exponent",0},{"mantissa",0.0}}},
                    {"cost",{{"exponent",9},{"mantissa",1.0}}},
                    {"purchases",0},
                    {"scaling",{{"exponent",8},{"mantissa",1.0}}},
                    {"tier",5},
                    {"unlocked",false}
                },
                {
                    {"amount",{{"exponent",0},{"mantissa",0.0}}},
                    {"cost",{{"exponent",13},{"mantissa",1.0}}},
                    {"purchases",0},
                    {"scaling",{{"exponent",10},{"mantissa",1.0}}},
                    {"tier",6},
                    {"unlocked",false}
                },
                {
                    {"amount",{{"exponent",0},{"mantissa",0.0}}},
                    {"cost",{{"exponent",18},{"mantissa",1.0}}},
                    {"purchases",0},
                    {"scaling",{{"exponent",12},{"mantissa",1.0}}},
                    {"tier",7},
                    {"unlocked",false}
                },
                {
                    {"amount",{{"exponent",0},{"mantissa",0.0}}},
                    {"cost",{{"exponent",24},{"mantissa",1.0}}},
                    {"purchases",0},
                    {"scaling",{{"exponent",15},{"mantissa",1.0}}},
                    {"tier",8},
                    {"unlocked",false}
                }
            },
        }}},
        {"canUseKonami",false},
        {"dimensionBoosts",5},
        {"realTimePlayed",5307522},
        {"sacrificed",{{"exponent",0},{"mantissa",1.0}}},
        {"tickspeedState",{{"cost",{{"exponent",3},{"mantissa",1.0}}},{"purchases",0},{"scaling",{{"exponent",1},{"mantissa",1.0}}},{"unlocked",false}}}
    };
    gameState = GameState(save);
    gameState.addInstructions({
        10, 20, 91, 31, 91, 39, 10, 92, 40, 91, 20, 91, 10, 91, 50, 91, 30, 91, 20, 10, 92, 40,
        60, 91, 10, 91, 30, 20, 92, 10, 91, 70, 50, 91, 40, 20, 91, 30, 10, 92, 20, 91, 10, 60,
        80, 108, 3.588, 91, 40, 30, 91, 50, 10, 91, 20, 92, 10, 91, 30, 91, 70, 40, 20, 91, 10,
        91, 60, 91, 50, 91, 30, 20, 10, 91, 40, 92, 10, 20, 92, 80, 108, 2.842, 30, 91, 10, 91,
        50, 91, 70, 20, 40, 91, 60, 10, 91, 30, 92, 20, 10, 92, 40, 91, 50, 10, 30, 91, 20, 92,
        10, 91, 60, 80, 91, 70, 40, 30, 20, 91, 10, 92, 50, 91, 20, 10, 91, 30, 91, 40, 91, 10,
        91, 20, 91, 60, 91, 30, 10, 50, 91, 40, 91, 20, 70, 91, 10, 85
    });
    gameState.runNextInstructions();

    while (!gameState.canBuyNextDimboost()) {
        gameState.tick(0.033);
        gameState.runNextInstructions();
    }
    cout << endl << gameState << endl;
    assert(gameState.realTimePlayed() == 6585546);
    cout << "Fixed order check 2 done; time taken is optimal" << endl << endl;
    
    cout << "Starting test simulation" << endl;
    gameState = GameState();
    gameState = run(gameState, [](GameState& st) {return st.canBuyNextDimboost();}, true);
    cout << endl << gameState << endl;
    vector<double> instructions = gameState.getCompletedInstructions();
    for (double instruction : instructions) {
        cout << instruction << " ";
    }
    cout << endl;
    assert(gameState.realTimePlayed() == 1080816);
    cout << "Time taken is optimal" << endl;
    cout << "All game state tests passed" << endl << endl;

    cout << "Started serialization tests" << endl;
    a = Decimal(69);
    json j = a.to_json();
    b = Decimal(j);
    assert(a == b);
    j = gameState.to_json();
    GameState gsCopy = GameState(j);
    assert(gsCopy.realTimePlayed() == gameState.realTimePlayed());
    assert(gsCopy.getAchievementBonus() == gameState.getAchievementBonus());
    cout << "All serialization tests passed" << endl << endl;

    cout << "All tests passed" << endl;
    return 0;
}