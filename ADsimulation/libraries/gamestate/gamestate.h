#pragma once
#include "list"
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
        list<int> instructions;

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
        bool handleKonamiCode();

        Decimal getAchievementBonus();

        void addInstructions(list<int> instructions);
        bool runInstruction(int instruction);
        void runNextInstructions();
};