#pragma once
#include "../decimal/decimal.h"
#include "../constants/constants.h"
#include "../interfaces/interfaces.h"


class Sacrifice : public ISerializable {
    private:
        Decimal _sacrificed = DC::D0;

        // volatile, shouldn't be serialized

        int sacrificesDone = 0;
        Decimal sacrificeBonus = DC::D1;

        void recalcSacrificeBonus();

    public:
        Sacrifice();
        Sacrifice(const json& j);

        Decimal getSacrificeExponent() const;
        Decimal getSacrificeBonus() const {return sacrificeBonus;}
        Decimal getSacrificedAmount() const {return _sacrificed;}
        int getSacrificesDone() const {return sacrificesDone;}

        Decimal nextSacrificeBoost(const Decimal& AD1amount) const;
        Decimal getReqForSac(const Decimal& sacValue) const;

        void increaseSacrifice(const Decimal& amount);

        json to_json() const override;
        void from_json(const json& j) override;
};
