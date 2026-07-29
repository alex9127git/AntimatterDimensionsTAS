#pragma once
#include <cstdint>
#include <vector>
#include <fstream>
#include "../constants/constants.h"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"
#include "../interfaces/interfaces.h"


class GameState : public ISerializable {
    private:
        Decimal _antimatter = DC::D10;
        AntimatterDimensions _AD = AntimatterDimensions();
        Tickspeed _tickspeed = Tickspeed();
        Achievements _achievements = Achievements();
        int32_t _dimensionBoosts = 0;
        int64_t _realTimePlayed = 0;
        bool _canUseKonami = true;
        Decimal _sacrificed = DC::D1;

        // volatile, shouldn't be serialized
        
        Decimal achievementBonus = DC::D1;
        Decimal sacrificeBonus = DC::D1;

        vector<double> purchaseInstructions;
        vector<double> sacrificeInstructions;
        vector<double> completedInstructions;
        vector<double> completedPurchases;
        vector<double> completedSacrifices;
        vector<Decimal> prices;
        Decimal nextPurchaseBranch = DC::D0;
        Decimal nextSacrificeBranch = DC::D1;
        Decimal nextSacrificeInstruction = DC::D1;
        Decimal currPriceRange = DC::D1;

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
        int64_t realTimePlayed();
        int32_t dimensionBoosts();

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
        void setNextSacBranching(double sacValue);
        Decimal getReqForSac(Decimal sacValue);
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