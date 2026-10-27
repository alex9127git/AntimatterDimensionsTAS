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
        Tickspeed(const json& j);

        Decimal getTickspeedMultiplier(const GameState& st) const;
        Decimal getDisplayMult(const GameState& st) const;

        void update(const GameState& st);

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