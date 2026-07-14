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

Tickspeed::Tickspeed(json& j) : Tickspeed::Tickspeed() {
    this->from_json(j);
}

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

json Tickspeed::to_json() {
    json j;
    j["cost"] = this->cost.to_json();
    j["scaling"] = this->scaling.to_json();
    j["purchases"] = this->purchases;
    j["unlocked"] = this->unlocked;
    return j;
}

void Tickspeed::from_json(json& j) {
    this->cost = Decimal(j["cost"]);
    this->scaling = Decimal(j["scaling"]);
    this->purchases = j["purchases"];
    this->unlocked = j["unlocked"];
}
