#pragma once
#include "interfaces.h"
#include "constants.h"
#include "decimal.h"
#include <cstdint>


class Infinity : public ISerializable {
    private:
        Decimal _infinities = DC::D0;
        Decimal _infinityPoints = DC::D0;
        uint16_t upgradesBitmask = 0;

    public:
        Infinity() = default;
        Infinity(const json& j);

        Decimal infinities() const {return _infinities;}
        Decimal infinityPoints() const {return _infinityPoints;}

        void addCurrency(Decimal gainedInfinities, Decimal gainedInfinityPoints);

        json to_json() const override;
        void from_json(const json& j) override;
};
