#include "gamestate.h"
#include "../constants/constants.h"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"


GameState::GameState()
    :   _antimatter(DC::D10),
        _AD(AntimatterDimensions()),
        tickspeed(Tickspeed())
    {};

Decimal GameState::antimatter() {
    return this->_antimatter;
}

AntimatterDimensions GameState::AD() {
    return this->_AD;
}

vector<AntimatterDimension> GameState::getVectorAD() {
    return _AD.getDims();
}

void GameState::tick(double diff) {
    // apply resources
    for (int i = 8; i >= 1; i--) {
        if (i == 1) {
            _AD[i].produceCurrency(_antimatter, diff);
        } else if (_AD[i].isUnlocked()) {
            _AD[i].produceDimensions(_AD[i - 1], diff / 10);
        };
    };
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
