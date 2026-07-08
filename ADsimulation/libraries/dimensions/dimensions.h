#pragma once
#include <vector>
#include "../decimal/decimal.h"
using namespace std;


class GameState;


class Dimension {
    protected:
        int tier;
        Decimal cost;
        Decimal scaling;
        Decimal amount;
        int purchases;
        Decimal multiplier;
        Decimal production;
        bool unlocked;

    public:
        Dimension(
            int _tier,
            Decimal _cost, 
            Decimal _scaling,
            bool _unlocked
        );
        
        virtual void update(GameState& st);
        Decimal productionPerSecond();
        Decimal productionPerDiff(double diff);
        Decimal getCost();
        Decimal getAmount();
        Decimal getMult();
        int getPurchases();
        bool isUnlocked();
        void unlock();
        void lock();
        bool canPurchase(Decimal resource);
        virtual void onPurchase();
        
        void produceCurrency(Decimal& currency, double diff);
        void produceDimensions(Dimension& dimension, double diff);
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
        void update(GameState& st) override;
        void onPurchase() override;
};

class AntimatterDimensions {
    private:
        vector<AntimatterDimension> dims;

    public:
        AntimatterDimensions();

        Dimension& operator[](int index);

        vector<AntimatterDimension>& getDims();

        void update(GameState& st);
};