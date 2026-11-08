#include "dimensions.h"
#include "tickspeed.h"
#include "constants.h"
#include <algorithm>
using namespace std;


Dimension::Dimension(
    Decimal _cost, 
    Decimal _scaling,
    bool _unlocked
)   :   costRule(_cost, _scaling, DC::D10, DC::INF),
        amount(DC::D0),
        purchases(0),
        unlocked(_unlocked),
        cost(_cost),
        multiplier(DC::D1)
    {}

Dimension::Dimension()
    :   costRule(DC::D10, DC::D1E3, DC::D10, DC::INF),
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

ExponentialCostScaling Dimension::getCostRule() const {
    return this->costRule;
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

void Dimension::produceDimensions(Dimension& dimension, double diff) const {
    dimension.amount += this->productionPerDiff(diff);
}

AntimatterDimension::AntimatterDimension(
    Decimal _cost, 
    Decimal _scaling,
    bool _unlocked
)   :   Dimension(_cost, _scaling, _unlocked)
    {}

Decimal AntimatterDimension::productionPerSecond(const Tickspeed& t) const {
    return this->amount * this->multiplier * t.perSecond();
}

Decimal AntimatterDimension::productionPerDiff(const Tickspeed& t, double diff) const {
    return this->productionPerSecond(t) * Decimal(diff);
}

void AntimatterDimension::produceDimensions(AntimatterDimension& dimension, const Tickspeed& t, double diff) const {
    dimension.amount += this->productionPerDiff(t, diff);
}

void AntimatterDimension::onPurchase() {
    this->purchases++;
    this->amount += DC::D1;
    if (this->purchases % 10 == 0) {
        this->cost = costRule.getCost(purchases / 10);
    };
}

json AntimatterDimension::to_json() const {
    json j;
    j["amount"] = this->amount.to_json();
    j["purchases"] = this->purchases;
    j["unlocked"] = this->unlocked;
    return j;
}

void AntimatterDimension::from_json(const json& j) {
    readIfPresent(j, "amount", this->amount);
    readIfPresent(j, "purchases", this->purchases);
    readIfPresent(j, "unlocked", this->unlocked);
    this->cost = costRule.getCost(purchases / 10);
}


AntimatterDimensions::AntimatterDimensions() 
    :   dims({
            AntimatterDimension(DC::D10,    DC::D1E3,     false),
            AntimatterDimension(DC::D100,   DC::D1E4,     false),
            AntimatterDimension(DC::D1E4,   DC::D1E5,     false),
            AntimatterDimension(DC::D1E6,   DC::D1E6,     false),
            AntimatterDimension(DC::D1E9,   DC::D1E8,     false),
            AntimatterDimension(DC::D1E13,  DC::D1E10,    false),
            AntimatterDimension(DC::D1E18,  DC::D1E12,    false),
            AntimatterDimension(DC::D1E24,  DC::D1E15,    false)
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
