#include <iostream>
#include <cassert>
#include "../libraries/constants/constants.h"
#include "../libraries/decimal/decimal.h"
#include "../libraries/gamestate/gamestate.h"
#include "../libraries/permutations/permutations.h"
using namespace std;


int main() {
    Decimal a = DC::D0;
    Decimal b = DC::D0;
    Decimal c = DC::D0;

    cout << "Started equivalence tests" << endl;
    a = Decimal(5);
    b = Decimal(5, 0);
    assert(a == b);
    a = Decimal(37);
    b = Decimal(3.7, 1);
    assert(a == b);
    a = Decimal(69420);
    b = Decimal(6.942, 4);
    c = Decimal(694.2, 2);
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
    a = Decimal(1, 100);
    b = Decimal(1, 150);
    assert(a + b == Decimal(1, 150));
    assert(a * b == Decimal(1, 250));
    a = Decimal(69);
    b = DC::D1E5;
    assert(Decimal::invert(a) == Decimal(1.0/69));
    assert(Decimal::invert(b) == Decimal(1, -5));
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
    gameState.buyOneDimension(1);
    for (int i = 0; i < 30; i++) {
        gameState.tick(1.0 / 30);
    }
    cout << "You have " << gameState.antimatter() << " antimatter." << endl;
    for (const Dimension& d : gameState.AD().getDims()) {
        cout << d << endl;
    };
}