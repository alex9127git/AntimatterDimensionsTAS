#pragma once
#include "costmath.h"
#include "decimal.h"
#include <functional>
#include <cstdint>
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

Decimal getCostForDimPurchases(const ExponentialCostScaling& costRule, int32_t dimTarget);

int getTierForDimboost(int32_t dimboostTarget);
int32_t getAmountForDimboost(int32_t dimboostTarget);

int32_t getAmountForGalaxy(int32_t galaxyTarget);