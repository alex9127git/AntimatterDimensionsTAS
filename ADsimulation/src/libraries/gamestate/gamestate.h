#pragma once
#include <vector>
#include <fstream>
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"
#include "../interfaces/interfaces.h"


class GameState : public ISerializable {
    private:
        Decimal _antimatter;
        AntimatterDimensions _AD;
        Tickspeed _tickspeed;
        Achievements _achievements;
        int _dimensionBoosts;
        long _realTimePlayed;
        bool _canUseKonami;
        Decimal _sacrificed;

        // volatile, shouldn't be serialized
        
        vector<double> instructions;
        vector<double> completedInstructions;
        vector<Decimal> prices;
        Decimal nextPurchase;
        Decimal currPriceRange;
        Decimal achievementBonus;

        void prepare();
        void calcNextPurchase();
        void recalcAchievementBonus();

    public:
        GameState();
        GameState(json& j);

        friend ostream& operator<<(ostream& os, GameState& st);

        // getters

        Decimal antimatter();
        AntimatterDimensions& AD();
        Tickspeed& tickspeed();
        Achievements& achievements();
        bool canUseKonami();
        long realTimePlayed();
        int dimensionBoosts();

        void tick(double diff);

        // purchase methods

        bool buyOneDimension(int dim);
        bool buyDimUntil10(int dim);
        bool buyTickspeed();
        bool handleKonamiCode();
        void unlockAchievement(int ach, bool force);

        // getting cached data

        Decimal getAchievementBonus();

        // simulation methods

        void addInstructions(vector<int> instructions);
        bool runInstruction(int instruction);
        void runNextInstructions();
        bool hasNextInstruction();
        vector<double> getCompletedInstructions();
        int instructionsExecuted();
        bool canBranch();
        Decimal getPriceRange();

        // reset methods

        bool requestDimboost();
        bool canBuyNextDimboost();
        bool sacrificeReset();

        // data & i/o manipulation

        GameState copy();

        json to_json() override;
        void from_json(json& j) override;

        void writeInstructions(ofstream& f);
};