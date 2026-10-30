#include "tickspeed.h"
#include "constants.h"


Tickspeed::Tickspeed() 
    :   cost(DC::D1E3),
        scaling(DC::D10),
        purchases(0),
        unlocked(false),
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
