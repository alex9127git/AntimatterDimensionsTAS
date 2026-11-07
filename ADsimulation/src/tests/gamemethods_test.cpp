#include <gtest/gtest.h>
#include "gamestate.h"
#include "constants.h"
#include "formulae.h"
#include "decimal.h"


TEST(GameAssumptions, Basic) {
    GameState st = GameState();
    EXPECT_EQ(st.antimatter(), DC::D10);
    EXPECT_EQ(st.AD()[1].getPurchases(), 0);
    EXPECT_EQ(st.tickspeed().getPurchases(), 0);
    EXPECT_EQ(st.getAchievementBonus(), Decimal::pow(DC::D1_03, 4));
}

TEST(GameAssumptions, Purchase) {
    GameState st = GameState();
    EXPECT_TRUE(st.buyOneDimension(1));
    EXPECT_EQ(st.antimatter(), 0);
    EXPECT_EQ(st.AD()[1].getPurchases(), 1);
    EXPECT_FALSE(st.buyOneDimension(1));
    EXPECT_EQ(st.antimatter(), 0);
    EXPECT_EQ(st.AD()[1].getPurchases(), 1);
}

TEST(GameAssumptions, DimensionBoostFormulaTier) {
    EXPECT_EQ(getTierForDimboost(1), 4);
    EXPECT_EQ(getTierForDimboost(3), 6);
    EXPECT_EQ(getTierForDimboost(5), 8);
    EXPECT_EQ(getTierForDimboost(10), 8);
}

TEST(GameAssumptions, DimensionBoostFormulaAmount) {
    EXPECT_EQ(getAmountForDimboost(1), 20);
    EXPECT_EQ(getAmountForDimboost(3), 20);
    EXPECT_EQ(getAmountForDimboost(5), 20);
    EXPECT_EQ(getAmountForDimboost(10), 95);
    EXPECT_EQ(getAmountForDimboost(15), 170);
    EXPECT_EQ(getAmountForDimboost(20), 245);
}

TEST(GameAssumptions, GalaxyFormulaAmount) {
    EXPECT_EQ(getAmountForGalaxy(1), 80);
    EXPECT_EQ(getAmountForGalaxy(2), 140);
    EXPECT_EQ(getAmountForGalaxy(3), 200);
    EXPECT_EQ(getAmountForGalaxy(10), 620);
}
