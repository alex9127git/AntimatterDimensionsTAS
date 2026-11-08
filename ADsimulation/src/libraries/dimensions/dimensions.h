#pragma once
#include "interfaces.h"
#include "costmath.h"
#include "decimal.h"
#include <cstdint>
#include <array>


class Tickspeed;


class Dimension : public ISerializable {
    protected:
        ExponentialCostScaling costRule;
        Decimal amount;
        int32_t purchases;
        bool unlocked;

        // volatile, shouldn't be serialized
        Decimal cost;
        Decimal multiplier;

    public:
        Dimension(
            Decimal _cost, 
            Decimal _scaling,
            bool _unlocked
        );
        Dimension();
        
        void update(const Decimal& newMult);
        Decimal productionPerSecond() const;
        Decimal productionPerDiff(double diff) const;
        Decimal getCost() const;
        ExponentialCostScaling getCostRule() const;
        Decimal getAmount() const;
        Decimal getMult() const; 
        int32_t getPurchases() const;
        bool isUnlocked() const;

        void unlock();
        void resetAmount();
        bool canPurchase(const Decimal& resource) const;
        virtual void onPurchase();
        
        void produceDimensions(Dimension& dimension, double diff) const;
};

class AntimatterDimension : public Dimension {
    public:
        AntimatterDimension(
            Decimal _cost, 
            Decimal _scaling,
            bool _unlocked
        );
        
        Decimal productionPerSecond() const = delete;
        Decimal productionPerSecond(const Tickspeed& t) const;
        Decimal productionPerDiff(double diff) const = delete;
        Decimal productionPerDiff(const Tickspeed& t, double diff) const;
        void produceDimensions(Dimension& dimension, double diff) const = delete;
        void produceDimensions(AntimatterDimension& dimension, const Tickspeed& t, double diff) const;

        void onPurchase() override;

        json to_json() const override;
        void from_json(const json& j) override;
};

class AntimatterDimensions : ISerializable {
    private:
        std::array<AntimatterDimension, 8> dims;

    public:
        AntimatterDimensions();
        AntimatterDimensions(const json& j);

        const AntimatterDimension& operator[](int index) const;
        AntimatterDimension& operator[](int index);

        json to_json() const override;
        void from_json(const json& j) override;
};
