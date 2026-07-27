#include <iostream>
#include <fstream>
#include "libraries/ioutils/ioutils.h"
#include "libraries/constants/constants.h"
#include "libraries/gamestate/gamestate.h"
#include "libraries/simulation/simulation.h"

using namespace std;


int main(int argc, char* argv[]) {
    string inputFile;
    string outputFile;
    string cmdOutFile;
    bool saveCommands = true;

    GameState gameState;
    json j;
    ifstream in;
    ofstream save_out;
    ofstream cmd_out;

    if (argc == 1) {
        cout << "Enter input filename, or leave blank to use default game state:" << endl;
        getline(cin, inputFile);
        cout << "Enter save output filename:" << endl;
        getline(cin, outputFile);
        if (outputFile.empty()) {
            cout << "Save output file name must be specified. Aborting the execution." << endl;
            return 0;
        }
        cout << "Enter command output filename:" << endl;
        getline(cin, cmdOutFile);
    } else if (needsHelp(argc, argv)) {
        cout << "Usage:" << endl;
        cout << "Run with no arguments to be prompted input and output filenames." << endl;
        cout << "Run with an input and output filename (like main in.json out.json) to use those instead." << endl;
        cout << "If you specify a command file (like main in.json out.json cmd.json)," << endl;
        cout << "command sequence will be saved too." << endl;
        return 0;
    } else {
        map<string, string> parsedArgs = parseArgs(argc, argv);
        inputFile = parsedArgs["--input-file"];
        outputFile = parsedArgs["--output-file"];
        cmdOutFile = parsedArgs["--cmd-file"];
    }
    if (!inputFile.empty()) {
        in = ifstream(inputFile);
        in >> j;
        gameState = GameState(j);
    } else {
        cout << "Input filename not specified, will use starting game state instead." << endl;
    }
    if (outputFile.empty()) {
        cout << "Save output filename must be specified. Aborting the program." << endl;
        return 0;
    }
    if (cmdOutFile.empty()) {
        cout << "Command output filename not specified. Command sequence will not be saved." << endl;
        saveCommands = false;
    }
    bool useSacrifice = false;
    if (gameState.dimensionBoosts() >= 5) {
        cout << "Input game state has 5 dimension boosts or more. Do you want to use sacrifice? [Y/n] ";
        char confirmation = getchar();
        switch (confirmation) {
            case 'Y':
            case 'y':
            case '\r':
            case '\n':
                useSacrifice = true;
                break;
            case 'N':
            case 'n':
                useSacrifice = false;
                break;
            default:
                cout << "Unrecognized input. Aborting the execution." << endl;
                return 0;
        }
    }
    
    save_out = ofstream(outputFile);
    vector<double> instructions;
    gameState = runDimboost(gameState, useSacrifice, 400);
    cout << "Finished simulation; dumping result game state into " << outputFile << endl;
    save_out << gameState.to_json();
    if (saveCommands) {
        cout << "Dumping command sequence into " << cmdOutFile << endl;
        cmd_out = ofstream(cmdOutFile);
        gameState.writeInstructions(cmd_out);
    }
    return 0;
};
