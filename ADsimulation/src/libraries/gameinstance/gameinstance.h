#pragma once
#include "../gamestate/gamestate.h"
#include "../instructions/instructions.h"
#include <vector>


class GameInstance {
    private:
        GameState gameState;

        InstructionQueue _queue;
        Decimal nextSacrificeInstruction = DC::D1;

        // search

        bool validated = false;
        std::vector<uint16_t> allowedPurchases;
        Decimal nextPurchaseCost = DC::D0;

        bool runInstruction(uint16_t instruction);
        bool runSacInstruction();

    public:
        GameInstance() = default;
        GameInstance(const json& j);

        const GameState& game() const {return gameState;}
        GameState& game() {return gameState;}
        const InstructionQueue& queue() const {return _queue;}

        void addInstructions(const std::vector<double>& instructions);
        void clearAllInstructions();
        void runNextInstructions();

        void tick(double diff) {gameState.tick(diff);}

        // search

        bool canBranch();
        const std::vector<uint16_t>& getPurchaseBranches() const;
};
