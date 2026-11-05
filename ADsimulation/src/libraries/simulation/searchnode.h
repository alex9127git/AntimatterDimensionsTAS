#pragma once
#include "gameinstance.h"
#include "instructions.h"
#include "constants.h"
#include <cstdint>
#include <vector>


class SearchNode {
    private:
        GameInstance instance;
        Decimal prevAD1 = DC::D1;
        Decimal nextSacrificeValue = DC::D1;
        Decimal nextSacrificeBranch = DC::D1;
        bool validated = false;
        std::vector<uint16_t> allowedPurchases;
        Decimal nextPurchaseCost = DC::D0;

        int sacsAtBranchSet = 0;
        size_t projRouteIndex = 0;
        std::vector<double> sacrificeDiffs;

    public:
        SearchNode(const GameState& st);

        // getters

        const GameInstance& gameInstance() const {return instance;}
        const GameState& game() const {return instance.game();}
        const InstructionQueue& queue() const {return instance.queue();}
        Decimal getPrevAD1() const {return prevAD1;}
        Decimal costOf(uint16_t code) const {
            return code == Instruction::TickspeedSingle ? 
                        game().tickspeed().getCost() : game().AD()[code].getCost();
        }
        size_t getProjRouteIndex() const {return projRouteIndex;}

        // instruction methods

        void addInstructions(const std::vector<double>& instructions) {instance.addInstructions(instructions);}
        void runNextInstructions() {if (instance.runNextInstructions()) validated = false;}
        void runNextPurchases() {if (instance.runNextPurchases()) validated = false;}

        void tick(double diff) {
            prevAD1 = instance.game().AD()[1].getAmount();
            instance.tick(diff);
        }

        // purchase branching methods

        bool canBranch();
    private: 
        void computeAllowedPurchases();
    public:
        const std::vector<uint16_t>& getPurchaseBranches() const;
        Decimal getNextSacValue() const {return nextSacrificeValue;}
        
        // sacrifice branching methods
        
        bool canSacBranch();
        void initProjRoute(size_t index) {projRouteIndex = index;}
        void setNextSacBranching(Decimal sacValue);
        const std::vector<double>& getSacrificeDiffs() const {return sacrificeDiffs;}
        void appendSacrificeDiff(double diff) {sacrificeDiffs.push_back(diff);}
};
