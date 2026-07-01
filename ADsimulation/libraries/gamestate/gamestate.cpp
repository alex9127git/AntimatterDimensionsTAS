#include "gamestate.h"
#include "../constants/constants.h"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"


GameState::GameState()
    :   _antimatter(DC::D10),
        _AD(AntimatterDimensions()),
        _tickspeed(Tickspeed()),
        _achievements(Achievements())
    {};

Decimal GameState::antimatter() {
    return this->_antimatter;
}

AntimatterDimensions& GameState::AD() {
    return this->_AD;
};

Achievements& GameState::achievements() {
    return this->_achievements;
};

void GameState::tick(double diff) {
    this->AD().update(*this);
    for (int i = 8; i >= 1; i--) {
        if (i == 1) {
            _AD[i].produceCurrency(_antimatter, diff);
        } else if (_AD[i].isUnlocked()) {
            _AD[i].produceDimensions(_AD[i - 1], diff / 10);
        };
    }
    for (Achievement& ach : this->achievements().achievements()) {
        if (ach.checkUnlockCondition(*this)) {
            ach.unlock();
        }
    }
};

bool GameState::buyOneDimension(int dim) {
    if (_antimatter >= _AD[dim].getCost()) {
        _antimatter -= _AD[dim].getCost();
        _AD[dim].onPurchase();
        return true;
    }
    return false;
}

Decimal GameState::getAchievementBonus() {
    Decimal result = DC::D1;
    int rowCounter = 0;
    int cycle = 0;
    for (Achievement& ach : this->achievements().achievements()) {
        if (ach.isUnlocked()) {
            result *= DC::D1_03;
            rowCounter++;
        }
        cycle++;
        if (cycle >= 8) {
            cycle = 0;
            if (rowCounter == 8) {
                result *= DC::D1_25;
            }
            rowCounter = 0;
        }
    }
    return result;
}
