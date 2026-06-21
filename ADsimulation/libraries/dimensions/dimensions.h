#pragma once
#include <vector>
#include "../decimal/decimal.h"
using namespace std;


class Dimension {
    protected:
        string name;
        Decimal scaling;
        Decimal multiplier;
        Decimal cost;
        Decimal amount;
        Decimal purchases;
        bool unlocked;

    public:
        Dimension(
            string _name,
            Decimal _cost, 
            Decimal _scaling,
            bool _unlocked
        );

        friend ostream& operator<<(ostream& os, const Dimension& d);
        
        virtual Decimal productionPerSecond();
        Decimal productionPerDiff(double diff);
        Decimal getCost();
        bool isUnlocked();
        virtual void onPurchase();
        
        void produceCurrency(Decimal& currency, double diff);
        void produceDimensions(Dimension& dimension, double diff);
};

class AntimatterDimension : public Dimension {
    public:
        AntimatterDimension(
            string _name,
            Decimal _cost, 
            Decimal _scaling,
            bool _unlocked
        );

        Decimal productionPerSecond() override;
        void onPurchase() override;
};

class AntimatterDimensions {
    private:
        vector<AntimatterDimension> dims;

    public:
        AntimatterDimensions();

        Dimension& operator[](int index);

        vector<AntimatterDimension> getDims();
};