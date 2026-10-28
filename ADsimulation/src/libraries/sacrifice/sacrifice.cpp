#include "sacrifice.h"


Sacrifice::Sacrifice() {
    recalcSacrificeBonus();
}

Sacrifice::Sacrifice(const json& j) {
    from_json(j);
}

void Sacrifice::recalcSacrificeBonus() {
    if (_sacrificed == DC::D0) {
        this->sacrificeBonus = DC::D1;
    } else {
        this->sacrificeBonus = max(Decimal::pow(
            Decimal(_sacrificed.log10()) / DC::D10,
            this->getSacrificeExponent()
        ), DC::D1);
    }
}

Decimal Sacrifice::getSacrificeExponent() const {
    return DC::D2;
}

Decimal Sacrifice::nextSacrificeBoost(const Decimal& AD1amount) const {
    Decimal a = (_sacrificed + AD1amount).log10() / DC::D10;
    Decimal b = Decimal::max(Decimal::max(_sacrificed, DC::D1).log10() / DC::D10, DC::D1);
    Decimal c = a / b;
    Decimal e = getSacrificeExponent();
    return Decimal::pow(c, e);
}

Decimal Sacrifice::getReqForSac(const Decimal& sacValue) const {
    return Decimal::pow(
        10, 
        Decimal::pow(sacValue, DC::D1 / getSacrificeExponent()) * 10
    ) - _sacrificed;
}

void Sacrifice::increaseSacrifice(const Decimal& amount) {
    _sacrificed += amount;
    sacrificesDone++;
    this->recalcSacrificeBonus();
}

json Sacrifice::to_json() const {
    return _sacrificed.to_json();
}

void Sacrifice::from_json(const json& j) {
    _sacrificed = Decimal(j);
    recalcSacrificeBonus();
}
