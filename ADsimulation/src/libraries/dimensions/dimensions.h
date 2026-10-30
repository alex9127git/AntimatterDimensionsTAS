#pragma once
#include "interfaces.h"
#include "decimal.h"
#include <cstdint>
#include <vector>


class Tickspeed;


class Dimension : public ISerializable {
    protected:
        int tier;
        Decimal initialCost;
        Decimal scaling;
        Decimal amount;
        int32_t purchases;
        bool unlocked;

        // volatile, shouldn't be serialized
        Decimal cost;
        Decimal multiplier;

    public:
        Dimension(
            int _tier,
            Decimal _cost, 
            Decimal _scaling,
            bool _unlocked
        );
        Dimension();
        
        void update(const Decimal& newMult);
        Decimal productionPerSecond() const;
        Decimal productionPerDiff(double diff) const;
        Decimal getCost() const;
        Decimal getInitialCost() const;
        Decimal getScaling() const;
        Decimal getAmount() const;
        Decimal getMult() const; 
        int32_t getPurchases() const;
        bool isUnlocked() const;

        void unlock();
        void resetAmount();
        bool canPurchase(const Decimal& resource) const;
        virtual void onPurchase();
        
        void produceCurrency(Decimal& currency, double diff) const;
        void produceDimensions(Dimension& dimension, double diff) const;
};

class AntimatterDimension : public Dimension {
    public:
        AntimatterDimension(
            int _tier,
            Decimal _cost, 
            Decimal _scaling,
            bool _unlocked
        );
        
        Decimal productionPerSecond() const = delete;
        Decimal productionPerSecond(const Tickspeed& t) const;
        Decimal productionPerDiff(double diff) const = delete;
        Decimal productionPerDiff(const Tickspeed& t, double diff) const;
        void produceCurrency(Decimal& currency, double diff) const = delete;
        void produceCurrency(Decimal& currency, const Tickspeed& t, double diff) const;
        void produceDimensions(Dimension& dimension, double diff) const = delete;
        void produceDimensions(AntimatterDimension& dimension, const Tickspeed& t, double diff) const;

        void onPurchase() override;

        json to_json() const override;
        void from_json(const json& j) override;
};

class AntimatterDimensions : ISerializable {
    private:
        std::vector<AntimatterDimension> dims;

    public:
        AntimatterDimensions();
        AntimatterDimensions(const json& j);

        const AntimatterDimension& operator[](int index) const;
        AntimatterDimension& operator[](int index);

        json to_json() const override;
        void from_json(const json& j) override;
};