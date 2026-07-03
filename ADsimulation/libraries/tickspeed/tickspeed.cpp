#include "tickspeed.h"
#include "../constants/constants.h"
#include "../gamestate/gamestate.h"


Tickspeed::Tickspeed() 
    :   cost(DC::D1E3),
        scaling(DC::D10),
        purchases(0),
        multiplier(DC::D1),
        unlocked(false)
    {};

Decimal Tickspeed::getTickspeedMultiplier(GameState& st) {
    return Decimal(1 / 1.1245);
}

Decimal Tickspeed::getDisplayMult(GameState& st) {
    return Decimal::invert(this->getTickspeedMultiplier(st));
}

void Tickspeed::update(GameState& st) {
    this->multiplier = Decimal::invert(
        Decimal::pow(getTickspeedMultiplier(st), purchases)
    );
}

Decimal Tickspeed::perSecond() {
    return this->multiplier;
}

Decimal Tickspeed::getCost() {
    return this->cost;
}

int Tickspeed::getPurchases() {
    return this->purchases;
}

bool Tickspeed::isUnlocked() {
    return this->unlocked;
}

void Tickspeed::unlock() {
    this->unlocked = true;
}

void Tickspeed::lock() {
    this->unlocked = false;
}

bool Tickspeed::canPurchase(Decimal resource) {
    return this->isUnlocked() && resource >= this->getCost();
}

void Tickspeed::onPurchase() {
    this->purchases++;
    this->cost *= this->scaling;
}
