#pragma once
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"


class GameState {
    private:
        Decimal _antimatter;
        AntimatterDimensions _AD;
        Tickspeed _tickspeed;
        Achievements _achievements;
        long tickCounter;
        double realTimePlayed;
        bool konamiCodeUsed;

    public:
        GameState();

        friend ostream& operator<<(ostream& os, GameState& st);

        Decimal antimatter();
        AntimatterDimensions& AD();
        Tickspeed& tickspeed();
        Achievements& achievements();

        void tick(double diff);

        bool buyOneDimension(int dim);
        bool buyDimUntil10(int dim);
        bool buyTickspeed();

        Decimal getAchievementBonus();

        void handleKonamiCode();
};