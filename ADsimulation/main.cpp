#include <iostream>
#include "libraries/gamestate/gamestate.h"
using namespace std;

int main() {
    GameState gameState = GameState();
    for (const auto& d : gameState.antimatterDimensions.dims) {
        cout << "Currency cost(" << d.cost.mantissa << ", " << d.cost.exponent << "), scaling(" << d.scaling.mantissa << ", " << d.scaling.exponent << ") "  << "\n";
    };
    return 0;
};
