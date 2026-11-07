#include "testHelper.h"
#include <stdexcept>
#include <fstream>
#include <format>
#include <cmath>
using json = nlohmann::json;
using namespace std;


json readJson(const string& path) {
    ifstream in = ifstream(path);
    if (!in.is_open()) throw runtime_error("file not found: " + path);
    json j;
    in >> j;
    return j;
}

vector<double> parseCommands(const json& commands) {
    vector<double> route;
    for (const json& line : commands) {
        const string name = line[0];
        const json& args = line[1];
        if (name == "buyDimension") {
            for (int i = 0; i < args[1].get<int>(); i++) route.push_back({args[0].get<double>()});
        } else if (name == "buyTickSpeed") {
            route.push_back((double) Instruction::TickspeedSingle);
        } else if (name == "trySacrificeReset") {
            route.push_back((double) Instruction::Sacrifice);
            route.push_back(log10(stod(args[0].get<string>())));
        } else if (name == "buyDimensionBoost") {
            route.push_back((double) Instruction::DimensionBoost);
        } else if (name == "buyGalaxyReset") {
            route.push_back((double) Instruction::AntimatterGalaxy);
        }
    }
    return route;
}

void runAllQueuedInstructions(GameInstance& instance) {
    instance.runNextInstructions();
    for (int64_t ticks = 0; instance.queue().hasNextPurchase(); ticks++) {
        if (ticks >= MAX_REPLAY_TICKS) {
            throw runtime_error(format("route took too long to execute: {} purchases left",
                                       instance.queue().getQueuedPurchases().size()));
        }
        instance.tick(0.033);
        instance.runNextInstructions();
    }
}
