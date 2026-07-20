#include "gamestate.h"
#include "../constants/constants.h"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


GameState::GameState() : 
    _antimatter(DC::D10),
    _AD(AntimatterDimensions()),
    _tickspeed(Tickspeed()),
    _achievements(Achievements()),
    _sacrificed(DC::D1),
    _dimensionBoosts(0),
    _realTimePlayed(0),
    _canUseKonami(true),
    nextPurchase(DC::D0),
    nextSacrifice(DC::D1),
    currPriceRange(DC::D0),
    achievementBonus(DC::D1),
    sacrificeBonus(DC::D1)
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
    this->purchaseInstructions = {};
    this->sacrificeInstructions = {};
    this->completedInstructions = {};
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

void GameState::recalcSacrificeBonus() {
    this->sacrificeBonus = Decimal::pow(Decimal(_sacrificed.log10()) / DC::D10, this->getSacrificeExponent());
    this->_AD[8].update(*this);
}

Decimal GameState::getSacrificeExponent() {
    return DC::D2;
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
        if (tier == 8 && _AD[8].getPurchases() == 1) {
            initializeSacBranching();
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

Decimal GameState::getSacrificeBonus() {
    return this->sacrificeBonus;
}

void GameState::addInstructions(vector<double> instructions) {
    bool isSacrifice = false;
    for (double instruction : instructions) {
        if (isSacrifice) {
            this->sacrificeInstructions.push_back(instruction);
            isSacrifice = false;
        } else if (10 <= instruction && instruction <= 99) {
            int type = instruction / 10;
            int times = (int) instruction % 10 + 10;
            if (times > 10) times -= 10;
            for (int i = 0; i < times; i++) {
                this->purchaseInstructions.push_back(type);
            }
        } else if (instruction == 108) {
            isSacrifice = true;
        } else {
            this->purchaseInstructions.push_back(instruction);
        }
    }
}

bool GameState::runInstruction(double instruction) {
    if (1 <= instruction && instruction <= 8 && floor(instruction) == instruction) {
        return this->buyOneDimension(instruction);
    } else if (instruction == 9) {
        return this->buyTickspeed();
    } else if (instruction == 130) {
        return this->handleKonamiCode();
    } else {
        return false;
    }
}

bool GameState::runSacInstruction(double instruction) {
    if (nextSacrificeBoost() < Decimal(1, instruction)) return false;
    return sacrificeReset();
}

void GameState::runNextInstructions() {
    while (hasNextPurchaseInstruction() && this->runInstruction((this->purchaseInstructions.front()))) {
        this->completedInstructions.push_back(this->purchaseInstructions.front());
        this->completedPurchases.push_back(this->purchaseInstructions.front());
        this->purchaseInstructions.erase(this->purchaseInstructions.begin());
    }
    while (hasNextSacrificeInstruction() && this->runSacInstruction((this->sacrificeInstructions.front()))) {
        this->completedInstructions.push_back(108);
        this->completedInstructions.push_back(this->sacrificeInstructions.front());
        this->completedSacrifices.push_back(this->sacrificeInstructions.front());
        this->sacrificeInstructions.erase(this->sacrificeInstructions.begin());
    }
}

bool GameState::hasNextPurchaseInstruction() {
    return !this->purchaseInstructions.empty();
}

bool GameState::hasNextSacrificeInstruction() {
    return !this->sacrificeInstructions.empty();
}

vector<double> GameState::getCompletedInstructions() {
    return this->completedInstructions;
}

vector<double> GameState::getCompletedPurchases() {
    return this->completedPurchases;
}

vector<double> GameState::getCompletedSacrifices() {
    return this->completedSacrifices;
}

int GameState::instructionsExecuted() {
    return this->completedInstructions.size();
}

bool GameState::canBranch() {
    return this->_antimatter > this->nextPurchase;
}

void GameState::initializeSacBranching() {
    this->nextSacrifice = this->_AD[1].getAmount();
}

void GameState::setNextSacBranching(double sacValue) {
    this->nextSacrifice = Decimal::pow(
        10, 
        pow(sacValue * Decimal::toNumber(sacrificeBonus), 0.5) * 10
    ) - _sacrificed;
}

bool GameState::canSacBranch() {
    return this->_AD[1].getAmount() >= this->nextSacrifice;
}

Decimal GameState::getPriceRange() {
    return this->currPriceRange;
}

Decimal GameState::getAntimatterGoalForDimboost() {
    Dimension& dimNeeded = this->AD()[min(8, 4 + _dimensionBoosts)];
    int scalingNeeded = ceil((double) (20 + 15 * max(0, _dimensionBoosts - 4)) / 10) - 1;
    return dimNeeded.getInitialCost() * Decimal::pow(dimNeeded.getScaling(), scalingNeeded);
}

bool GameState::canBuyNextDimboost() {
    return this->AD()[min(8, 4 + _dimensionBoosts)].getPurchases() >= 20 + 15 * max(0, _dimensionBoosts - 4);
}

bool GameState::requestDimboost() {
    if (canBuyNextDimboost()) {
        _dimensionBoosts += 1;
        _AD = AntimatterDimensions();
        _tickspeed = Tickspeed();
        _antimatter = DC::D10;
        _tickspeed.update(*this);
        _sacrificed = DC::D1;
        _AD.update(*this);
        return true;
    }
    return false;
}

bool GameState::canSacrifice() {
    if (_dimensionBoosts < 5) return false;
    if (_AD[8].getAmount() == DC::D0) return false;
    if (_AD[1].getAmount() <= _sacrificed) return false;
    return true;
}

bool GameState::sacrificeReset() {
    if (!canSacrifice()) return false;
    _sacrificed += _AD[1].getAmount();
    this->nextSacrifice = _sacrificed;
    this->recalcSacrificeBonus();
    for (int i = 1; i <= 7; i++) {
        _AD[i].resetAmount();
    }
    return true;
}

Decimal GameState::nextSacrificeBoost() {
    Decimal a = (_sacrificed + _AD[1].getAmount()).log10() / DC::D10;
    Decimal b = Decimal::max(_sacrificed.log10() / DC::D10, DC::D1);
    Decimal c = a / b;
    Decimal e = getSacrificeExponent();
    return Decimal::pow(c, e);
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
    j["sacrificed"] = this->_sacrificed.to_json();
    j["dimensionBoosts"] = this->_dimensionBoosts;
    j["realTimePlayed"] = this->_realTimePlayed;
    j["canUseKonami"] = this->_canUseKonami;
    return j;
}

void GameState::from_json(json& j) {
    if (j.contains("antimatter") && !j["antimatter"].is_null()) 
        this->_antimatter = Decimal(j["antimatter"]);
    if (j.contains("antimatterDimensionState") && !j["antimatterDimensionState"].is_null()) 
        this->_AD = AntimatterDimensions(j["antimatterDimensionState"]);
    if (j.contains("tickspeedState") && !j["tickspeedState"].is_null()) 
        this->_tickspeed = Tickspeed(j["tickspeedState"]);
    if (j.contains("achievementState") && !j["achievementState"].is_null()) 
        this->_achievements = Achievements(j["achievementState"]);
    if (j.contains("sacrificed") && !j["sacrificed"].is_null()) 
        this->_sacrificed = Decimal(j["sacrificed"]);
    if (j.contains("dimensionBoosts") && !j["dimensionBoosts"].is_null()) 
        this->_dimensionBoosts = j["dimensionBoosts"];
    if (j.contains("realTimePlayed") && !j["realTimePlayed"].is_null()) 
        this->_realTimePlayed = j["realTimePlayed"];
    if (j.contains("canUseKonami") && !j["canUseKonami"].is_null()) 
        this->_canUseKonami = j["canUseKonami"];
    this->prepare();
}

void GameState::writeInstructions(ofstream& f) {
    f << "[\n";
    int dim_tier = 0;
    int dim_bulk = 0;
    this->completedInstructions.push_back(0);
    vector<string> lines;
    for (double instruction : completedInstructions) {
        if (1 <= instruction && instruction <= 8) {
            if (dim_tier != 0 && dim_tier != instruction && dim_bulk > 0) {
                lines.push_back("[\"buyDimension\",[" + to_string(dim_tier) + "," + to_string(dim_bulk) + "]]");
                dim_bulk = 0;
            }
            dim_tier = instruction;
            dim_bulk++;
        } else {
            if (dim_bulk > 0) {
                lines.push_back("[\"buyDimension\",[" + to_string(dim_tier) + "," + to_string(dim_bulk) + "]]");
                dim_bulk = 0;
            }
            if (instruction == 9) {
                lines.push_back("[\"buyTickspeed\",[0]]");
            }
        }
    }
    if (dim_bulk > 0) {
        lines.push_back("[\"buyDimension\",[" + to_string(dim_tier) + "," + to_string(dim_bulk) + "]]");
        dim_bulk = 0;
    }
    for (int i = 0; i < lines.size(); i++) {
        f << lines[i];
        if (i != lines.size() - 1) f << ",\n";
    }
    f << "\n]\n";
}
