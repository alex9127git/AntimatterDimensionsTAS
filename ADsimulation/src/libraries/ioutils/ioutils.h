#pragma once
#include <cstdio>
#include <string>
#include <vector>
#include <map>


inline const std::map<std::string, std::string> SHORTHANDS = {
    {"-i", "--input-file"},
    {"-o", "--output-file"},
    {"-c", "--cmd-file"}
};
inline const std::vector<std::string> DEFAULTARGS = {
    "--input-file", "--output-file", "--cmd-file"
};

bool isTerminal(FILE* stream);
void enableAnsiEscapes();

std::string renderTime(long ms);
bool needsHelp(int argc, char* argv[]);
std::map<std::string, std::string> parseArgs(int argc, char* argv[]);
std::string renderProgressBar(double fraction);
