#include "gamestate.h"
#include "constants.h"
#include "formulae.h"
#include <algorithm>
#include <cmath>
using namespace std;


const function<bool(const GameState&)> AchConstants::NOT_APPLICABLE = 
        [](const GameState&) {return false;};
const function<bool(const GameState&)> AchConstants::ALWAYS_UNLOCKED = 
        [](const GameState&) {return true;};
const function<bool(const GameState&)> AchConstants::NOT_IMPLEMENTED_YET = 
        [](const GameState&) {return false;};

const vector<function<bool(const GameState&)>> AchConstants::unlockConditions = {
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,

    NOT_IMPLEMENTED_YET,
    ALWAYS_UNLOCKED,
    [](const GameState& st) {return st.AD()[8].getPurchases() == 99;},
    [](const GameState& st) {return st.antimatter() >= DC::D1E80;},
    [](const GameState& st) {return st.dimensionBoosts() >= 10;},
    NOT_APPLICABLE,
    [](const GameState& st) {return st.antimatterGalaxies() >= 2;},
    [](const GameState& st) {return st.AD()[1].getAmount() >= DC::D1E150;},

    [](const GameState& st) {
        for (int i = 1; i <= 8; i++) {
            if (st.AD()[i].getMult() >= DC::D1E31) {
                return true;
            }
        }
        return false;
    },
    [](const GameState& st) {return st.sacrifice().getSacrificeBonus() >= DC::D600;},
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    ALWAYS_UNLOCKED,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,

    NOT_IMPLEMENTED_YET,
    [](const GameState& st) {
        return st.antimatter() >= DC::D1E63 && 
            st.AD()[1].productionPerSecond(st.tickspeed()) > st.antimatter();
    },
    [](const GameState& st) {
        for (int i = 1; i <= 7; i++) {
            if (st.AD()[i].getMult() >= st.AD()[i+1].getMult()) return false;
        }
        return true;
    },
    ALWAYS_UNLOCKED,
    [](const GameState& st) {return st.tickspeed().perSecond() >= DC::D1E28;},
    [](const GameState& st) {return st.AD()[7].getAmount() >= DC::D1E12;},
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,


    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,

    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    ALWAYS_UNLOCKED,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET
};

Decimal getADmult(const GameState& st, int tier) {
    Decimal mult = DC::D1;
    mult *= st.getAchievementBonus();
    mult *= Decimal::pow(DC::D2, floor(st.AD()[tier].getPurchases() / 10));
    mult *= Decimal::pow(DC::D2, max(0, st.dimensionBoosts() - tier + 1));
    if (tier == 1) {
        mult *= st.achievements().effectOrDefault(28);
        mult *= st.achievements().effectOrDefault(31);
    }
    if (tier == 8) {
        mult *= st.sacrifice().getSacrificeBonus();
        mult *= st.achievements().effectOrDefault(23);
    }
    return mult;
}

Decimal getTickspeedMultPerPurchase(const GameState& st) {
    double baseMultiplier;
    switch (st.antimatterGalaxies()) {
        case 0: baseMultiplier = 1 / 1.1245; break;
        case 1: baseMultiplier = 1 / 1.11888888; break;
        case 2: baseMultiplier = 1 / 1.11267177; break;
        default: baseMultiplier = 0.8;
    }
    return Decimal(max(0.01, baseMultiplier - 0.02 * st.antimatterGalaxies()));
}

Decimal getTotalTickspeed(const GameState& st) {
    return DC::D1E3 * Decimal::pow(getTickspeedMultPerPurchase(st), st.tickspeed().getPurchases());
}

Decimal getCostForDimPurchases(const ExponentialCostScaling& costRule, int32_t dimTarget) {
    return costRule.getCost((dimTarget - 1) / 10) * Decimal(dimTarget % 10 == 0 ? 10 : dimTarget % 10);
}

int getTierForDimboost(int32_t dimboostTarget) {
    return min(8, 3 + dimboostTarget);
}

int32_t getAmountForDimboost(int32_t dimboostTarget) {
    return 20 + 15 * max(0, dimboostTarget - 5);
}

int32_t getAmountForGalaxy(int32_t galaxyTarget) {
    return 80 + 60 * max(0, galaxyTarget - 1);
}
