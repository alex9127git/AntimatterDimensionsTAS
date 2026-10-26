#include "gamestate.h"
#include "../ioutils/ioutils.h"
#include "../constants/constants.h"
#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"
#include "../achievements/achievements.h"
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;


GameState::GameState() {
    this->prepare();
};

GameState::GameState(const json& j) : GameState()
{
    this->from_json(j);
};

void GameState::prepare() {
    for (int id : AchConstants::achIdList) {
        this->unlockAchievement(id, false);
    }
    this->purchaseInstructions = {};
    this->sacrificeInstructions = {};
    this->completedInstructions = {};
    calcPurchases();
    recalcSacrificeBonus();
    recalcAchievementBonus();
    this->_tickspeed.update(*this);
    this->_AD.update(*this);
}

void GameState::calcPurchases() {
    allowedPurchases.clear();
    int frontier = 1;
    while (_AD[frontier].getPurchases() >= 10 && frontier < min(_dimensionBoosts + 4, 8)) {
        frontier++;
    }
    for (int i = 1; i <= frontier; i++) {
        if (_AD[i].getPurchases() > 10 && _AD[i].getPurchases() % 10 != 0) {
            allowedPurchases.push_back(i);
            return;
        }
    }
    Decimal minPrice = _tickspeed.getCost();
    for (int i = 1; i < frontier; i++) {
        minPrice = min(_AD[i].getCost() * DC::D10, minPrice);
    }
    Decimal frontierPrice = _AD[frontier].getCost() * DC::D10;
    if (frontierPrice > DC::D0_2 * minPrice) {
        if (_tickspeed.getCost() == minPrice) allowedPurchases.push_back(9);
        for (int i = 1; i < frontier; i++) {
            if (_AD[i].getCost() * DC::D10 == minPrice) allowedPurchases.push_back(i);
        }
    }
    if (frontierPrice < DC::D20 * minPrice) {
        allowedPurchases.push_back(frontier);
    }
    stable_sort(allowedPurchases.begin(), allowedPurchases.end(),
        [&](uint16_t a, uint16_t b) {
            Decimal a_cost = a == 9 ? _tickspeed.getCost() : _AD[a].getCost();
            Decimal b_cost = b == 9 ? _tickspeed.getCost() : _AD[b].getCost();
            return a_cost < b_cost;
        }
    );
    int minPurchase = allowedPurchases.front();
    nextPurchaseCost = minPurchase == 9 ? _tickspeed.getCost() : _AD[minPurchase].getCost();
}

void GameState::recalcAchievementBonus() {
    Decimal result = DC::D1;
    int rowCounter = 0;
    int cycle = 0;
    for (int id : AchConstants::achIdList) {
        if (_achievements.isUnlocked(id)) {
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
    this->_AD.update(*this);
}

void GameState::recalcSacrificeBonus() {
    if (_sacrificed == DC::D0) {
        this->sacrificeBonus = DC::D1;
    } else {
        this->sacrificeBonus = max(Decimal::pow(
            Decimal(_sacrificed.log10()) / DC::D10,
            this->getSacrificeExponent()
        ), DC::D1);
    }
    this->_AD[8].update(*this);
}

Decimal GameState::getSacrificeExponent() const {
    return DC::D2;
}

ostream& operator<<(ostream& os, const GameState& st) {
    os << "You have " << st.antimatter() << " antimatter." << endl;
    os << "You have been playing for " << renderTime(st._realTimePlayed) << endl;
    os << "Tickspeed bonus: x" << st._tickspeed.perSecond() << " (x" << st._tickspeed.getDisplayMult(st).toString(3) << " per upgrade)" << endl;
    for (const AntimatterDimension& d : st._AD.getDims()) {
        os << d << endl;
    };
    return os;
}

Decimal GameState::antimatter() const {
    return this->_antimatter;
}

Decimal GameState::totalAntimatter() const {
    return this->_totalAntimatter;
}

Decimal GameState::antimatterThisReset() const {
    return this->_antimatterThisReset;
}

const AntimatterDimensions& GameState::AD() const {
    return this->_AD;
}

Decimal GameState::getPrevAD1() const {
    return prevAD1;
}

const Tickspeed& GameState::tickspeed() const {
    return this->_tickspeed;
}

const Achievements& GameState::achievements() const {
    return this->_achievements;
}

bool GameState::canUseKonami() const {
    return this->_canUseKonami;
}

int64_t GameState::realTimePlayed() const {
    return this->_realTimePlayed;
}

int32_t GameState::dimensionBoosts() const {
    return this->_dimensionBoosts;
}

void GameState::tick(double diff) {
    prevAD1 = _AD[1].getAmount();
    _realTimePlayed += diff * 1000;
    for (int i = 8; i >= 1; i--) {
        if (i == 1) {
            _AD[i].produceCurrency(_antimatter, diff);
            _AD[i].produceCurrency(_totalAntimatter, diff);
            _AD[i].produceCurrency(_antimatterThisReset, diff);
        } else if (_AD[i].isUnlocked()) {
            _AD[i].produceDimensions(_AD[i - 1], diff / 10);
        };
    }
    for (int ach : Achievements::afterTickAchievements) {
        this->unlockAchievement(ach, false);
    }
}

bool GameState::buyOneDimension(int tier) {
    if (tier > 1 && _AD[tier - 1].getAmount() == DC::D0) return false;
    if (_AD[tier].canPurchase(_antimatter)) {
        _antimatter -= _AD[tier].getCost();
        _AD[tier].onPurchase();
        if (_AD[tier].getPurchases() % 10 == 0) {
            _AD[tier].update(*this);
        }
        unlockAchievement(10 + tier, true);
        calcPurchases();
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
        calcPurchases();
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
    if (this->_achievements.isUnlocked(ach)) return;
    if (force || this->_achievements.checkUnlockCondition(ach, *this)) {
        this->_achievements.unlockAchievement(ach, this->realTimePlayed());
        this->recalcAchievementBonus();
    }
}

Decimal GameState::getAchievementBonus() const {
    return this->achievementBonus;
}

Decimal GameState::getSacrificeBonus() const {
    return this->sacrificeBonus;
}

void GameState::clearAllInstructions() {
    this->purchaseInstructions.clear();
    this->sacrificeInstructions.clear();
}

void GameState::addInstructions(const vector<double>& instructions) {
    bool isSacrifice = false;
    for (double instruction : instructions) {
        if (isSacrifice) {
            if (this->sacrificeInstructions.empty()) {
                this->nextSacrificeInstruction = getReqForSac(Decimal(1, instruction));
            }
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
    } else if (instruction == 100) {
        return this->requestDimboost();
    } else if (instruction == 130) {
        return this->handleKonamiCode();
    } else {
        return false;
    }
}

bool GameState::runSacInstruction(double instruction) {
    if (this->sacrificeInstructions.empty()) return false;
    if (this->_AD[1].getAmount() < this->nextSacrificeInstruction) return false;
    return sacrificeReset();
}

void GameState::runNextInstructions() {
    while (hasNextPurchaseInstruction() && this->runInstruction((this->purchaseInstructions.front()))) {
        this->completedInstructions.push_back(this->purchaseInstructions.front());
        this->purchaseInstructions.erase(this->purchaseInstructions.begin());
    }
    while (hasNextSacrificeInstruction() && this->runSacInstruction((this->sacrificeInstructions.front()))) {
        this->completedInstructions.push_back(108);
        this->completedSacrifices.push_back(this->sacrificeInstructions.front());
        this->sacrificeInstructions.erase(this->sacrificeInstructions.begin());
        if (!this->sacrificeInstructions.empty()) {
            this->nextSacrificeInstruction = getReqForSac(Decimal(1, this->sacrificeInstructions.front()));
        }
    }
}

bool GameState::hasNextPurchaseInstruction() const {
    return !this->purchaseInstructions.empty();
}

bool GameState::hasNextSacrificeInstruction() const {
    return !this->sacrificeInstructions.empty();
}

vector<double> GameState::getCompletedPurchases() const {
    vector<double> purchases;
    for (double instruction : this->completedInstructions) {
        if (instruction != 108) {
            purchases.push_back(instruction);
        }
    }
    return purchases;
}

const vector<double>& GameState::getCompletedSacrifices() const {
    return completedSacrifices;
}

int GameState::instructionsExecuted() const {
    return this->completedInstructions.size();
}

Decimal GameState::getAntimatterGoalForDimboost() const {
    const Dimension& dimNeeded = this->_AD[min(8, 4 + _dimensionBoosts)];
    int dimAmountRequied = 20 + 15 * max(0, _dimensionBoosts - 4);
    int scalingNeeded = ceil(dimAmountRequied / 10.0) - 1;
    int highPurchases = dimAmountRequied % 10 + (dimAmountRequied % 10 == 0 ? 10 : 0);
    return dimNeeded.getInitialCost() * Decimal::pow(dimNeeded.getScaling(), scalingNeeded) * Decimal(highPurchases);
}

bool GameState::canBranch() const {
    return this->_antimatter >= this->nextPurchaseCost;
}

const vector<uint16_t>& GameState::getPurchaseBranches() const {
    return this->allowedPurchases;
}

void GameState::initializeSacBranching() {
    this->nextSacrificeBranch = this->_AD[1].getAmount();
}

void GameState::setNextSacBranching(Decimal sacValue) {
    this->nextSacrificeBranch = getReqForSac(sacValue);
}

Decimal GameState::getReqForSac(const Decimal& sacValue) const {
    return Decimal::pow(
        10, 
        Decimal::pow(sacValue, DC::D1 / getSacrificeExponent()) * 10
    ) - _sacrificed;
}

bool GameState::canSacBranch() const {
    return this->_AD[1].getAmount() >= this->nextSacrificeBranch;
}

bool GameState::canBuyNextDimboost() const {
    return this->_AD[min(8, 4 + _dimensionBoosts)].getPurchases() >= 20 + 15 * max(0, _dimensionBoosts - 4);
}

bool GameState::requestDimboost() {
    if (canBuyNextDimboost()) {
        _dimensionBoosts += 1;
        sacrificesDone = 0;
        _AD = AntimatterDimensions();
        _tickspeed = Tickspeed();
        _antimatter = DC::D10;
        _antimatterThisReset = DC::D10;
        _tickspeed.update(*this);
        _sacrificed = DC::D0;
        recalcSacrificeBonus();
        _AD.update(*this);
        return true;
    }
    return false;
}

bool GameState::canSacrifice() const {
    if (_dimensionBoosts < 5) return false;
    if (_AD[8].getAmount() == DC::D0) return false;
    if (_AD[1].getAmount() <= max(_sacrificed, DC::D1E10)) return false;
    return true;
}

Decimal GameState::nextSacrificeBoost() const {
    return nextSacrificeBoost(_AD[1].getAmount());
}

Decimal GameState::nextSacrificeBoost(const Decimal& AD1amount) const {
    Decimal a = (_sacrificed + AD1amount).log10() / DC::D10;
    Decimal b = Decimal::max(Decimal::max(_sacrificed, DC::D1).log10() / DC::D10, DC::D1);
    Decimal c = a / b;
    Decimal e = getSacrificeExponent();
    return Decimal::pow(c, e);
}

bool GameState::sacrificeReset() {
    if (!canSacrifice()) return false;
    _sacrificed += _AD[1].getAmount();
    sacrificesDone++;
    this->recalcSacrificeBonus();
    for (int i = 1; i <= 7; i++) {
        _AD[i].resetAmount();
    }
    return true;
}

int GameState::getSacrificesDone() const {
    return sacrificesDone;
}

GameState GameState::copy() const {
    return *this;
}

json GameState::to_json() const {
    json j;
    j["antimatter"] = this->_antimatter.to_json();
    j["totalAntimatter"] = this->_totalAntimatter.to_json();
    j["antimatterThisReset"] = this->_antimatterThisReset.to_json();
    j["antimatterDimensionState"] = this->_AD.to_json();
    j["tickspeedState"] = this->_tickspeed.to_json();
    j["achievementState"] = this->_achievements.to_json();
    j["sacrificed"] = this->_sacrificed.to_json();
    j["dimensionBoosts"] = this->_dimensionBoosts;
    j["realTimePlayed"] = this->_realTimePlayed;
    j["canUseKonami"] = this->_canUseKonami;
    return j;
}

void GameState::from_json(const json& j) {
    readIfPresent(j, "antimatter", this->_antimatter);
    readIfPresent(j, "totalAntimatter", this->_totalAntimatter);
    readIfPresent(j, "antimatterThisReset", this->_antimatterThisReset);
    readIfPresent(j, "antimatterDimensionState", this->_AD);
    readIfPresent(j, "tickspeedState", this->_tickspeed);
    readIfPresent(j, "achievementState", this->_achievements);
    readIfPresent(j, "sacrificed", this->_sacrificed);
    readIfPresent(j, "dimensionBoosts", this->_dimensionBoosts);
    readIfPresent(j, "realTimePlayed", this->_realTimePlayed);
    readIfPresent(j, "canUseKonami", this->_canUseKonami);
    this->prepare();
}

void GameState::writeInstructions(ofstream& f) {
    f << "[\n";
    int dim_tier = 0;
    int dim_bulk = 0;
    bool isNextSacrifice = 0;
    Decimal sacTarget = DC::D1;
    int sacrificeIndex = 0;
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
                lines.push_back("[\"buyTickSpeed\",[0]]");
            }
            if (instruction == 100) {
                lines.push_back("[\"buyDimensionBoost\",[0]]");
            }
            if (instruction == 108) {
                sacTarget = Decimal(1, this->completedSacrifices[sacrificeIndex++]);
                lines.push_back("[\"trySacrificeReset\",[\"" + sacTarget.toString(10) + "\"]]");
            }
        }
    }
    for (int i = 0; i < lines.size(); i++) {
        f << lines[i];
        if (i != lines.size() - 1) f << ",\n";
    }
    f << "\n]\n";
}
