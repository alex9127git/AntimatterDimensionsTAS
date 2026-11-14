#pragma once
#include "interfaces.h"
#include "constants.h"
#include "costmath.h"
#include "decimal.h"
#include <cstdint>


class Tickspeed : public ISerializable {
    private:
        ExponentialCostScaling costRule {DC::D1E3, DC::D10, DC::D10, DC::INF};
        int32_t purchases = 0;
        bool unlocked = false;

        // volatile, shouldn't be serialized

        Decimal cost = DC::D1E3;
        Decimal value = DC::D1E3;
        Decimal valuePerSecond = DC::D1;
    
    public:
        Tickspeed() = default;
        Tickspeed(const json& j);

        void update(const Decimal& newVal);

        Decimal perSecond() const;
        Decimal getCost() const;
        int32_t getPurchases() const;
        bool isUnlocked() const;

        void unlock();
        bool canPurchase(const Decimal& resource) const;
        void onPurchase();

        json to_json() const override;
        void from_json(const json& j) override;
};
