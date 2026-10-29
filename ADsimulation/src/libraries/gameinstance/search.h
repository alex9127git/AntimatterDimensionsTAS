#pragma once
#include <vector>
#include <cstdint>


class GameState;


std::vector<uint16_t> computeAllowedPurchases(const GameState& st);
