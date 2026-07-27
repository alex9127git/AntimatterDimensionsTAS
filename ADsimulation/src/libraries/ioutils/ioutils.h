#pragma once
#include <string>
#include <vector>
#include <map>
using namespace std;


inline map<string, string> SHORTHANDS = {
    {"-i", "--input-file"},
    {"-o", "--output-file"},
    {"-c", "--cmd-file"}
};
inline vector<string> DEFAULTARGS = {
    "--input-file", "--output-file", "--cmd-file"
};


string renderTime(long ms);
bool needsHelp(int argc, char* argv[]);
map<string, string> parseArgs(int argc, char* argv[]);