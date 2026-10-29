#pragma once
#include "../gameinstance/gameinstance.h"
#include "../constants/constants.h"
#include <vector>
#include <cstdint>


class SearchNode {
    private:
        GameInstance instance;
        Decimal prevAD1 = DC::D1;
        Decimal nextSacrificeBranch = DC::D1;
        bool validated = false;
        std::vector<uint16_t> allowedPurchases;
        Decimal nextPurchaseCost = DC::D0;

    public:
        SearchNode(const GameState& st);

        // getters

        const GameInstance& gameInstance() const {return instance;}
        const GameState& game() const {return instance.game();}
        const InstructionQueue& queue() const {return instance.queue();}
        Decimal getPrevAD1() const {return prevAD1;}

        // instruction methods

        void addInstructions(const std::vector<double>& instructions) {instance.addInstructions(instructions);}
        void runNextInstructions() {if (instance.runNextInstructions()) validated = false;}

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
        
        // sacrifice branching methods
        
        bool canSacBranch() const;
        void setNextSacBranching(Decimal sacValue);
};
