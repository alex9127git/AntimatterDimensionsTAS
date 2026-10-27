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

Tickspeed::Tickspeed(const json& j) : Tickspeed::Tickspeed() {
    this->from_json(j);
}

Decimal Tickspeed::getTickspeedMultiplier(const GameState& st) const {
    return Decimal(1 / 1.1245);
}

Decimal Tickspeed::getDisplayMult(const GameState& st) const {
    return Decimal::invert(this->getTickspeedMultiplier(st));
}

void Tickspeed::update(const GameState& st) {
    this->multiplier = Decimal::invert(
        Decimal::pow(getTickspeedMultiplier(st), purchases)
    );
}

Decimal Tickspeed::perSecond() const {
    return this->multiplier;
}

Decimal Tickspeed::getCost() const {
    return this->cost;
}

int32_t Tickspeed::getPurchases() const {
    return this->purchases;
}

bool Tickspeed::isUnlocked() const {
    return this->unlocked;
}

void Tickspeed::unlock() {
    this->unlocked = true;
}

bool Tickspeed::canPurchase(const Decimal& resource) const {
    return this->isUnlocked() && resource >= this->getCost();
}

void Tickspeed::onPurchase() {
    this->purchases++;
    this->cost *= this->scaling;
}

json Tickspeed::to_json() const {
    json j;
    j["cost"] = this->cost.to_json();
    j["scaling"] = this->scaling.to_json();
    j["purchases"] = this->purchases;
    j["unlocked"] = this->unlocked;
    return j;
}

void Tickspeed::from_json(const json& j) {
    this->cost = Decimal(j.at("cost"));
    this->scaling = Decimal(j.at("scaling"));
    this->purchases = j.at("purchases");
    this->unlocked = j.at("unlocked");
}
