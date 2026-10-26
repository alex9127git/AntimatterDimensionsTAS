#pragma once
#include <cstdint>
#include <iosfwd>
#include <vector>
#include "../decimal/decimal.h"
#include "../interfaces/interfaces.h"


class GameState;


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
        
        virtual void update(const GameState& st);
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
        void lock();
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

        friend ostream& operator<<(ostream& os, const AntimatterDimension& d);
        void update(const GameState& st) override;
        void onPurchase() override;

        json to_json() const override;
        void from_json(const json& j) override;
};

class AntimatterDimensions : ISerializable {
    private:
        vector<AntimatterDimension> dims;

    public:
        AntimatterDimensions();
        AntimatterDimensions(const json& j);

        const Dimension& operator[](int index) const;
        Dimension& operator[](int index);

        const vector<AntimatterDimension>& getDims() const;

        void update(const GameState& st);

        json to_json() const override;
        void from_json(const json& j) override;
};