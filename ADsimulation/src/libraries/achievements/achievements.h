#pragma once
#include "interfaces.h"
#include "constants.h"
#include "decimal.h"
#include <cstdint>
#include <vector>
#include <array>


class Achievements : public ISerializable {
    public:
        inline static constexpr int ROWS = 7;
        inline static constexpr int COUNT = ROWS * 8;

        inline static const std::vector<int> afterTickAchievements = {
            24, 31, 42, 43, 45, 46
        };

    private:
        std::array<uint8_t, ROWS> bitmasks = {};

    public:
        Achievements() = default;
        Achievements(const json& j);
        bool isUnlocked(int id) const;
        
        void unlockAchievement(int id);

        static int idToIndex(int id);
        static int indexToId(int index);
        int achCount() const;

        Decimal calcBonus() const;
        Decimal effectOrDefault(int id, Decimal arg=DC::D0) const;

        json to_json() const override;
        void from_json(const json& j) override;
};
