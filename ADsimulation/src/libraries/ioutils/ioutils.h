#pragma once
#include <string>
#include <vector>
#include <map>
using namespace std;


inline const map<string, string> SHORTHANDS = {
    {"-i", "--input-file"},
    {"-o", "--output-file"},
    {"-c", "--cmd-file"}
};
inline const vector<string> DEFAULTARGS = {
    "--input-file", "--output-file", "--cmd-file"
};


void enableAnsiEscapes();

string renderTime(long ms);
bool needsHelp(int argc, char* argv[]);
map<string, string> parseArgs(int argc, char* argv[]);