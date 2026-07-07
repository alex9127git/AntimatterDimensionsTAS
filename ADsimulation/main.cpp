#include <iostream>
#include "libraries/constants/constants.h"
#include "libraries/gamestate/gamestate.h"
#include "libraries/simulation/simulation.h"
using namespace std;

int main() {
    GameState gameState = GameState();
    gameState = run(gameState, 
        [](GameState& st) {return st.AD()[4].getPurchases() >= 20;}, false);
    cout << endl << gameState << endl;
    return 0;
};
