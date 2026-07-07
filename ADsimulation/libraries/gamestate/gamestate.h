#pragma once
#include <list>
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"


class GameState {
    private:
        Decimal _antimatter;
        AntimatterDimensions _AD;
        Tickspeed _tickspeed;
        Achievements _achievements;
        int _dimensionBoosts;
        long _tickCounter;
        long _realTimePlayed;
        bool _canUseKonami;

        list<int> instructions;
        list<int> completedInstructions;

    public:
        GameState();
        GameState(
            Decimal _antimatter,
            int _dimensionBoosts,
            long _tickCounter, 
            long _realTimePlayed, 
            bool _canUseKonami,
            vector<int> _startingAchievements
        );

        friend ostream& operator<<(ostream& os, GameState& st);

        Decimal antimatter();
        AntimatterDimensions& AD();
        Tickspeed& tickspeed();
        Achievements& achievements();
        bool canUseKonami();
        long realTimePlayed();

        void tick(double diff);

        bool buyOneDimension(int dim);
        bool buyDimUntil10(int dim);
        bool buyTickspeed();
        bool handleKonamiCode();

        Decimal getAchievementBonus();

        void addInstructions(vector<int> instructions);
        bool runInstruction(int instruction);
        void runNextInstructions();
        bool hasNextInstruction();
        list<int> getCompletedInstructions();
        int instructionsExecuted();

        GameState copy();
};