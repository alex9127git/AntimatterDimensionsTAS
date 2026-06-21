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

AntimatterDimensions GameState::AD() {
    return this->_AD;
};

Achievements GameState::achievements() {
    return this->_achievements;
};

void GameState::tick(double diff) {
    // apply resources
    for (int i = 8; i >= 1; i--) {
        if (i == 1) {
            _AD[i].produceCurrency(_antimatter, diff);
        } else if (_AD[i].isUnlocked()) {
            _AD[i].produceDimensions(_AD[i - 1], diff / 10);
        };
    }
    for (Achievement ach : this->_achievements.achievements()) {
        if (ach.checkUnlockCondition(*this)) ach.unlock();
    }
};

bool GameState::buyOneDimension(int dim) {
    cout << _antimatter << " " << _AD[dim].getCost() << endl;
    if (_antimatter >= _AD[dim].getCost()) {
        _antimatter -= _AD[dim].getCost();
        _AD[dim].onPurchase();
        return true;
    }
    return false;
}
