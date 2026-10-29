#include "gameinstance.h"
#include "search.h"
using namespace std;


GameInstance::GameInstance(const json& j)
    :   gameState(j)
    {}

void GameInstance::addInstructions(const vector<double>& instructions) {
    _queue.add(instructions);
    if (queue().hasNextSacrifice()) {
        this->nextSacrificeInstruction = gameState.sacrifice().getReqForSac(
            Decimal(1, queue().peekNextSacrifice())
        );
    }
}

void GameInstance::clearAllInstructions() {
    _queue.clearAllInstructions();
}

void GameInstance::runNextInstructions() {
    while (queue().hasNextPurchase() && this->runInstruction(queue().peekNextPurchase())) {
        _queue.completePurchase();
        validated = false;
    }
    while (queue().hasNextSacrifice() && this->runSacInstruction()) {
        _queue.completeSacrifice();
        validated = false;
        if (queue().hasNextSacrifice()) {
            this->nextSacrificeInstruction = gameState.sacrifice().getReqForSac(
                Decimal(1, queue().peekNextSacrifice())
            );
        }
    }
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

bool GameInstance::canBranch() {
    if (!validated) {
        allowedPurchases = computeAllowedPurchases(gameState);
        nextPurchaseCost = gameState.costOf(allowedPurchases.front());
        validated = true;
    }
    return gameState.antimatter() >= nextPurchaseCost;
}

const vector<uint16_t>& GameInstance::getPurchaseBranches() const {
    return this->allowedPurchases;
}