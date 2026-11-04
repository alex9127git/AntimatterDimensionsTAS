#pragma once
#include "searchnode.h"
#include "gamestate.h"
#include <functional>
#include <vector>
#include <array>
using Key = std::array<int32_t, 9>;


inline constexpr size_t GROUP_THRESHOLD = 4096;

struct Grouping {
    std::vector<Key> keys;
    std::vector<int> groupLookup;
    std::vector<char> groupCompares;
    size_t groupCnt = 0;

    void initialize(std::vector<SearchNode>& nodes, std::vector<int>& indices);
    bool canDominate(int a, int b) const;
};

void appendValues(const GameState& st, std::vector<double>& values);

Key getPurchasesFrom(const GameState& st);
int compare(const double* st1, const double* st2, size_t width, bool strictlyBetter);
bool isFutureEqual(const SearchNode& nodeA, const SearchNode& nodeB);
void purge(std::vector<SearchNode>& nodes, const std::function<void(int, double)>& onProgress={});