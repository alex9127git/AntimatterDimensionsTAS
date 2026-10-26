#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include "../decimal/decimal.h"
#include "../interfaces/interfaces.h"
using namespace std;


class GameState;


struct AchConstants {
    static const function<Decimal(const Decimal&)> NO_EFFECT;
    static const vector<function<Decimal(const Decimal&)>> effectFormulae;
    static const function<bool(const GameState&)> NOT_APPLICABLE;
    static const function<bool(const GameState&)> ALWAYS_UNLOCKED;
    static const function<bool(const GameState&)> NOT_IMPLEMENTED_YET;
    static const vector<function<bool(const GameState&)>> unlockConditions;
    static const vector<int> achIdList;
};

class Achievements : public ISerializable {
    private:
        vector<uint8_t> bitmasks;
        vector<int64_t> unlockTimes;

    public:
        inline static const vector<int> afterTickAchievements = {
            24, 31, 42, 43, 45, 46
        };

        Achievements();
        Achievements(const json& j);
        
        bool checkUnlockCondition(int id, const GameState& st) const;
        bool isUnlocked(int id) const;
        
        void unlockAchievement(int id, int64_t time);
        void lockAchievement(int id);

        static int idToIndex(int id);
        static int indexToId(int index);
        int achCount() const;

        json to_json() const override;
        void from_json(const json& j) override;
};
