#pragma once
#include <vector>
#include "../decimal/decimal.h"
#include "../interfaces/interfaces.h"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;


class GameState;


class Dimension : public ISerializable {
    protected:
        int tier;
        Decimal cost;
        Decimal scaling;
        Decimal amount;
        int purchases;
        bool unlocked;

        // volatile, shouldn't be serialized
        Decimal multiplier;
        Decimal production;

    public:
        Dimension(
            int _tier,
            Decimal _cost, 
            Decimal _scaling,
            bool _unlocked
        );
        Dimension();
        
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
        AntimatterDimension(json& j);

        friend ostream& operator<<(ostream& os, const AntimatterDimension& d);
        void update(GameState& st) override;
        void onPurchase() override;

        json to_json() override;
        void from_json(json& j) override;
};

class AntimatterDimensions : ISerializable {
    private:
        vector<AntimatterDimension> dims;

    public:
        AntimatterDimensions();
        AntimatterDimensions(json& j);

        Dimension& operator[](int index);

        vector<AntimatterDimension>& getDims();

        void update(GameState& st);

        json to_json() override;
        void from_json(json& j) override;
};