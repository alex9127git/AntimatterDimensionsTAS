#pragma once
#include <cstdint>
#include <vector>
#include <iosfwd>
#include "../instructions/instructions.h"
#include "../constants/constants.h"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../sacrifice/sacrifice.h"
#include "../achievements/achievements.h"
#include "../interfaces/interfaces.h"


class GameState : public ISerializable {
    private:
        Decimal _antimatter = DC::D10;
        Decimal _totalAntimatter = DC::D10;
        AntimatterDimensions _AD = AntimatterDimensions();
        Tickspeed _tickspeed = Tickspeed();
        Achievements _achievements = Achievements();
        Sacrifice _sacrifice = Sacrifice();
        int32_t _dimensionBoosts = 0;
        int64_t _realTimePlayed = 0;
        bool _canUseKonami = true;

        // volatile, shouldn't be serialized
        
        Decimal achievementBonus = DC::D1;

        Decimal _antimatterThisReset = DC::D10;

        InstructionQueue instructionQueue;

        Decimal nextPurchaseCost = DC::D0;
        std::vector<uint16_t> allowedPurchases;

        Decimal prevAD1 = DC::D1;
        Decimal nextSacrificeBranch = DC::D1;
        Decimal nextSacrificeInstruction = DC::D1;

        bool validated = false;

        void prepare();

        // recalculation methods

        void recalcAD();
        void recalcAD(int tier);
        void recalcTickspeed();
        void recalcAchievementBonus();

        // checks

        bool checkUnlockCondition(int id) const;

        // runner methods

        bool runInstruction(uint16_t instruction);
        bool runSacInstruction();

    public:
        GameState();
        GameState(const json& j);

        friend std::ostream& operator<<(std::ostream& os, const GameState& st);

        // getters

        Decimal antimatter() const {return _antimatter;}
        Decimal totalAntimatter() const {return _totalAntimatter;}
        Decimal antimatterThisReset() const {return _antimatterThisReset;}
        const AntimatterDimensions& AD() const {return _AD;}
        const Tickspeed& tickspeed() const {return _tickspeed;}
        const Achievements& achievements() const {return _achievements;}
        const Sacrifice& sacrifice() const {return _sacrifice;}
        const InstructionQueue& queue() const {return instructionQueue;}
        int64_t realTimePlayed() const {return _realTimePlayed;}
        int32_t dimensionBoosts() const {return _dimensionBoosts;}
        bool canUseKonami() const {return _canUseKonami;}
        Decimal getPrevAD1() const {return prevAD1;}
        Decimal costOf(int tier) const {return tier == 9 ? tickspeed().getCost() : AD()[tier].getCost();}

        void tick(double diff);

        // purchase methods

        bool buyOneDimension(int dim);
        bool buyDimUntil10(int dim);
        bool buyTickspeed();
        bool handleKonamiCode();
        void unlockAchievement(int ach, bool force);

        // getting cached data

        Decimal getAchievementBonus() const;

        // instruction methods

        void addInstructions(const std::vector<double>& instructions);
        void clearAllInstructions();
        void runNextInstructions();
        
        // brnaching methods

        Decimal getAntimatterGoalForDimboost() const;

        bool canBranch();
        const std::vector<uint16_t>& getPurchaseBranches() const;
        
        void setNextSacBranching(Decimal sacValue);
        bool canSacBranch() const;

        // reset methods

        bool canBuyNextDimboost() const;
        bool requestDimboost();
        bool canSacrifice() const;
        Decimal nextSacrificeBoost() const;
        bool sacrificeReset();

        // data & i/o manipulation

        json to_json() const override;
        void from_json(const json& j) override;
};