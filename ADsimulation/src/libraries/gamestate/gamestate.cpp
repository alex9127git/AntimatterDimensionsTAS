#include "gamestate.h"
#include "formulae.h"
#include "../ioutils/ioutils.h"
#include "../constants/constants.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <string>
#include <format>
using namespace std;


GameState::GameState() {
    this->prepare();
}

GameState::GameState(const json& j)
{
    this->from_json(j);
}

void GameState::prepare() {
    for (int i = 0; i < achievements().achCount(); i++) {
        int id = Achievements::indexToId(i);
        this->unlockAchievement(id, false);
    }
    calcPurchases();
    recalcAchievementBonus();
    recalcTickspeed();
    recalcAD();
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

void GameState::recalcAD() {
    for (int i = 8; i >= 1; i--) recalcAD(i);
}

void GameState::recalcAD(int tier) {
    _AD[tier].update(getADmult(*this, tier));
}

void GameState::recalcTickspeed() {
    _tickspeed.update(getTotalTickspeed(*this));
}

void GameState::recalcAchievementBonus() {
    achievementBonus = _achievements.calcBonus();
    recalcAD();
}

bool GameState::checkUnlockCondition(int id) const {
    return AchConstants::unlockConditions[Achievements::idToIndex(id)](*this);
}

bool GameState::runInstruction(uint16_t instruction) {
    if (Instruction::isDimension(instruction)) {
        return this->buyOneDimension(instruction);
    } else if (instruction == Instruction::TickspeedSingle) {
        return this->buyTickspeed();
    } else if (instruction == Instruction::DimensionBoost) {
        return this->requestDimboost();
    } else if (instruction == Instruction::KonamiCode) {
        return this->handleKonamiCode();
    } else {
        return false;
    }
}

bool GameState::runSacInstruction() {
    if (this->_AD[1].getAmount() < this->nextSacrificeInstruction) return false;
    return sacrificeReset();
}

ostream& operator<<(ostream& os, const GameState& st) {
    os << "You have " << st.antimatter() << " antimatter." << endl;
    os << "You have been playing for " << renderTime(st._realTimePlayed) << endl;
    os << format("Tickspeed bonus: x{} (x{} per upgrade)",
            st._tickspeed.perSecond().toString(3),
            Decimal::invert(getTickspeedMultPerPurchase(st)).toString(3)) << endl;
    for (const AntimatterDimension& d : st._AD.getDims()) {
        os << d << endl;
    };
    return os;
}

void GameState::tick(double diff) {
    prevAD1 = _AD[1].getAmount();
    _realTimePlayed += diff * 1000;
    for (int i = 8; i >= 2; i--) {
        if (_AD[i].isUnlocked()) {
            _AD[i].produceDimensions(_AD[i - 1], _tickspeed, diff / 10);
        }
    }
    Decimal currencyProduction = _AD[1].productionPerDiff(_tickspeed, diff);
    _antimatter += currencyProduction;
    _totalAntimatter += currencyProduction;
    _antimatterThisReset += currencyProduction;
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
            recalcAD(tier);
        }
        unlockAchievement(10 + tier, true);
        calcPurchases();
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
        if (!buyOneDimension(tier)) return false;
    } while (_AD[tier].getPurchases() % 10 != 0);
    return true;
}

bool GameState::buyTickspeed() {
    if (_tickspeed.canPurchase(_antimatter)) {
        _antimatter -= _tickspeed.getCost();
        _tickspeed.onPurchase();
        recalcTickspeed();
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
    if (force || this->checkUnlockCondition(ach)) {
        this->_achievements.unlockAchievement(ach, this->realTimePlayed());
        this->recalcAchievementBonus();
    }
}

Decimal GameState::getAchievementBonus() const {
    return this->achievementBonus;
}

void GameState::addInstructions(const vector<double>& instructions) {
    instructionQueue.add(instructions);
    if (queue().hasNextSacrifice()) {
        this->nextSacrificeInstruction = _sacrifice.getReqForSac(Decimal(1, queue().peekNextSacrifice()));
    }
}

void GameState::clearAllInstructions() {
    this->instructionQueue.clearAllInstructions();
}

void GameState::runNextInstructions() {
    while (queue().hasNextPurchase() && this->runInstruction(queue().peekNextPurchase())) {
        instructionQueue.completePurchase();
    }
    while (queue().hasNextSacrifice() && this->runSacInstruction()) {
        instructionQueue.completeSacrifice();
        if (queue().hasNextSacrifice()) {
            this->nextSacrificeInstruction = _sacrifice.getReqForSac(Decimal(1, queue().peekNextSacrifice()));
        }
    }
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

void GameState::setNextSacBranching(Decimal sacValue) {
    this->nextSacrificeBranch = _sacrifice.getReqForSac(sacValue);
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
        _AD = AntimatterDimensions();
        _tickspeed = Tickspeed();
        _sacrifice = Sacrifice();
        _antimatter = DC::D10;
        _antimatterThisReset = DC::D10;
        recalcTickspeed();
        recalcAD();
        return true;
    }
    return false;
}

bool GameState::canSacrifice() const {
    if (_dimensionBoosts < 5) return false;
    if (_AD[8].getAmount() == DC::D0) return false;
    if (_AD[1].getAmount() <= max(_sacrifice.getSacrificedAmount(), DC::D1E10)) return false;
    return true;
}

Decimal GameState::nextSacrificeBoost() const {
    return _sacrifice.nextSacrificeBoost(_AD[1].getAmount());
}

bool GameState::sacrificeReset() {
    if (!canSacrifice()) return false;
    _sacrifice.increaseSacrifice(_AD[1].getAmount());
    for (int i = 1; i <= 7; i++) {
        _AD[i].resetAmount();
    }
    recalcAD(8);
    return true;
}

json GameState::to_json() const {
    json j;
    j["antimatter"] = this->_antimatter.to_json();
    j["totalAntimatter"] = this->_totalAntimatter.to_json();
    j["antimatterThisReset"] = this->_antimatterThisReset.to_json();
    j["antimatterDimensionState"] = this->_AD.to_json();
    j["tickspeedState"] = this->_tickspeed.to_json();
    j["achievementState"] = this->_achievements.to_json();
    j["sacrificed"] = this->_sacrifice.to_json();
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
    readIfPresent(j, "sacrificed", this->_sacrifice);
    readIfPresent(j, "dimensionBoosts", this->_dimensionBoosts);
    readIfPresent(j, "realTimePlayed", this->_realTimePlayed);
    readIfPresent(j, "canUseKonami", this->_canUseKonami);
    this->prepare();
}
