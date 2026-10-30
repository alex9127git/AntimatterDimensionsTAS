#pragma once
#include "searchnode.h"
#include "gamestate.h"
#include <functional>
#include <vector>


void appendValues(const GameState& st, std::vector<double>& values);

bool compare(const double* st1, const double* st2, size_t width);
void purge(std::vector<SearchNode>& nodes, const std::function<void(int, double)>& onProgress={});