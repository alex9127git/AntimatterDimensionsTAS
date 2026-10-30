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

    NOT_APPLICABLE,
    ALWAYS_UNLOCKED,
    NOT_APPLICABLE,
    [](const GameState& st) {return st.antimatter() >= DC::D1E80;},
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,

    [](const GameState& st) {
        for (int i = 1; i <= 8; i++) {
            if (st.AD()[i].getMult() >= DC::D1E31) {
                return true;
            }
        }
        return false;
    },
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    ALWAYS_UNLOCKED,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,

    NOT_APPLICABLE,
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
    [](const GameState& st) {return st.tickspeed().perSecond() >= DC::D1E26;},
    [](const GameState& st) {return st.AD()[7].getAmount() >= DC::D1E12;},
    NOT_APPLICABLE,
    NOT_APPLICABLE,


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
    if (tier == 8) {
        mult *= st.sacrifice().getSacrificeBonus();
    }
    return mult;
}

Decimal getTickspeedMultPerPurchase([[maybe_unused]] const GameState& st) {
    return Decimal(1 / 1.1245);
}

Decimal getTotalTickspeed(const GameState& st) {
    return DC::D1E3 * Decimal::pow(getTickspeedMultPerPurchase(st), st.tickspeed().getPurchases());
}

int getTierForDimboost(int32_t dimboostTarget) {
    return min(8, 3 + dimboostTarget);
}

int32_t getAmountForDimboost(int32_t dimboostTarget) {
    return 20 + 15 * max(0, dimboostTarget - 5);
}
