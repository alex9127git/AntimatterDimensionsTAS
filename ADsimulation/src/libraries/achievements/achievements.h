#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include "../decimal/decimal.h"
#include "../interfaces/interfaces.h"
using namespace std;


class GameState;


struct AchConstants {
    static function<Decimal(Decimal&)> NO_EFFECT;
    static vector<function<Decimal(Decimal&)>> effectFormulae;
    static function<bool(GameState&)> NOT_APPLICABLE;
    static function<bool(GameState&)> ALWAYS_UNLOCKED;
    static function<bool(GameState&)> NOT_IMPLEMENTED_YET;
    static vector<function<bool(GameState&)>> unlockConditions;
    static vector<int> achIdList;
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
        Achievements(json& j);
        
        bool checkUnlockCondition(int id, GameState& st) const;
        bool isUnlocked(int id) const;
        
        void unlockAchievement(int id, int64_t time);
        void lockAchievement(int id);

        static int idToIndex(int id);
        static int indexToId(int index);
        int achCount();

        json to_json() override;
        void from_json(json& j) override;
};
