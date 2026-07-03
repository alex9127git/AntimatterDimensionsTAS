#pragma once
#include "../decimal/decimal.h"


class GameState;


class Tickspeed {
    private:
        Decimal cost;
        Decimal scaling;
        int purchases;
        Decimal multiplier;
        bool unlocked;
    
    public:
        Tickspeed();

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
};