#include "achievements.h"
#include "constants.h"
#include <functional>
using namespace std;


namespace {
    const function<Decimal(const Decimal&)> NO_EFFECT = 
            [](const Decimal& _) {return DC::D1;};

    const vector<function<Decimal(const Decimal&)>> effectFormulae = {
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
}

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

Decimal Achievements::calcBonus() const {
    int fullRows = 0;
    int achievements = 0;
    int rowCounter = 0;
    int cycle = 0;
    for (int i = 0; i < achCount(); i++) {
        int id = indexToId(i);
        if (isUnlocked(id)) {
            achievements++;
            rowCounter++;
        }
        cycle++;
        if (cycle >= 8) {
            cycle = 0;
            if (rowCounter == 8) {
                fullRows++;;
            }
            rowCounter = 0;
        }
    }
    return Decimal::pow(DC::D1_25, fullRows) * Decimal::pow(DC::D1_03, achievements);
}

json Achievements::to_json() const {
    json j;
    vector<int> ids;
    for (int i = 0; i < achCount(); i++) {
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
