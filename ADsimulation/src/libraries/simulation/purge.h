#pragma once
#include "searchnode.h"
#include <functional>
#include <vector>


void purge(std::vector<SearchNode>& nodes, const std::function<void(int, double)>& onProgress={});
