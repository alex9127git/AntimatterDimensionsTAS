#pragma once
#include <cstdint>
#include <vector>
#include <iosfwd>
#include "../instructions/instructions.h"
#include "../constants/constants.h"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"
#include "../interfaces/interfaces.h"


class GameState : public ISerializable {
    private:
        Decimal _antimatter = DC::D10;
        Decimal _totalAntimatter = DC::D10;
        AntimatterDimensions _AD = AntimatterDimensions();
        Tickspeed _tickspeed = Tickspeed();
        Achievements _achievements = Achievements();
        int32_t _dimensionBoosts = 0;
        int64_t _realTimePlayed = 0;
        bool _canUseKonami = true;
        Decimal _sacrificed = DC::D0;

        // volatile, shouldn't be serialized
        
        Decimal achievementBonus = DC::D1;
        Decimal sacrificeBonus = DC::D1;

        Decimal _antimatterThisReset = DC::D10;

        InstructionQueue instructionQueue;

        Decimal nextPurchaseCost = DC::D0;
        vector<uint16_t> allowedPurchases;

        int sacrificesDone = 0;
        Decimal prevAD1 = DC::D1;
        Decimal nextSacrificeBranch = DC::D1;
        Decimal nextSacrificeInstruction = DC::D1;

        void prepare();
        void calcPurchases();
        void recalcAchievementBonus();
        void recalcSacrificeBonus();
        Decimal getSacrificeExponent() const;

        bool runInstruction(uint16_t instruction);
        bool runSacInstruction();

    public:
        GameState();
        GameState(const json& j);

        friend ostream& operator<<(ostream& os, const GameState& st);

        // getters

        Decimal antimatter() const {return _antimatter;}
        Decimal totalAntimatter() const {return _totalAntimatter;}
        Decimal antimatterThisReset() const {return _antimatterThisReset;}
        const AntimatterDimensions& AD() const {return _AD;}
        const Tickspeed& tickspeed() const {return _tickspeed;}
        const Achievements& achievements() const {return _achievements;}
        const InstructionQueue& queue() const {return instructionQueue;}
        int64_t realTimePlayed() const {return _realTimePlayed;}
        int32_t dimensionBoosts() const {return _dimensionBoosts;}
        bool canUseKonami() const {return _canUseKonami;}
        Decimal getPrevAD1() const {return prevAD1;}

        void tick(double diff);

        // purchase methods

        bool buyOneDimension(int dim);
        bool buyDimUntil10(int dim);
        bool buyTickspeed();
        bool handleKonamiCode();
        void unlockAchievement(int ach, bool force);

        // getting cached data

        Decimal getAchievementBonus() const;
        Decimal getSacrificeBonus() const;

        // instruction methods

        void addInstructions(const vector<double>& instructions);
        void clearAllInstructions();
        void runNextInstructions();
        
        // brnaching methods

        Decimal getAntimatterGoalForDimboost() const;

        bool canBranch() const;
        const vector<uint16_t>& getPurchaseBranches() const;
        
        void initializeSacBranching();
        void setNextSacBranching(Decimal sacValue);
        Decimal getReqForSac(const Decimal& sacValue) const;
        bool canSacBranch() const;

        // reset methods

        bool canBuyNextDimboost() const;
        bool requestDimboost();
        bool canSacrifice() const;
        Decimal nextSacrificeBoost() const;
        Decimal nextSacrificeBoost(const Decimal& AD1amount) const;
        bool sacrificeReset();
        int getSacrificesDone() const;

        // data & i/o manipulation

        json to_json() const override;
        void from_json(const json& j) override;

        void writeInstructions(ofstream& f);
};