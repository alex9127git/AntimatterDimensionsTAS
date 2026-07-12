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

    gameState = GameState(
        DC::D10,
        5,
        5307522,
        false,
        {11, 12, 13, 14, 15, 16, 17, 18}
    );
    // gameState.addInstructions(
    //     {10, 20, 9, 31, 9, 39, 9, 10, 9, 40, 9, 20, 9, 10, 9, 50, 9, 30, 9, 20, 10, 9, 9, 40, 60, 9, 10, 9, 30, 20, 9, 9,
    //     10, 9, 70, 50, 9, 40, 20, 9, 30, 10, 9, 9, 20, 9, 10, 9, 80, 60, 9, 40, 30, 9, 50, 10, 9, 20, 9, 9, 10, 9, 30, 9,
    //     70, 40, 20, 9, 10, 9, 9, 60, 50, 9, 30, 20, 10, 9, 9, 40, 9, 10, 9, 20, 80, 9, 30, 9, 10, 9, 50, 9, 70, 20, 40, 9,
    //     60, 10, 9, 30, 9, 9, 20, 10, 9, 9, 40, 9, 50, 30, 10, 9, 20, 9, 9, 10, 9, 60, 80, 9, 70, 40, 30, 20, 9, 10, 9, 9, 
    //     50, 9, 20, 10, 9, 30, 9, 40, 9, 10, 9, 20, 9, 60, 10, 9, 30, 50, 9, 40, 9, 20, 70, 9, 10, 85}
    // );
    // gameState.runNextInstructions();
    // while (gameState.AD()[8].getPurchases() < 35) {
    //     gameState.tick(0.033);
    //     gameState.runNextInstructions();
    // }
    gameState = run(gameState, 
        [](GameState& st) {return st.AD()[8].getPurchases() >= 35;}, false);
    cout << endl << gameState << endl;
    instructions = gameState.getCompletedInstructions();
    for (int instruction : instructions) {
        cout << instruction << " ";
    }
    cout << endl << endl;
    return 0;
};
