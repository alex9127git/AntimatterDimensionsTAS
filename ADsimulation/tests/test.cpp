#include <iostream>
#include <cassert>
#include "../libraries/constants/constants.h"
#include "../libraries/decimal/decimal.h"
#include "../libraries/gamestate/gamestate.h"
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
    cout << "All math tests passed" << endl << endl;

    cout << "Started game state tests" << endl;
    GameState gameState = GameState();
    gameState.buyOneDimension(1);
    cout << gameState.AD()[1].productionPerSecond() << endl;
    for (int i = 0; i < 30; i++) {
        gameState.tick(1.0 / 30);
    }
    cout << "You have " << gameState.antimatter() << " antimatter." << endl;
    for (const Dimension& d : gameState.getVectorAD()) {
        cout << d << endl;
    };
}