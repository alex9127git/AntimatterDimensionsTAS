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
        
        vector<double> purchaseInstructions;
        vector<double> sacrificeInstructions;
        vector<double> completedInstructions;
        vector<double> completedPurchases;
        vector<double> completedSacrifices;
        vector<Decimal> prices;
        Decimal nextPurchase;
        Decimal nextSacrifice;
        Decimal currPriceRange;
        Decimal achievementBonus;
        Decimal sacrificeBonus;

        void prepare();
        void calcNextPurchase();
        void recalcAchievementBonus();
        void recalcSacrificeBonus();
        Decimal getSacrificeExponent();

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
        Decimal getSacrificeBonus();

        // simulation methods

        void addInstructions(vector<double> instructions);
        bool runInstruction(double instruction);
        bool runSacInstruction(double instruction);
        void runNextInstructions();
        bool hasNextPurchaseInstruction();
        bool hasNextSacrificeInstruction();
        vector<double> getCompletedInstructions();
        vector<double> getCompletedPurchases();
        vector<double> getCompletedSacrifices();
        int instructionsExecuted();
        bool canBranch();
        void initializeSacBranching();
        void setNextSacBranching(double value);
        bool canSacBranch();
        Decimal getPriceRange();
        Decimal getAntimatterGoalForDimboost();

        // reset methods

        bool canBuyNextDimboost();
        bool requestDimboost();
        bool canSacrifice();
        Decimal nextSacrificeBoost();
        bool sacrificeReset();

        // data & i/o manipulation

        GameState copy();

        json to_json() override;
        void from_json(json& j) override;

        void writeInstructions(ofstream& f);
};