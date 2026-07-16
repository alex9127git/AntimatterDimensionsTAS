#pragma once
#include "../interfaces/interfaces.h"
#include "../decimal/decimal.h"


class GameState;


class Tickspeed : public ISerializable {
    private:
        Decimal cost;
        Decimal scaling;
        int purchases;
        bool unlocked;

        // volatile, shouldn't be serialized
        Decimal multiplier;
    
    public:
        Tickspeed();
        Tickspeed(json& j);

        Decimal getTickspeedMultiplier(GameState& st);
        Decimal getDisplayMult(GameState& st);

        void update(GameState& st);
        Decimal perSecond();
        Decimal getCost();
        int getPurchases();
        bool isUnlocked();
        void unlock();
        void lock();
        bool canPurchase(Decimal resource);
        void onPurchase();

        json to_json() override;
        void from_json(json& j) override;
};