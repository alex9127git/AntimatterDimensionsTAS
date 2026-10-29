#pragma once
#include "../gamestate/gamestate.h"
#include "../constants/constants.h"
#include "../instructions/instructions.h"
#include <vector>


class GameInstance {
    private:
        GameState gameState;

        InstructionQueue _queue;
        Decimal nextSacrificeInstruction = DC::D1;

        bool runInstruction(uint16_t instruction);
        bool runSacInstruction();

    public:
        GameInstance() = default;
        GameInstance(const GameState& st);

        const GameState& game() const {return gameState;}
        GameState& game() {return gameState;}
        const InstructionQueue& queue() const {return _queue;}

        void addInstructions(const std::vector<double>& instructions);
        void clearAllInstructions();
        bool runNextInstructions();

        void tick(double diff) {gameState.tick(diff);}
};
