#include <cmath>
#include <algorithm>
#include <ostream>
#include <string>
#include "dimensions.h"
#include "../constants/constants.h"
#include "../gamestate/gamestate.h"


Dimension::Dimension(
    int _tier,
    Decimal _cost, 
    Decimal _scaling,
    bool _unlocked
)   :   tier(_tier),
        initialCost(_cost),
        cost(_cost),
        scaling(_scaling),
        amount(DC::D0),
        purchases(0),
        multiplier(DC::D1),
        unlocked(_unlocked) 
    {}

Dimension::Dimension() :
        tier(1),
        initialCost(DC::D10),
        cost(DC::D10),
        scaling(DC::D1E3),
        amount(DC::D0),
        purchases(0),
        multiplier(DC::D1),
        unlocked(true) 
    {}

void Dimension::update(const GameState& st) {
    this->multiplier = DC::D1;
}

Decimal Dimension::productionPerSecond() const {
    return this->amount * this->multiplier;
}

Decimal Dimension::productionPerDiff(double diff) const {
    return this->productionPerSecond() * Decimal(diff);
}

Decimal Dimension::getCost() const {
    return this->cost;
}

Decimal Dimension::getInitialCost() const {
    return this->initialCost;
}

Decimal Dimension::getScaling() const {
    return this->scaling;
}

Decimal Dimension::getAmount() const {
    return this->amount;
}

Decimal Dimension::getMult() const {
    return this->multiplier;
}

int32_t Dimension::getPurchases() const {
    return this->purchases;
}

bool Dimension::isUnlocked() const {
    return this->unlocked;
}

void Dimension::unlock() {
    this->unlocked = true;
}

void Dimension::resetAmount() {
    this->amount = DC::D0;
}

bool Dimension::canPurchase(const Decimal& resource) const {
    return this->isUnlocked() && resource >= this->getCost();
}

void Dimension::onPurchase() {

}

void Dimension::produceCurrency(Decimal& currency, double diff) const {
    currency += this->productionPerDiff(diff);
}

void Dimension::produceDimensions(Dimension& dimension, double diff) const {
    dimension.amount += this->productionPerDiff(diff);
}

AntimatterDimension::AntimatterDimension(
    int _tier,
    Decimal _cost, 
    Decimal _scaling,
    bool _unlocked
)   :   Dimension(_tier, _cost, _scaling, _unlocked)
    {}

ostream& operator<<(ostream& os, const AntimatterDimension& d) {
    string suffix;
    switch (d.tier) {
        case 1:
            suffix = "st";
            break;
        case 2:
            suffix = "nd";
            break;
        case 3:
            suffix = "rd";
            break;
        default:
            suffix = "th";
    }
    os << d.tier << suffix << " Antimatter Dimension (x" << d.multiplier << ") " << d.amount << endl;
    os << "Purchases: " << d.purchases << ", Cost: " << d.cost;
    return os;
}

void AntimatterDimension::update(const GameState& st) {
    Decimal mult = DC::D1;
    mult *= Decimal::pow(DC::D2, floor(purchases / 10));
    mult *= st.getAchievementBonus();
    mult *= Decimal::pow(DC::D2, max(0, st.dimensionBoosts() - tier + 1));
    mult *= st.tickspeed().perSecond();
    if (this->tier == 8) {
        mult *= st.sacrifice().getSacrificeBonus();
    }
    this->multiplier = mult;
}

void AntimatterDimension::onPurchase() {
    this->purchases++;
    this->amount += DC::D1;
    if (this->purchases % 10 == 0) {
        this->cost *= this->scaling;
    };
}

json AntimatterDimension::to_json() const {
    json j;
    j["tier"] = this->tier;
    j["initialCost"] = this->initialCost.to_json();
    j["scaling"] = this->scaling.to_json();
    j["amount"] = this->amount.to_json();
    j["purchases"] = this->purchases;
    j["unlocked"] = this->unlocked;
    return j;
}

void AntimatterDimension::from_json(const json& j) {
    readIfPresent(j, "initialCost", this->initialCost);
    readIfPresent(j, "scaling", this->scaling);
    readIfPresent(j, "amount", this->amount);
    readIfPresent(j, "purchases", this->purchases);
    readIfPresent(j, "unlocked", this->unlocked);
    this->cost = this->initialCost * Decimal::pow(this->scaling, floor(purchases / 10));
}


AntimatterDimensions::AntimatterDimensions() 
    :   dims({
            AntimatterDimension(1,   DC::D10,    DC::D1E3,     false),
            AntimatterDimension(2,   DC::D100,   DC::D1E4,     false),
            AntimatterDimension(3,   DC::D1E4,   DC::D1E5,     false),
            AntimatterDimension(4,   DC::D1E6,   DC::D1E6,     false),
            AntimatterDimension(5,   DC::D1E9,   DC::D1E8,     false),
            AntimatterDimension(6,   DC::D1E13,  DC::D1E10,    false),
            AntimatterDimension(7,   DC::D1E18,  DC::D1E12,    false),
            AntimatterDimension(8,   DC::D1E24,  DC::D1E15,    false)
        })
{
    (*this)[1].unlock();
}

AntimatterDimensions::AntimatterDimensions(const json& j) : AntimatterDimensions::AntimatterDimensions() {
    this->from_json(j);
}

const vector<AntimatterDimension>& AntimatterDimensions::getDims() const {
    return this->dims;
}

const Dimension& AntimatterDimensions::operator[](int index) const {
    return this->dims[index - 1];
}

Dimension& AntimatterDimensions::operator[](int index) {
    return this->dims[index - 1];
}

void AntimatterDimensions::update(const GameState& st) {
    for (int i = 8; i >= 1; i--) {
        (*this)[i].update(st);
    }
}

json AntimatterDimensions::to_json() const {
    json j;
    vector<json> jsons;
    for (const AntimatterDimension& dim : dims) {
        jsons.push_back(dim.to_json());
    }
    j["dims"] = jsons;
    return j;
}

void AntimatterDimensions::from_json(const json& j) {
    vector<json> jsons = j.at("dims");
    for (int i = 0; i < min(jsons.size(), dims.size()); i++) {
        dims[i].from_json(jsons[i]);
    }
}