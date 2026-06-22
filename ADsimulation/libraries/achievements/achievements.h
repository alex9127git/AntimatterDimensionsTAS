#pragma once
#include <vector>
#include <functional>
#include "../decimal/decimal.h"
using namespace std;


class GameState;


class Achievement {
    private:
        int id;
        function<Decimal(Decimal)> effect;
        Decimal defaultEffect;
        function<bool(GameState&)> unlockCondition;
        bool unlocked;

    public:
        Achievement(
            int _id, 
            Decimal _effect, 
            Decimal _defaultEffect,
            function<bool(GameState&)> _unlockCondition
        );
        Achievement(
            int _id, 
            function<Decimal(Decimal)> _effect, 
            Decimal _defaultEffect,
            function<bool(GameState&)> _unlockCondition
        );

        void unlock();
        void lock();

        int getId();
        bool isUnlocked();
        bool checkUnlockCondition(GameState& st);

        void print();
};

class Achievements {
    private:
        vector<Achievement> _achievements;

    public:
        Achievements();

        vector<Achievement>& achievements();

        Achievement& operator[](int index);
};
