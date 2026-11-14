#include "gamestate.h"
#include "formulae.h"
#include <algorithm>
using namespace std;


GameState::GameState() {
    this->prepare();
}

GameState::GameState(const json& j)
{
    this->from_json(j);
}

void GameState::prepare() {
    recalcAchievementBonus();
    recalcSacrifice();
    recalcTickspeed();
    recalcAD();
    for (int i = 0; i < achievements().achCount(); i++) {
        int id = Achievements::indexToId(i);
        this->unlockAchievement(id, false);
    }
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

void GameState::recalcSacrifice() {
    _sacrifice.updateExponent(getSacrificeExponent(*this));
}

void GameState::recalcAchievementBonus() {
    achievementBonus = _achievements.calcBonus();
    recalcAD();
}

void GameState::tier1Reset() {
    _AD = AntimatterDimensions();
    _tickspeed = Tickspeed();
    _sacrifice = Sacrifice();
    Decimal startAntimatter = max({
        DC::D10,
        achievements().effectOrDefault(21)
    });
    _antimatter = startAntimatter;
    _antimatterThisReset = startAntimatter;
}

void GameState::tier2Reset() {
    tier1Reset();
    _maxAntimatterThisInfinity = _antimatter;
}

bool GameState::checkUnlockCondition(int id) const {
    return AchConstants::unlockConditions[Achievements::idToIndex(id)](*this);
}

void GameState::tick(double diff) {
    _realTimePlayed += diff * 1000;
    for (int i = 8; i >= 2; i--) {
        if (_AD[i].isUnlocked()) {
            _AD[i].produceDimensions(_AD[i - 1], _tickspeed, getAMcap(), diff / 10);
        }
    }
    Decimal currencyProduction = _AD[1].productionPerDiff(_tickspeed, getAMcap(), diff);
    _antimatter += currencyProduction;
    _totalAntimatter += currencyProduction;
    _antimatterThisReset += currencyProduction;
    _maxAntimatterThisInfinity = max(_maxAntimatterThisInfinity, _antimatter);
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
        unlockAchievement(23, false);
        if (tier == 1) unlockAchievement(28, false);
        if (tier == 2) _tickspeed.unlock();
        if (tier < 8 && _dimensionBoosts >= tier - 3) _AD[tier + 1].unlock();
        return true;
    }
    return false;
}

bool GameState::buyTickspeed() {
    if (_tickspeed.canPurchase(_antimatter)) {
        _antimatter -= _tickspeed.getCost();
        _tickspeed.onPurchase();
        recalcTickspeed();
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
        this->_achievements.unlockAchievement(ach);
        this->recalcAchievementBonus();
    }
}

Decimal GameState::getAchievementBonus() const {
    return this->achievementBonus;
}

Decimal GameState::getAntimatterGoalForDimboost() const {
    const Dimension& dimNeeded = this->_AD[getTierForDimboost(_dimensionBoosts + 1)];
    int32_t dimAmountRequied = getAmountForDimboost(_dimensionBoosts + 1);
    return getCostForDimPurchases(dimNeeded.getCostRule(), dimAmountRequied);
}

Decimal GameState::getAntimatterGoalForGalaxy() const {
    const Dimension& dimNeeded = this->_AD[8];
    int32_t dimAmountRequied = getAmountForGalaxy(_antimatterGalaxies + 1);
    return getCostForDimPurchases(dimNeeded.getCostRule(), dimAmountRequied);
}

bool GameState::canBuyNextDimboost() const {
    if (!infinityBroken() && maxAntimatterThisInfinity() > DC::INF) return false;
    return (this->_AD[getTierForDimboost(_dimensionBoosts + 1)].getPurchases() >= 
            getAmountForDimboost(_dimensionBoosts + 1));
}

bool GameState::canBuyNextGalaxy() const {
    if (!infinityBroken() && maxAntimatterThisInfinity() > DC::INF) return false;
    return (this->_AD[8].getPurchases() >= getAmountForGalaxy(_antimatterGalaxies + 1));
}

bool GameState::canCrunch() const {
    return this->maxAntimatterThisInfinity() >= DC::INF;
}

bool GameState::dimboostReset() {
    if (canBuyNextDimboost()) {
        _dimensionBoosts += 1;
        tier1Reset();
        unlockAchievement(25, false);
        recalcSacrifice();
        recalcTickspeed();
        recalcAD();
        return true;
    }
    return false;
}

bool GameState::galaxyReset() {
    if (canBuyNextGalaxy()) {
        unlockAchievement(26, true);
        _dimensionBoosts = 0;
        _antimatterGalaxies += 1;
        tier1Reset();
        unlockAchievement(27, false);
        recalcSacrifice();
        recalcTickspeed();
        recalcAD();
        return true;
    }
    return false;
}

bool GameState::infinityReset() {
    if (canCrunch()) {
        unlockAchievement(21, true);
        _dimensionBoosts = 0;
        _antimatterGalaxies = 0;
        _infinity.addCurrency(DC::D1, DC::D1);
        tier2Reset();
        recalcSacrifice();
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

Decimal GameState::sacrificeIncrease() const {
    return _sacrifice.sacrificeIncrease(_AD[1].getAmount());
}

Decimal GameState::getBoostAfterSacrifice() const {
    return _sacrifice.getSacrificeBonus() * sacrificeIncrease();
}

bool GameState::sacrificeReset() {
    if (!canSacrifice()) return false;
    _sacrifice.increaseSacrifice(_AD[1].getAmount());
    unlockAchievement(32, false);
    for (int i = 1; i <= 7; i++) {
        _AD[i].resetAmount();
    }
    recalcSacrifice();
    recalcAD(8);
    return true;
}

json GameState::to_json() const {
    json j;
    j["antimatter"] = this->_antimatter.to_json();
    j["totalAntimatter"] = this->_totalAntimatter.to_json();
    j["antimatterThisReset"] = this->_antimatterThisReset.to_json();
    j["maxAntimatterThisInfinity"] = this->_maxAntimatterThisInfinity.to_json();
    j["antimatterDimensionState"] = this->_AD.to_json();
    j["tickspeedState"] = this->_tickspeed.to_json();
    j["achievementState"] = this->_achievements.to_json();
    j["sacrificed"] = this->_sacrifice.to_json();
    j["infinityState"] = this->_infinity.to_json();
    j["dimensionBoosts"] = this->_dimensionBoosts;
    j["antimatterGalaxies"] = this->_antimatterGalaxies;
    j["realTimePlayed"] = this->_realTimePlayed;
    j["canUseKonami"] = this->_canUseKonami;
    return j;
}

void GameState::from_json(const json& j) {
    readIfPresent(j, "antimatter", this->_antimatter);
    readIfPresent(j, "totalAntimatter", this->_totalAntimatter);
    readIfPresent(j, "antimatterThisReset", this->_antimatterThisReset);
    readIfPresent(j, "maxAntimatterThisInfinity", this->_maxAntimatterThisInfinity);
    readIfPresent(j, "antimatterDimensionState", this->_AD);
    readIfPresent(j, "tickspeedState", this->_tickspeed);
    readIfPresent(j, "achievementState", this->_achievements);
    readIfPresent(j, "sacrificed", this->_sacrifice);
    readIfPresent(j, "infinityState", this->_infinity);
    readIfPresent(j, "dimensionBoosts", this->_dimensionBoosts);
    readIfPresent(j, "antimatterGalaxies", this->_antimatterGalaxies);
    readIfPresent(j, "realTimePlayed", this->_realTimePlayed);
    readIfPresent(j, "canUseKonami", this->_canUseKonami);
    this->prepare();
}
