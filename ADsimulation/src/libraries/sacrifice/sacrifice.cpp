#include "sacrifice.h"
#include <algorithm>
using namespace std;


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
            this->sacrificeExponent
        ), DC::D1);
    }
}

Decimal Sacrifice::sacrificeIncrease(const Decimal& AD1amount) const {
    Decimal a = (_sacrificed + AD1amount).log10() / DC::D10;
    Decimal b = max(max(_sacrificed, DC::D1).log10() / DC::D10, DC::D1);
    Decimal c = a / b;
    Decimal e = sacrificeExponent;
    return Decimal::pow(c, e);
}

Decimal Sacrifice::getReqForSac(const Decimal& sacValue) const {
    return Decimal::pow(
        10, 
        Decimal::pow(sacValue, DC::D1 / sacrificeExponent) * 10
    ) - _sacrificed;
}

void Sacrifice::increaseSacrifice(const Decimal& amount) {
    _sacrificed += amount;
    sacrificesDone++;
    this->recalcSacrificeBonus();
}

void Sacrifice::updateExponent(const Decimal& newExponent) {
    sacrificeExponent = newExponent;
    recalcSacrificeBonus();
}

json Sacrifice::to_json() const {
    return _sacrificed.to_json();
}

void Sacrifice::from_json(const json& j) {
    _sacrificed = Decimal(j);
    recalcSacrificeBonus();
}
