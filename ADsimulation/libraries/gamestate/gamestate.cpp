#include "gamestate.h"
#include "../constants/constants.h"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"


GameState::GameState() : 
    GameState::GameState(
        DC::D10,
        0,
        0,
        0,
        true,
        {}
    ) 
    {};

GameState::GameState(
    Decimal _antimatter,
    int _dimensionBoosts,
    long _tickCounter,
    long _realTimePlayed,
    bool _canUseKonami,
    vector<int> _startingAchievements
)   :   _antimatter(_antimatter),
        _AD(AntimatterDimensions()),
        _tickspeed(Tickspeed()),
        _achievements(Achievements()),
        _dimensionBoosts(_dimensionBoosts),
        _tickCounter(_tickCounter),
        _realTimePlayed(_realTimePlayed),
        _canUseKonami(_canUseKonami)
{
    for (Achievement& ach : this->achievements().achievements()) {
        if (ach.checkUnlockCondition(*this)) {
            ach.unlock();
        }
    }
    for (int ach : _startingAchievements) {
        this->achievements()[ach].unlock();
    }
    completedInstructions = {};
};


ostream& operator<<(ostream& os, GameState& st) {
    os << "You have " << st.antimatter() << " antimatter." << endl;
    os << "You have been playing for " << st._realTimePlayed << " milliseconds." << endl;
    os << "Tickspeed bonus: x" << st.tickspeed().perSecond() << " (x" << st.tickspeed().getDisplayMult(st).toString(3) << " per upgrade)" << endl;
    for (const Dimension& d : st.AD().getDims()) {
        os << d << endl;
    };
    return os;
}

Decimal GameState::antimatter() {
    return this->_antimatter;
}

AntimatterDimensions& GameState::AD() {
    return this->_AD;
}

Tickspeed& GameState::tickspeed() {
    return this->_tickspeed;
}

Achievements& GameState::achievements() {
    return this->_achievements;
}

bool GameState::canUseKonami() {
    return this->_canUseKonami;
}

long GameState::realTimePlayed() {
    return this->_realTimePlayed;
}

void GameState::tick(double diff) {
    _tickCounter++;
    _realTimePlayed += diff * 1000;
    this->tickspeed().update(*this);
    for (int i = 8; i >= 1; i--) {
        this->AD()[i].update(*this);
        if (i == 1) {
            _AD[i].produceCurrency(_antimatter, diff);
        } else if (_AD[i].isUnlocked()) {
            _AD[i].produceDimensions(_AD[i - 1], diff / 10);
        };
    }
    for (Achievement& ach : this->achievements().achievements()) {
        if (ach.checkUnlockCondition(*this)) {
            ach.unlock();
        }
    }
}

bool GameState::buyOneDimension(int dim) {
    if (_AD[dim].canPurchase(_antimatter)) {
        _antimatter -= _AD[dim].getCost();
        _achievements[10 + dim].unlock();
        _AD[dim].onPurchase();
        if (dim == 2) {
            _tickspeed.unlock();
        }
        if (dim < 8 && _dimensionBoosts >= dim - 3) {
            _AD[dim + 1].unlock();
        }
        return true;
    }
    return false;
}

bool GameState::buyDimUntil10(int dim) {
    do {
        buyOneDimension(dim);
    } while (_AD[dim].getPurchases() % 10 == 0 && _AD[dim].canPurchase(_antimatter));
    return _AD[dim].getPurchases() % 10 == 0;
}

bool GameState::buyTickspeed() {
    if (_tickspeed.canPurchase(_antimatter)) {
        _antimatter -= _tickspeed.getCost();
        _tickspeed.onPurchase();
        return true;
    }
    return false;
}

bool GameState::handleKonamiCode() {
    if (canUseKonami()) { 
        this->_antimatter = Decimal(30);
        _canUseKonami = false;
        return true;
    }
    return false;
}

Decimal GameState::getAchievementBonus() {
    Decimal result = DC::D1;
    int rowCounter = 0;
    int cycle = 0;
    for (Achievement& ach : this->achievements().achievements()) {
        if (ach.isUnlocked()) {
            result *= DC::D1_03;
            rowCounter++;
        }
        cycle++;
        if (cycle >= 8) {
            cycle = 0;
            if (rowCounter == 8) {
                result *= DC::D1_25;
            }
            rowCounter = 0;
        }
    }
    return result;
}

void GameState::addInstructions(vector<int> instructions) {
    for (int instruction : instructions) {
        if (10 <= instruction && instruction <= 99) {
            int type = instruction / 10;
            int times = instruction % 10 + 10;
            if (times > 10) times -= 10;
            for (int i = 0; i < times; i++) {
                this->instructions.push_back(type);
            }
        } else {
            this->instructions.push_back(instruction);
        }
    }
}

bool GameState::runInstruction(int instruction) {
    if (1 <= instruction && instruction <= 8) {
        return this->buyOneDimension(instruction);
    } else if (instruction == 9) {
        return this->buyTickspeed();
    } else if (instruction == 130) {
        return this->handleKonamiCode();
    } else {
        return false;
    }
}

void GameState::runNextInstructions() {
    while (hasNextInstruction() && this->runInstruction((this->instructions.front()))) {
        this->completedInstructions.push_back(this->instructions.front());
        this->instructions.pop_front();
    }
}

bool GameState::hasNextInstruction() {
    return !this->instructions.empty();
}

list<int> GameState::getCompletedInstructions() {
    return this->completedInstructions;
}

int GameState::instructionsExecuted() {
    return this->completedInstructions.size();
}

GameState GameState::copy() {
    return *this;
}
