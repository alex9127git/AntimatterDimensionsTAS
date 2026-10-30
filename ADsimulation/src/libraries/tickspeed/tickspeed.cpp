#include "tickspeed.h"
#include "constants.h"


Tickspeed::Tickspeed() 
    :   costRule(DC::D1E3, DC::D10, DC::D10, DC::INF),
        purchases(0),
        unlocked(false),
        cost(DC::D1E3),
        value(DC::D1E3),
        valuePerSecond(DC::D1)
    {};

Tickspeed::Tickspeed(const json& j) : Tickspeed::Tickspeed() {
    this->from_json(j);
}

void Tickspeed::update(const Decimal& newVal) {
    this->value = newVal;
    this->valuePerSecond = DC::D1E3 / this->value;
}

Decimal Tickspeed::perSecond() const {
    return this->valuePerSecond;
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
    this->cost = costRule.getCost(purchases);
}

json Tickspeed::to_json() const {
    json j;
    j["purchases"] = this->purchases;
    j["unlocked"] = this->unlocked;
    return j;
}

void Tickspeed::from_json(const json& j) {
    readIfPresent(j, "purchases", purchases);
    readIfPresent(j, "unlocked", unlocked);
    this->cost = costRule.getCost(purchases);
}
