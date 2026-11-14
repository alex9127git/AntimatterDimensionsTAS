#include "infinity.h"


Infinity::Infinity(const json& j) {
    from_json(j);
}

void Infinity::addCurrency(Decimal gainedInfinities, Decimal gainedInfinityPoints) {
    this->_infinities += gainedInfinities;
    this->_infinityPoints += gainedInfinityPoints;
}

json Infinity::to_json() const {
    json j;
    j["infinities"] = this->_infinities.to_json();
    j["infinityPoints"] = this->_infinityPoints.to_json();
    j["upgradesBitmask"] = this->upgradesBitmask;
    return j;
}

void Infinity::from_json(const json& j) {
    readIfPresent(j, "infinities", this->_infinities);
    readIfPresent(j, "infinityPoints", this->_infinityPoints);
    readIfPresent(j, "upgradesBitmask", this->upgradesBitmask);
}
