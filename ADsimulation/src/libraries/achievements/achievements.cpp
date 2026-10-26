#include "achievements.h"
#include "../gamestate/gamestate.h"
#include "../constants/constants.h"

const function<Decimal(const Decimal&)> AchConstants::NO_EFFECT = 
        [](const Decimal& _) {return DC::D1;};

const vector<function<Decimal(const Decimal&)>> AchConstants::effectFormulae = {
    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,

    [](const Decimal& _) {return DC::D100;},
    NO_EFFECT,
    [](const Decimal& _) {return DC::D1_1;},
    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,
    [](const Decimal& _) {return DC::D1_1;},

    [](const Decimal& _) {return DC::D1_05;},
    [](const Decimal& _) {return DC::D1_1;},
    NO_EFFECT,
    [](const Decimal& _) {return DC::D1_02;},
    NO_EFFECT,
    [](const Decimal& _) {return DC::D1_02;},
    [](const Decimal& _) {return DC::D5E3;},
    NO_EFFECT,

    NO_EFFECT,
    NO_EFFECT,
    [](const Decimal& tier) {return DC::D1_01 * tier;},
    NO_EFFECT,
    [](const Decimal& _) {return DC::D1_02;},
    NO_EFFECT,
    NO_EFFECT,
    [](const Decimal& _) {return DC::D1_1;},

    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,
    [](const Decimal& _) {return DC::D5E5;},
    [](const Decimal& _) {return DC::D5E10;},
    [](Decimal minutes) {return Decimal::max(DC::D6 / (minutes + DC::D3), DC::D1);},
    [](const Decimal& _) {return DC::D1_1;},
    [](const Decimal& _) {return DC::D1_01;},

    NO_EFFECT,
    NO_EFFECT,
    NO_EFFECT,
    [](const Decimal& _) {return DC::D1_25;},
    [](Decimal minutes) {return Decimal::max(DC::D4 / (minutes + DC::D1), DC::D1);},
    [](const Decimal& _) {return DC::D1_02;},
    NO_EFFECT,
    [](const Decimal& _) {return DC::D1_5;},

    [](const Decimal& _) {return DC::D3;},
    [](const Decimal& _) {return DC::D1_1;},
    [](Decimal antimatter) {return Decimal::pow(antimatter, 0.00002) + DC::D1;},
    [](const Decimal& _) {return DC::D1_4;},
    NO_EFFECT,
    [](Decimal days) {return Decimal::max(Decimal::pow(days / DC::D2, 0.05), DC::D1);},
    NO_EFFECT,
    [](const Decimal& _) {return DC::D5E25;},
};

const function<bool(const GameState&)> AchConstants::NOT_APPLICABLE = 
        [](const GameState& st) {return false;};
const function<bool(const GameState&)> AchConstants::ALWAYS_UNLOCKED = 
        [](const GameState& st) {return true;};
const function<bool(const GameState&)> AchConstants::NOT_IMPLEMENTED_YET = 
        [](const GameState& st) {return false;};

const vector<function<bool(const GameState&)>> AchConstants::unlockConditions = {
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,

    NOT_APPLICABLE,
    ALWAYS_UNLOCKED,
    NOT_APPLICABLE,
    [](const GameState& st) {return st.antimatter() >= DC::D1E80;},
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,

    [](const GameState& st) {
        for (int i = 1; i <= 8; i++) {
            if (st.AD()[i].getMult() >= DC::D1E31) {
                return true;
            }
        }
        return false;
    },
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    ALWAYS_UNLOCKED,
    NOT_APPLICABLE,
    NOT_APPLICABLE,
    NOT_APPLICABLE,

    NOT_APPLICABLE,
    [](const GameState& st) {
        return st.antimatter() >= DC::D1E63 && 
            st.AD()[1].productionPerSecond() > st.antimatter();
    },
    [](const GameState& st) {
        for (int i = 1; i <= 7; i++) {
            if (st.AD()[i].getMult() >= st.AD()[i+1].getMult()) return false;
        }
        return true;
    },
    ALWAYS_UNLOCKED,
    [](const GameState& st) {return st.tickspeed().perSecond() >= DC::D1E26;},
    [](const GameState& st) {return st.AD()[7].getAmount() >= DC::D1E12;},
    NOT_APPLICABLE,
    NOT_APPLICABLE,


    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,

    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET,
    ALWAYS_UNLOCKED,
    NOT_IMPLEMENTED_YET,
    NOT_IMPLEMENTED_YET
};

const vector<int> AchConstants::achIdList = {
    11, 12, 13, 14, 15, 16, 17, 18,
    21, 22, 23, 24, 25, 26, 27, 28,
    31, 32, 33, 34, 35, 36, 37, 38,
    41, 42, 43, 44, 45, 46, 47, 48,
    51, 52, 53, 54, 55, 56, 57, 58,
    61, 62, 63, 64, 65, 66, 67, 68,
    71, 72, 73, 74, 75, 76, 77, 78
};


Achievements::Achievements() {
    bitmasks = {0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < bitmasks.size() * 8; i++) {
        unlockTimes.push_back(-1LL);
    }
}

Achievements::Achievements(const json& j) : Achievements::Achievements() {
    this->from_json(j);
}

void Achievements::unlockAchievement(int id, int64_t time) {
    bitmasks[id / 10 - 1] |= (1 << (id % 10 - 1));
    unlockTimes[idToIndex(id)] = time;
}

void Achievements::lockAchievement(int id) {
    bitmasks[id / 10 - 1] &= (255 - (1 << (id % 10 - 1)));
    unlockTimes[idToIndex(id)] = -1;
}

bool Achievements::checkUnlockCondition(int id, const GameState& st) const {
    return AchConstants::unlockConditions[Achievements::idToIndex(id)](st);
}

bool Achievements::isUnlocked(int id) const {
    return (bitmasks[id / 10 - 1] & (1 << (id % 10 - 1))) != 0;
}

int Achievements::idToIndex(int id) {
    return (id / 10 - 1) * 8 + (id % 10 - 1);
}

int Achievements::indexToId(int index) {
    return (index / 8 + 1) * 10 + (index % 8 + 1);
}

int Achievements::achCount() const {
    return unlockTimes.size();
}

json Achievements::to_json() const {
    json j;
    vector<int> ids;
    for (int i = 0; i < unlockTimes.size(); i++) {
        int id = indexToId(i);
        if (isUnlocked(id)) ids.push_back(id);
    }
    j["unlockedAchievements"] = ids;
    return j;
}

void Achievements::from_json(const json& j) {
    vector<int> ids = j.at("unlockedAchievements");
    for (int id : ids) {
        unlockAchievement(id, 0);
    }
}
