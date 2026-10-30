#pragma once
#include "achievements.h"
#include "interfaces.h"
#include "dimensions.h"
#include "tickspeed.h"
#include "sacrifice.h"
#include "constants.h"
#include <cstdint>
#include <iosfwd>


class GameState : public ISerializable {
    private:
        Decimal _antimatter = DC::D10;
        Decimal _totalAntimatter = DC::D10;
        AntimatterDimensions _AD = AntimatterDimensions();
        Tickspeed _tickspeed = Tickspeed();
        Achievements _achievements = Achievements();
        Sacrifice _sacrifice = Sacrifice();
        int32_t _dimensionBoosts = 0;
        int32_t _antimatterGalaxies = 0;
        int64_t _realTimePlayed = 0;
        bool _canUseKonami = true;

        // volatile, shouldn't be serialized
        
        Decimal achievementBonus = DC::D1;
        Decimal _antimatterThisReset = DC::D10;

        void prepare();

        // recalculation methods

        void recalcAD();
        void recalcAD(int tier);
        void recalcTickspeed();
        void recalcAchievementBonus();

        // checks

        bool checkUnlockCondition(int id) const;

    public:
        GameState();
        GameState(const json& j);

        // getters

        Decimal antimatter() const {return _antimatter;}
        Decimal totalAntimatter() const {return _totalAntimatter;}
        Decimal antimatterThisReset() const {return _antimatterThisReset;}
        const AntimatterDimensions& AD() const {return _AD;}
        const Tickspeed& tickspeed() const {return _tickspeed;}
        const Achievements& achievements() const {return _achievements;}
        const Sacrifice& sacrifice() const {return _sacrifice;}
        int64_t realTimePlayed() const {return _realTimePlayed;}
        int32_t dimensionBoosts() const {return _dimensionBoosts;}
        int32_t antimatterGalaxies() const {return _antimatterGalaxies;}
        bool canUseKonami() const {return _canUseKonami;}

        void tick(double diff);

        // purchase methods

        bool buyOneDimension(int dim);
        bool buyDimUntil10(int dim);
        bool buyTickspeed();
        bool handleKonamiCode();
        void unlockAchievement(int ach, bool force);

        // getting cached data

        Decimal getAchievementBonus() const;

        // reset-adjacent methods

        Decimal getAntimatterGoalForDimboost() const;
        Decimal getAntimatterGoalForGalaxy() const;
        bool canBuyNextDimboost() const;
        bool requestDimboost();
        bool canSacrifice() const;
    private:
        Decimal sacrificeIncrease() const;
    public:
        Decimal getBoostAfterSacrifice() const;
        bool sacrificeReset();

        // data & i/o manipulation

        json to_json() const override;
        void from_json(const json& j) override;
};