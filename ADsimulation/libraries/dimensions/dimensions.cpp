#include "dimensions.h"
#include "../constants/constants.h"


Dimension::Dimension(
    string _name,
    Decimal _cost, 
    Decimal _scaling,
    bool _unlocked
)   :   name(_name),
        multiplier(DC::D1), 
        cost(_cost), 
        scaling(_scaling),
        amount(DC::D0),
        purchases(DC::D0),
        unlocked(_unlocked) 
    {};

ostream& operator<<(ostream& os, const Dimension& d) {
    os << d.name << " (x" << d.multiplier << ") " << d.amount << endl;
    os << "Purchases: " << d.amount << ", Cost: " << d.cost;
    return os;
}

Decimal Dimension::productionPerSecond() {
    return DC::D0;
}

Decimal Dimension::productionPerDiff(double diff) {
    return this->productionPerSecond() * Decimal(diff);
};

Decimal Dimension::getCost() {
    return this->cost;
};

bool Dimension::isUnlocked() {
    return this->unlocked;
};

void Dimension::onPurchase() {};

void Dimension::produceCurrency(Decimal& currency, double diff) {
    currency += this->productionPerDiff(diff);
};

void Dimension::produceDimensions(Dimension& dimension, double diff) {
    dimension.amount += this->productionPerDiff(diff);
};

AntimatterDimension::AntimatterDimension(
    string _name,
    Decimal _cost, 
    Decimal _scaling,
    bool _unlocked
)   :   Dimension(_name, _cost, _scaling, _unlocked)
    {};

Decimal AntimatterDimension::productionPerSecond() {
    Decimal base = this->amount;
    base = base * this->multiplier;
    return base;
}

void AntimatterDimension::onPurchase() {
    this->purchases += DC::D1;
    this->amount += DC::D1;
}

AntimatterDimensions::AntimatterDimensions() 
    :   dims({
            AntimatterDimension("1st Antimatter Dimension",   DC::D10,    DC::D1E3,     true),
            AntimatterDimension("2nd Antimatter Dimension",   DC::D100,   DC::D1E4,     false),
            AntimatterDimension("3rd Antimatter Dimension",   DC::D1E4,   DC::D1E5,     false),
            AntimatterDimension("4th Antimatter Dimension",   DC::D1E6,   DC::D1E6,     false),
            AntimatterDimension("5th Antimatter Dimension",   DC::D1E9,   DC::D1E8,     false),
            AntimatterDimension("6th Antimatter Dimension",   DC::D1E13,  DC::D1E10,    false),
            AntimatterDimension("7th Antimatter Dimension",   DC::D1E18,  DC::D1E12,    false),
            AntimatterDimension("8th Antimatter Dimension",   DC::D1E24,  DC::D1E15,    false)
        })
    {};

vector<AntimatterDimension> AntimatterDimensions::getDims() {
    return this->dims;
}

Dimension& AntimatterDimensions::operator[](int index) {
    return this->dims[index - 1];
}