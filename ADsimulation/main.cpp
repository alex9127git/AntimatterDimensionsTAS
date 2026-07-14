#include <iostream>
#include <fstream>
#include "libraries/constants/constants.h"
#include "libraries/gamestate/gamestate.h"
#include "libraries/simulation/simulation.h"

using namespace std;


int main(int argc, char* argv[]) {
    string inputFile;
    string outputFile;

    GameState gameState;
    json j;
    ifstream i;
    ofstream o;

    if (argc == 1) {
        cout << "Enter input filename, or leave blank to use default game state:" << endl;
        getline(cin, inputFile);
        if (!inputFile.empty()) {
            i = ifstream(inputFile);
            i >> j;
            gameState = GameState(j);
        }
        cout << "Enter output filename:" << endl;
        getline(cin, outputFile);
        o = ofstream(outputFile);
    } else if (argc == 3) {
        cout << "Command line arguments detected: using first argument as an input file and second argument as an output file" << endl;
        i = ifstream(argv[1]);
        o = ofstream(argv[2]);
        outputFile = argv[2];
        i >> j;
        gameState = GameState(j);
    } else {
        cout << "Usage:" << endl;
        cout << "Run with no arguments to be prompted input and output filenames." << endl;
        cout << "Run with an input and output filename (like main in.json out.json) to use those instead." << endl;
        return 0;
    }

    vector<int> instructions;
    gameState = run(gameState, 
        [](GameState& st) {return st.canBuyNextDimboost();}, false);
    gameState.requestDimboost();
    cout << "Finished simulation; dumping result game state into " << outputFile << endl;
    o << gameState.to_json();
    cout << "Resulting instruction sequence:" << endl;
    instructions = gameState.getCompletedInstructions();
    for (int instruction : instructions) {
        cout << instruction << " ";
    }
    cout << endl;
    return 0;
};
