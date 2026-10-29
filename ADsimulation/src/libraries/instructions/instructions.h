#pragma once
#include <cstdint>
#include <vector>
#include <iosfwd>


namespace Instruction {
    constexpr bool isDimension(double code) { return 1 <= code && code <= 8; }
    constexpr bool isBulkInstruction(double code) { return 10 <= code && code <= 99; }

    constexpr int bulkTier(double code) { return (int) code / 10; }
    constexpr int bulkCount(double code) { int m = (int) code % 10; return m == 0 ? 10 : m; }
    constexpr uint16_t bulk(int tier, int count) { return tier * 10 + count % 10; }

    constexpr uint16_t TickspeedSingle = 9;
    constexpr uint16_t DimensionBoost = 100;
    constexpr uint16_t Sacrifice = 108;
    constexpr uint16_t KonamiCode = 130;
}

class InstructionQueue {
    private:
        std::vector<uint16_t> purchaseInstructions;
        std::vector<double> sacrificeInstructions;
        std::vector<uint16_t> completedInstructions;
        std::vector<double> completedSacrifices;
        size_t purchaseIndex = 0;
        size_t sacrificeIndex = 0;

    public:
        void add(const std::vector<double>& instructions);
        void clearAllInstructions();
        bool hasNextPurchase() const;
        bool hasNextSacrifice() const;
        uint16_t peekNextPurchase() const;
        double peekNextSacrifice() const;
        void completePurchase();
        void completeSacrifice();
        const std::vector<uint16_t>& getCompletedInstructions() const;
        std::vector<double> getCompletedPurchases() const;
        const std::vector<double>& getCompletedSacrifices() const;
};

void writeInstructions(std::ostream& f, const InstructionQueue& queue);