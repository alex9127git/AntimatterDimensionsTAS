#pragma once
#include <nlohmann/json.hpp>
#include "gameinstance.h"
#include <cstdint>
#include <vector>
#include <string>


inline constexpr int64_t MAX_REPLAY_TICKS = 1000000;


nlohmann::json readJson(const std::string& path);
std::vector<double> parseCommands(const nlohmann::json& commands);
void runAllQueuedInstructions(GameInstance& instance);
json restrictToKeysOf(const json& actual, const json& expected);
