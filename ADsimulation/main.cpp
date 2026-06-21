#include <iostream>
#include "libraries/gamestate/gamestate.h"
using namespace std;

int main() {
    GameState gameState = GameState();
    for (const Dimension& d : gameState.getVectorAD()) {
        cout << d << endl;
    };
    return 0;
};
