#include <iostream>
#include "libraries/gamestate/gamestate.h"
using namespace std;

int main() {
    GameState gameState = GameState();
    for (const Dimension& d : gameState.getVectorAD()) {
        cout << "Currency cost(" << d.cost << "), scaling(" << d.scaling << ")\n";
    };
    return 0;
};
