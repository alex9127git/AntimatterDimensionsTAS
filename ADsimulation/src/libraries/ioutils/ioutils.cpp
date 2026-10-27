#include "ioutils.h"
#include <format>
#include <cmath>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif
using namespace std;

bool isTerminal(FILE* stream) {
#ifdef _WIN32
    return _isatty(_fileno(stream));
#else
    return isatty(fileno(stream));
#endif
}

void enableAnsiEscapes() {
#ifdef _WIN32
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(out, &mode)) {
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

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
                argName = SHORTHANDS.at(arg);
            } else {
                argName = arg;
            }
        } else {
            if (!argName.empty()) {
                parsedArgs.emplace(argName, arg);
                argName = "";
            } else {
                bool argSet = false;
                for (const string& defArg : DEFAULTARGS) {
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
    for (const string& defArg : DEFAULTARGS) {
        if (!parsedArgs.contains(defArg)) {
            parsedArgs.emplace(defArg, "");
        }
    }
    return parsedArgs;
}

string renderProgressBar(double fraction) {
    string s = format("{:>7.3f}%", fraction * 100);
    int completedWidth = floor(fraction * 50);
    s +=  " [" + string(completedWidth, '#') + string(50 - completedWidth, ' ') + "]";
    return s;
}
