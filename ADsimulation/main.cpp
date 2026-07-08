#include <iostream>
#include "libraries/constants/constants.h"
#include "libraries/gamestate/gamestate.h"
#include "libraries/simulation/simulation.h"
using namespace std;

int main() {
    GameState gameState;
    vector<int> instructions;
    gameState = GameState();
    gameState = run(gameState, 
        [](GameState& st) {return st.AD()[4].getPurchases() >= 20;}, false);
    cout << endl << gameState << endl;
    instructions = gameState.getCompletedInstructions();
    for (int instruction : instructions) {
        cout << instruction << " ";
    }
    cout << endl << endl;

    gameState = GameState(
        DC::D10,
        1,
        1080816,
        false,
        {11, 12, 13, 14}
    );
    gameState = run(gameState, 
        [](GameState& st) {return st.AD()[5].getPurchases() >= 20;}, false);
    cout << endl << gameState << endl;
    instructions = gameState.getCompletedInstructions();
    for (int instruction : instructions) {
        cout << instruction << " ";
    }
    cout << endl << endl;
    
    gameState = GameState(
        DC::D10,
        2,
        2337027,
        false,
        {11, 12, 13, 14, 15}
    );
    gameState = run(gameState, 
        [](GameState& st) {return st.AD()[6].getPurchases() >= 20;}, false);
    cout << endl << gameState << endl;
    instructions = gameState.getCompletedInstructions();
    for (int instruction : instructions) {
        cout << instruction << " ";
    }
    cout << endl << endl;
    
    gameState = GameState(
        DC::D10,
        3,
        3562746,
        false,
        {11, 12, 13, 14, 15, 16}
    );
    gameState = run(gameState, 
        [](GameState& st) {return st.AD()[7].getPurchases() >= 20;}, false);
    cout << endl << gameState << endl;
    instructions = gameState.getCompletedInstructions();
    for (int instruction : instructions) {
        cout << instruction << " ";
    }
    cout << endl << endl;
    
    gameState = GameState(
        DC::D10,
        4,
        4595712,
        false,
        {11, 12, 13, 14, 15, 16, 17}
    );
    gameState = run(gameState, 
        [](GameState& st) {return st.AD()[8].getPurchases() >= 20;}, false);
    cout << endl << gameState << endl;
    instructions = gameState.getCompletedInstructions();
    for (int instruction : instructions) {
        cout << instruction << " ";
    }
    cout << endl << endl;
    return 0;
};
