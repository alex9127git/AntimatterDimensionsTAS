#include <functional>
#include "achievements.h"
#include "../gamestate/gamestate.h"
#include "../constants/constants.h"


Achievement::Achievement(
    int _id,
    Decimal _effect,
    Decimal _defaultEffect,
    function<bool(GameState&)> _unlockCondition
)   :   id(_id),
        effect([_effect](Decimal _) {return _effect;}),
        defaultEffect(_defaultEffect),
        unlockCondition(_unlockCondition),
        unlocked(false)
    {};

Achievement::Achievement(
    int _id,
    function<Decimal(Decimal)> _effect,
    Decimal _defaultEffect,
    function<bool(GameState&)> _unlockCondition
)   :   id(_id),
        effect(_effect),
        defaultEffect(_defaultEffect),
        unlockCondition(_unlockCondition),
        unlocked(false)
    {};

void Achievement::unlock() {
    this->unlocked = true;
};

void Achievement::lock() {
    this->unlocked = false;
};

int Achievement::getId() {
    return this->id;
}

bool Achievement::isUnlocked() {
    return this->unlocked;
}

bool Achievement::checkUnlockCondition(GameState& st) {
    return this->unlockCondition(st);
};

void Achievement::print() {
    cout << this->id << " " << this->unlocked << " " << this->isUnlocked() << endl;
}

Achievements::Achievements() {
    _achievements = {
        Achievement(11, DC::D1, DC::D1,
            [](GameState& st) {return st.AD()[1].isUnlocked();}),
        Achievement(12, DC::D1, DC::D1,
            [](GameState& st) {return st.AD()[2].isUnlocked();}),
        Achievement(13, DC::D1, DC::D1,
            [](GameState& st) {return st.AD()[3].isUnlocked();}),
        Achievement(14, DC::D1, DC::D1,
            [](GameState& st) {return st.AD()[4].isUnlocked();}),
        Achievement(15, DC::D1, DC::D1,
            [](GameState& st) {return st.AD()[5].isUnlocked();}),
        Achievement(16, DC::D1, DC::D1,
            [](GameState& st) {return st.AD()[6].isUnlocked();}),
        Achievement(17, DC::D1, DC::D1,
            [](GameState& st) {return st.AD()[7].isUnlocked();}),
        Achievement(18, DC::D1, DC::D1,
            [](GameState& st) {return st.AD()[8].isUnlocked();}),

        Achievement(21, DC::D100, DC::D0,
            [](GameState& st) {return false;}),
        Achievement(22, DC::D1, DC::D1,
            [](GameState& st) {return true;}),
        Achievement(23, DC::D1_1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(24, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(25, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(26, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(27, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(28, DC::D1_1, DC::D1,
            [](GameState& st) {return false;}),

        Achievement(31, DC::D1_05, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(32, DC::D1_1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(33, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(34, DC::D1_02, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(35, DC::D1, DC::D1,
            [](GameState& st) {return true;}),
        Achievement(36, DC::D1_02, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(37, DC::D5E3, DC::D0,
            [](GameState& st) {return false;}),
        Achievement(38, DC::D1, DC::D1,
            [](GameState& st) {return false;}),

        Achievement(41, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(42, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(43, 
            [](Decimal tier) {return DC::D1_01 * tier;}, 
            DC::D1,
            [](GameState& st) {return false;}),
        Achievement(44, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(45, DC::D1_02, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(46, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(47, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(48, DC::D1_1, DC::D1,
            [](GameState& st) {return false;}),

        Achievement(51, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(52, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(53, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(54, DC::D5E5, DC::D0,
            [](GameState& st) {return false;}),
        Achievement(55, DC::D5E10, DC::D0,
            [](GameState& st) {return false;}),
        Achievement(56, 
            [](Decimal minutes) {return Decimal::max(DC::D6 / (minutes + DC::D3), DC::D1);}, 
            DC::D1,
            [](GameState& st) {return false;}),
        Achievement(57, DC::D1_1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(58, DC::D1_01, DC::D1,
            [](GameState& st) {return false;}),

        Achievement(61, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(62, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(63, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(64, DC::D1_25, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(65, 
            [](Decimal minutes) {return Decimal::max(DC::D4 / (minutes + DC::D1), DC::D1);}, 
            DC::D1,
            [](GameState& st) {return false;}),
        Achievement(66, DC::D1_02, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(67, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(68, DC::D1_5, DC::D1,
            [](GameState& st) {return false;}),

        Achievement(71, DC::D3, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(72, DC::D1_1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(73, 
            [](Decimal antimatter) {return Decimal::pow(antimatter, 0.00002) + DC::D1;},
            DC::D1,
            [](GameState& st) {return false;}),
        Achievement(74, DC::D1_4, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(75, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(76, 
            [](Decimal days) {return Decimal::max(Decimal::pow(days / DC::D2, 0.05), DC::D1);}, 
            DC::D1,
            [](GameState& st) {return true;}),
        Achievement(77, DC::D1, DC::D1,
            [](GameState& st) {return false;}),
        Achievement(78, DC::D5E25, DC::D0,
            [](GameState& st) {return false;}),
    };
};

vector<Achievement>& Achievements::achievements() {
    return this->_achievements;
}

Achievement& Achievements::operator[] (int index) {
    return this->_achievements[(index / 10 - 1) * 8 + (index % 10 - 1)];
}