#pragma once
#include "../decimal/decimal.h"
#include <functional>
#include <vector>


class GameState;


struct AchConstants {
    static const std::function<bool(const GameState&)> NOT_APPLICABLE;
    static const std::function<bool(const GameState&)> ALWAYS_UNLOCKED;
    static const std::function<bool(const GameState&)> NOT_IMPLEMENTED_YET;
    static const std::vector<std::function<bool(const GameState&)>> unlockConditions;
};

Decimal getADmult(const GameState& st, int tier);
Decimal getTickspeedMultPerPurchase(const GameState& st);
Decimal getTotalTickspeed(const GameState& st);