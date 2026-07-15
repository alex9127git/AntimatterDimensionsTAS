#include "gamestate.h"
#include "../constants/constants.h"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;


GameState::GameState() : 
    _antimatter(DC::D10),
    _AD(AntimatterDimensions()),
    _tickspeed(Tickspeed()),
    _achievements(Achievements()),
    _dimensionBoosts(0),
    _realTimePlayed(0),
    _canUseKonami(true),
    nextPurchase(DC::D0),
    currPriceRange(DC::D0),
    achievementBonus(DC::D1)
{
    this->prepare();
};

GameState::GameState(json& j) : GameState()
{
    this->from_json(j);
};

void GameState::prepare() {
    for (Achievement& ach : this->achievements().achievements()) {
        this->unlockAchievement(ach.getId(), false);
    }
    completedInstructions = {};
    prices.push_back(_tickspeed.getCost());
    for (int i = 1; i <= 8; i++) {
        prices.push_back(_AD[i].getCost());
    }
    calcNextPurchase();
    recalcAchievementBonus();
    this->tickspeed().update(*this);
    this->AD().update(*this);
}

void GameState::calcNextPurchase() {
    Decimal minStackPrice = prices[0];
    this->currPriceRange = prices[0];
    this->nextPurchase = prices[0];
    for (int i = 1; i <= min(_dimensionBoosts + 4, 8); i++) {
        Decimal price = _AD[i].getPurchases() < 10 || i == min(_dimensionBoosts + 4, 8) 
                ? prices[i] : prices[i] * DC::D10;
        if (price < minStackPrice) {
            minStackPrice = price;
            currPriceRange = price;
            this->nextPurchase = prices[i];
        } else if (price == minStackPrice) {
            if (prices[i] < this->nextPurchase) {
                this->nextPurchase = prices[i];
            }
        }
    }
}

void GameState::recalcAchievementBonus() {
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
    this->achievementBonus = result;
    this->AD().update(*this);
}

ostream& operator<<(ostream& os, GameState& st) {
    os << "You have " << st.antimatter() << " antimatter." << endl;
    os << "You have been playing for " << st._realTimePlayed << " milliseconds." << endl;
    os << "Tickspeed bonus: x" << st.tickspeed().perSecond() << " (x" << st.tickspeed().getDisplayMult(st).toString(3) << " per upgrade)" << endl;
    for (const AntimatterDimension& d : st.AD().getDims()) {
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

int GameState::dimensionBoosts() {
    return this->_dimensionBoosts;
}

void GameState::tick(double diff) {
    _realTimePlayed += diff * 1000;
    for (int i = 8; i >= 1; i--) {
        if (i == 1) {
            _AD[i].produceCurrency(_antimatter, diff);
        } else if (_AD[i].isUnlocked()) {
            _AD[i].produceDimensions(_AD[i - 1], diff / 10);
        };
    }
    for (int ach : Achievements::afterTickAchievements) {
        this->unlockAchievement(ach, false);
    }
}

bool GameState::buyOneDimension(int tier) {
    if (_AD[tier].canPurchase(_antimatter)) {
        _antimatter -= _AD[tier].getCost();
        _AD[tier].onPurchase();
        if (_AD[tier].getPurchases() % 10 == 0) {
            _AD[tier].update(*this);
        }
        prices[tier] = _AD[tier].getCost();
        unlockAchievement(10 + tier, true);
        calcNextPurchase();
        if (tier == 2) {
            _tickspeed.unlock();
        }
        if (tier < 8 && _dimensionBoosts >= tier - 3) {
            _AD[tier + 1].unlock();
        }
        return true;
    }
    return false;
}

bool GameState::buyDimUntil10(int tier) {
    do {
        buyOneDimension(tier);
    } while (_AD[tier].getPurchases() % 10 == 0 && _AD[tier].canPurchase(_antimatter));
    return _AD[tier].getPurchases() % 10 == 0;
}

bool GameState::buyTickspeed() {
    if (_tickspeed.canPurchase(_antimatter)) {
        _antimatter -= _tickspeed.getCost();
        _tickspeed.onPurchase();
        _tickspeed.update(*this);
        _AD.update(*this);
        prices[0] = _tickspeed.getCost();
        calcNextPurchase();
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

void GameState::unlockAchievement(int ach, bool force) {
    if (force || this->achievements()[ach].checkUnlockCondition(*this)) {
        if (!this->achievements()[ach].isUnlocked()) {
            this->achievements()[ach].unlock();
            this->recalcAchievementBonus();
        }
    }
}

Decimal GameState::getAchievementBonus() {
    return this->achievementBonus;
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
        this->instructions.erase(this->instructions.begin());
    }
}

bool GameState::hasNextInstruction() {
    return !this->instructions.empty();
}

vector<int> GameState::getCompletedInstructions() {
    return this->completedInstructions;
}

int GameState::instructionsExecuted() {
    return this->completedInstructions.size();
}

bool GameState::canBranch() {
    return this->_antimatter > this->nextPurchase;
}

Decimal GameState::getPriceRange() {
    return this->currPriceRange;
}

bool GameState::requestDimboost() {
    if (canBuyNextDimboost()) {
        _dimensionBoosts += 1;
        _AD = AntimatterDimensions();
        _tickspeed = Tickspeed();
        _antimatter = DC::D10;
        _tickspeed.update(*this);
        _AD.update(*this);
        return true;
    }
    return false;
}

bool GameState::canBuyNextDimboost() {
    return this->AD()[min(8, 4 + _dimensionBoosts)].getPurchases() >= 20 + 15 * max(0, _dimensionBoosts - 4);
}

GameState GameState::copy() {
    return *this;
}

json GameState::to_json() {
    json j;
    j["antimatter"] = this->_antimatter.to_json();
    j["antimatterDimensionState"] = this->_AD.to_json();
    j["tickspeedState"] = this->_tickspeed.to_json();
    j["achievementState"] = this->_achievements.to_json();
    j["dimensionBoosts"] = this->_dimensionBoosts;
    j["realTimePlayed"] = this->_realTimePlayed;
    j["canUseKonami"] = this->_canUseKonami;
    return j;
}

void GameState::from_json(json& j) {
    this->_antimatter = Decimal(j["antimatter"]);
    this->_AD = AntimatterDimensions(j["antimatterDimensionState"]);
    this->_tickspeed = Tickspeed(j["tickspeedState"]);
    this->_achievements = Achievements(j["achievementState"]);
    this->_dimensionBoosts = j["dimensionBoosts"];
    this->_realTimePlayed = j["realTimePlayed"];
    this->_canUseKonami = j["canUseKonami"];
    this->prepare();
}
