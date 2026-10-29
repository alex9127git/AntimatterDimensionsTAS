#pragma once
#include "interfaces.h"
#include "decimal.h"
#include <cstdint>
#include <vector>


class Achievements : public ISerializable {
    private:
        std::vector<uint8_t> bitmasks;
        std::vector<int64_t> unlockTimes;

    public:
        inline static const std::vector<int> afterTickAchievements = {
            24, 31, 42, 43, 45, 46
        };

        Achievements();
        Achievements(const json& j);
        bool isUnlocked(int id) const;
        
        void unlockAchievement(int id, int64_t time);
        void lockAchievement(int id);

        static int idToIndex(int id);
        static int indexToId(int index);
        int achCount() const;

        Decimal calcBonus() const;

        json to_json() const override;
        void from_json(const json& j) override;
};
