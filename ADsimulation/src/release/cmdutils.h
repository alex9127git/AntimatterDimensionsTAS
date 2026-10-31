#pragma once
#include "gameinstance.h"
#include "ioutils.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <map>


enum RunType {
    NextDimboost,
    NextGalaxy,
    ForceNextGalaxy
};

struct InputData {
    bool isValid = true;

    std::string inputFile;
    std::string outputFile;
    std::string cmdOutFile;
    bool saveCommands = true;
    bool useSacrifice = false;

    GameState gameState;
    RunType runType = NextDimboost;
    
    void saveResult(const GameInstance& instance) {
        std::cout << "Finished simulation; dumping result game state into " << outputFile << std::endl;
        std::ofstream save_out = std::ofstream(outputFile);
        save_out << instance.game().to_json();
        if (saveCommands) {
            std::cout << "Dumping command sequence into " << cmdOutFile << std::endl;
            std::ofstream cmd_out = std::ofstream(cmdOutFile);
            writeInstructions(cmd_out, instance.queue());
        }
    }
};

inline InputData cancel(InputData& data) {
    std::cout << "Unrecognized input. Aborting execution." << std::endl;
    data.isValid = false;
    return data;
}

inline InputData getInputFromCmd(int argc, char* argv[]) {
    InputData result = InputData();
    if (argc == 1) {
        std::cout << "Enter input filename, or leave blank to use default game state:" << std::endl;
        std::getline(std::cin, result.inputFile);
        std::cout << "Enter save output filename:" << std::endl;
        std::getline(std::cin, result.outputFile);
        if (result.outputFile.empty()) {
            std::cout << "Save output file name must be specified. Aborting the execution." << std::endl;
            result.isValid = false;
            return result;
        }
        std::cout << "Enter command output filename:" << std::endl;
        getline(std::cin, result.cmdOutFile);
    } else if (needsHelp(argc, argv)) {
        std::cout << "Usage:" << std::endl;
        std::cout << "Run with no arguments to be prompted input and output filenames." << std::endl;
        std::cout << "Run with an input and output filename (like main in.json out.json) to use those instead." << std::endl;
        std::cout << "If you specify a command file (like main in.json out.json cmd.json)," << std::endl;
        std::cout << "command sequence will be saved too." << std::endl;
        result.isValid = false;
        return result;
    } else {
        std::map<std::string, std::string> parsedArgs = parseArgs(argc, argv);
        result.inputFile = parsedArgs["--input-file"];
        result.outputFile = parsedArgs["--output-file"];
        result.cmdOutFile = parsedArgs["--cmd-file"];
    }
    if (!result.inputFile.empty()) {
        std::ifstream in = std::ifstream(result.inputFile);
        json j;
        in >> j;
        result.gameState = GameState(j);
    } else {
        std::cout << "Input filename not specified, will use starting game state instead." << std::endl;
    }
    if (result.outputFile.empty()) {
        std::cout << "Save output filename must be specified. Aborting the program." << std::endl;
        result.isValid = false;
        return result;
    }
    if (result.cmdOutFile.empty()) {
        std::cout << "Command output filename not specified. Command sequence will not be saved." << std::endl;
        result.saveCommands = false;
    }
    std::cout << "Choose type of run:" << std::endl;
    std::cout << "0 - Get next Dimension Boost" << std::endl;
    std::cout << "1 - Get next Antimatter Galaxy" << std::endl;
    if (result.gameState.dimensionBoosts() >= 4) {
        std::cout << "2 - Force Antimatter Galaxy" << std::endl;
    }
    std::string line;
    std::getline(std::cin, line);
    int t;
    try {
        t = line.empty() ? 0 : std::stoi(line);
    } catch (const std::exception& e) {
        return cancel(result);
    };
    if (t < 0 || t > 2) return cancel(result);
    if (t == 2 && result.gameState.dimensionBoosts() < 4) return cancel(result);
    result.runType = (RunType) t;
    if (result.gameState.dimensionBoosts() >= 5) {
        std::cout << "Input game state has 5 dimension boosts or more. Do you want to use sacrifice? [Y/n] ";
        std::getline(std::cin, line);
        char confirmation = line.empty() ? 'y' : line[0];
        switch (confirmation) {
            case 'Y':
            case 'y':
            case '\r':
                result.useSacrifice = true;
                break;
            case 'N':
            case 'n':
                result.useSacrifice = false;
                break;
            default:
                return cancel(result);
        }
    }
    return result;
}
