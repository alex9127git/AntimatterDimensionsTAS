#pragma once
#include <cstdint>
#include "../interfaces/interfaces.h"
#include "../decimal/decimal.h"


class GameState;


class Tickspeed : public ISerializable {
    private:
        Decimal cost;
        Decimal scaling;
        int32_t purchases;
        bool unlocked;

        // volatile, shouldn't be serialized
        Decimal multiplier;
    
    public:
        Tickspeed();
        Tickspeed(json& j);

        Decimal getTickspeedMultiplier(GameState& st);
        Decimal getDisplayMult(GameState& st);

        void update(GameState& st);

        Decimal perSecond() const;
        Decimal getCost() const;
        int32_t getPurchases() const;
        bool isUnlocked() const;

        void unlock();
        void lock();
        bool canPurchase(Decimal resource);
        void onPurchase();

        json to_json() override;
        void from_json(json& j) override;
};