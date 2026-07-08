#include <cmath>
#include "dimensions.h"
#include "../constants/constants.h"
#include "../gamestate/gamestate.h"


Dimension::Dimension(
    int _tier,
    Decimal _cost, 
    Decimal _scaling,
    bool _unlocked
)   :   tier(_tier),
        cost(_cost),
        scaling(_scaling),
        amount(DC::D0),
        purchases(0),
        multiplier(DC::D1),
        production(DC::D0),
        unlocked(_unlocked) 
    {}

void Dimension::update(GameState& st) {
    this->production = DC::D0;
    this->multiplier = DC::D1;
}

Decimal Dimension::productionPerSecond() {
    return this->production;
}

Decimal Dimension::productionPerDiff(double diff) {
    return this->productionPerSecond() * Decimal(diff);
}

Decimal Dimension::getCost() {
    return this->cost;
}

Decimal Dimension::getAmount() {
    return this->amount;
}

Decimal Dimension::getMult() {
    return this->multiplier;
}

int Dimension::getPurchases() {
    return this->purchases;
}

bool Dimension::isUnlocked() {
    return this->unlocked;
}

void Dimension::unlock() {
    this->unlocked = true;
}

void Dimension::lock() {
    this->unlocked = false;
}

bool Dimension::canPurchase(Decimal resource) {
    return this->isUnlocked() && resource >= this->getCost();
}

void Dimension::onPurchase() {

}

void Dimension::produceCurrency(Decimal& currency, double diff) {
    currency += this->productionPerDiff(diff);
}

void Dimension::produceDimensions(Dimension& dimension, double diff) {
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

void AntimatterDimension::update(GameState& st) {
    Decimal prod = this->amount;
    Decimal mult = DC::D1;
    mult *= Decimal::pow(DC::D2, floor(purchases / 10));
    mult *= st.getAchievementBonus();
    mult *= Decimal::pow(DC::D2, max(0, st.dimensionBoosts() - tier + 1));
    prod *= mult;
    prod *= st.tickspeed().perSecond();
    this->production = prod;
    this->multiplier = mult;
}

void AntimatterDimension::onPurchase() {
    this->purchases++;
    this->amount += DC::D1;
    if (this->purchases % 10 == 0) {
        this->cost *= this->scaling;
    };
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

vector<AntimatterDimension>& AntimatterDimensions::getDims() {
    return this->dims;
}

Dimension& AntimatterDimensions::operator[](int index) {
    return this->dims[index - 1];
}

void AntimatterDimensions::update(GameState& st) {
    for (int i = 8; i >= 1; i--) {
        (*this)[i].update(st);
    }
}