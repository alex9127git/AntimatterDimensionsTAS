#include "instructions.h"
#include "decimal.h"
#include <ostream>
#include <format>
#include <string>
using namespace std;


void InstructionQueue::add(const vector<double>& instructions) {
    bool isSacrifice = false;
    for (double instruction : instructions) {
        if (isSacrifice) {
            sacrificeInstructions.push_back(instruction);
            isSacrifice = false;
        } else if (Instruction::isBulkInstruction(instruction)) {
            int type = instruction / 10;
            int times = (int) instruction % 10;
            times = times == 0 ? 10 : times;
            for (int i = 0; i < times; i++) {
                purchaseInstructions.push_back(type);
            }
        } else if (instruction == Instruction::Sacrifice) {
            isSacrifice = true;
        } else {
            purchaseInstructions.push_back(instruction);
        }
    }
}

void InstructionQueue::clearAllInstructions() {
    purchaseInstructions.clear();
    sacrificeInstructions.clear();
    purchaseIndex = 0;
    sacrificeIndex = 0;
}

bool InstructionQueue::hasNextPurchase() const {
    return purchaseIndex < purchaseInstructions.size();
}

bool InstructionQueue::hasNextSacrifice() const {
    return sacrificeIndex < sacrificeInstructions.size();
}

uint16_t InstructionQueue::peekNextPurchase() const {
    return purchaseInstructions[purchaseIndex];
}

double InstructionQueue::peekNextSacrifice() const {
    return sacrificeInstructions[sacrificeIndex];
}

void InstructionQueue::completePurchase() {
    completedInstructions.push_back(purchaseInstructions[purchaseIndex++]);
    if (purchaseIndex == purchaseInstructions.size()) {
        purchaseInstructions.clear();
        purchaseIndex = 0;
    }
}

void InstructionQueue::completeSacrifice() {
    completedInstructions.push_back(Instruction::Sacrifice);
    completedSacrifices.push_back(sacrificeInstructions[sacrificeIndex++]);
    if (sacrificeIndex == sacrificeInstructions.size()) {
        sacrificeInstructions.clear();
        sacrificeIndex = 0;
    }
}

const vector<uint16_t>& InstructionQueue::getCompletedInstructions() const {
    return completedInstructions;
}

vector<double> InstructionQueue::getCompletedPurchases() const {
    vector<double> purchases;
    for (double instruction : this->completedInstructions) {
        if (instruction != Instruction::Sacrifice) {
            purchases.push_back(instruction);
        }
    }
    return purchases;
}

const vector<double>& InstructionQueue::getCompletedSacrifices() const {
    return completedSacrifices;
}

void writeInstructions(ostream& f, const InstructionQueue& queue) {
    f << "[\n";
    const vector<uint16_t>& completed = queue.getCompletedInstructions();
    const vector<double>& sacrifices = queue.getCompletedSacrifices();
    int sacrificeIndex = 0;
    vector<string> lines;
    size_t i = 0;
    while (i < completed.size()) {
        uint16_t instruction = completed[i];
        size_t groupEnd = i + 1;
        if (Instruction::isDimension(instruction)) {
            while (groupEnd < completed.size() && completed[groupEnd] == instruction) groupEnd++;
            lines.push_back(format("[\"buyDimension\",[{},{}]]", instruction, groupEnd - i));
        } else if (instruction == Instruction::TickspeedSingle) {
            lines.push_back("[\"buyTickSpeed\",[0]]");
        } else if (instruction == Instruction::DimensionBoost) {
            lines.push_back("[\"buyDimensionBoost\",[0]]");
        } else if (instruction == Instruction::Sacrifice) {
            Decimal sacTarget = Decimal(1, sacrifices[sacrificeIndex++]);
            lines.push_back(format("[\"trySacrificeReset\",[\"{}\"]]", sacTarget.toString(10)));
        }
        i = groupEnd;
    }
    for (size_t i = 0; i < lines.size(); i++) {
        f << lines[i];
        if (i != lines.size() - 1) f << ",\n";
    }
    f << "\n]\n";
}
