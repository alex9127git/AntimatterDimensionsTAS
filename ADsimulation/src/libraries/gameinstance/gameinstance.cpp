#include "gameinstance.h"
using namespace std;


GameInstance::GameInstance(const GameState& st)
    :   gameState(st)
    {}

void GameInstance::addInstructions(const vector<double>& instructions) {
    _queue.add(instructions);
    if (queue().hasNextSacrifice()) setNextSacrificeInstruction();
}

void GameInstance::clearAllInstructions() {
    _queue.clearAllInstructions();
}

bool GameInstance::runNextInstructions() {
    bool ranInstruction = false;
    while (queue().hasNextPurchase() && this->runInstruction(queue().peekNextPurchase())) {
        _queue.completePurchase();
        ranInstruction = true;
    }
    while (queue().hasNextSacrifice() && this->runSacInstruction()) {
        _queue.completeSacrifice();
        ranInstruction = true;
        if (queue().hasNextSacrifice()) setNextSacrificeInstruction();
    }
    return ranInstruction;
}

void GameInstance::setNextSacrificeInstruction() {
    this->nextSacrificeInstruction = gameState.sacrifice().getReqForSac(
        Decimal(1, queue().peekNextSacrifice())
    );
}

bool GameInstance::runInstruction(uint16_t instruction) {
    if (Instruction::isDimension(instruction)) {
        return gameState.buyOneDimension(instruction);
    } else if (instruction == Instruction::TickspeedSingle) {
        return gameState.buyTickspeed();
    } else if (instruction == Instruction::DimensionBoost) {
        return gameState.requestDimboost();
    } else if (instruction == Instruction::KonamiCode) {
        return gameState.handleKonamiCode();
    } else {
        return false;
    }
}

bool GameInstance::runSacInstruction() {
    if (gameState.AD()[1].getAmount() < this->nextSacrificeInstruction) return false;
    return gameState.sacrificeReset();
}