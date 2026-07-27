#include "ioutils.h"
#include <format>
#include <map>


string renderTime(long ms) {
    int hours = ms / 3600000;
    int minutes = (ms % 3600000) / 60000;
    int seconds = (ms % 60000) / 1000;
    int milliseconds = ms % 1000;
    return format("{:02d}:{:02d}:{:02d}.{:03d}", hours, minutes, seconds, milliseconds);
}

bool needsHelp(int argc, char* argv[]) {
    string arg = argv[1];
    return argc == 2 && (arg == "-h" || arg == "--help");
}

map<string, string> parseArgs(int argc, char* argv[]) {
    map<string, string> parsedArgs;
    string argName = "";
    string argValue = "";
    string arg;
    int unknownArgs = 0;
    for (int i = 1; i < argc; i++) {
        arg = argv[i];
        if (arg.starts_with("--")) {
            auto index = arg.find("=");
            if (index == string::npos) {
                argName = arg;
            } else {
                argName = arg.substr(0, index);
                argValue = arg.substr(index + 1);
                if (!argValue.empty()) {
                    parsedArgs.emplace(argName, argValue);
                    argName = "";
                }
            }
        } else if (arg.starts_with("-")) {
            if (SHORTHANDS.contains(arg)) {
                argName = SHORTHANDS[arg];
            } else {
                argName = arg;
            }
        } else {
            if (!argName.empty()) {
                parsedArgs.emplace(argName, arg);
                argName = "";
            } else {
                bool argSet = false;
                for (string defArg : DEFAULTARGS) {
                    if (!parsedArgs.contains(defArg)) {
                        parsedArgs.emplace(defArg, arg);
                        argSet = true;
                        break;
                    }
                }
                if (!argSet) {
                    parsedArgs.emplace("--unknown-arg-" + unknownArgs, arg);
                    unknownArgs++;
                }
            }
        }
    }
    for (string defArg : DEFAULTARGS) {
        if (!parsedArgs.contains(defArg)) {
            parsedArgs.emplace(defArg, "");
        }
    }
    return parsedArgs;
}