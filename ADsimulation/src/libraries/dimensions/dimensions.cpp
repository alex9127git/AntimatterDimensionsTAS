#include "dimensions.h"
#include "tickspeed.h"
#include "constants.h"
#include <algorithm>
#include <cmath>
using namespace std;


Dimension::Dimension(
    int _tier,
    Decimal _cost, 
    Decimal _scaling,
    bool _unlocked
)   :   tier(_tier),
        initialCost(_cost),
        scaling(_scaling),
        amount(DC::D0),
        purchases(0),
        unlocked(_unlocked),
        cost(_cost),
        multiplier(DC::D1)
    {}

Dimension::Dimension()
    :   tier(1),
        initialCost(DC::D10),
        scaling(DC::D1E3),
        amount(DC::D0),
        purchases(0),
        unlocked(true),
        cost(DC::D10),
        multiplier(DC::D1)
    {}

void Dimension::update(const Decimal& newMult) {
    this->multiplier = newMult;
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

Decimal AntimatterDimension::productionPerSecond(const Tickspeed& t) const {
    return this->amount * this->multiplier * t.perSecond();
}

Decimal AntimatterDimension::productionPerDiff(const Tickspeed& t, double diff) const {
    return this->productionPerSecond(t) * Decimal(diff);
}

void AntimatterDimension::produceCurrency(Decimal& currency, const Tickspeed& t, double diff) const {
    currency += this->productionPerDiff(t, diff);
}

void AntimatterDimension::produceDimensions(AntimatterDimension& dimension, const Tickspeed& t, double diff) const {
    dimension.amount += this->productionPerDiff(t, diff);
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

const AntimatterDimension& AntimatterDimensions::operator[](int index) const {
    return this->dims[index - 1];
}

AntimatterDimension& AntimatterDimensions::operator[](int index) {
    return this->dims[index - 1];
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
    for (size_t i = 0; i < min(jsons.size(), dims.size()); i++) {
        dims[i].from_json(jsons[i]);
    }
}