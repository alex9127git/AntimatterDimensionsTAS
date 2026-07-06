#pragma once
#include "list"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"


template <typename T>
class GameStateProperty {
    private:
        T value;
        T& operator=(const T& newValue) {
            value = newValue;
            return value;
        }

    public:
        GameStateProperty(T _value) : value(_value) {};

        operator T() const {
            return get();
        }

        T& get() {
            return value;
        }
};


class GameState {
    private:
        Decimal _antimatter;
        AntimatterDimensions _AD;
        Tickspeed _tickspeed;
        Achievements _achievements;
        long tickCounter;
        long realTimePlayed;
        bool konamiCodeUsed;
        list<int> instructions;

    public:
        GameState();
        GameState(
            Decimal antimatter, 
            long tickCounter, 
            long realTimePlayed, 
            bool konamiCodeUsed, 
            list<int> startingInstructions,
            list<int> startingAchievements
        );

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