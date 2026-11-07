#include <gtest/gtest.h>
#include "constants.h"
#include "decimal.h"
#include <cstdint>



TEST(DecimalEquivalence, Basic) {
    EXPECT_EQ(Decimal(5), Decimal(5, (int64_t) 0));
}

TEST(DecimalEquivalence, WithExponent) {
    EXPECT_EQ(Decimal(37), Decimal(3.7, (int64_t) 1));
}

TEST(DecimalEquivalence, TransitiveAndDiffExponentEquivalence) {
    Decimal a = Decimal(69420);
    Decimal b = Decimal(6.942, (int64_t) 4);
    Decimal c = Decimal(694.2, (int64_t) 2);
    EXPECT_EQ(a, b);
    EXPECT_EQ(b, c);
    EXPECT_EQ(c, a);
}

TEST(DecimalEquivalence, ZeroEquivalence) {
    EXPECT_EQ(Decimal(0, (int64_t) 0), Decimal(0, (int64_t) 1));
}

TEST(DecimalEquivalence, ConstantEquivalence) {
    EXPECT_EQ(Decimal(10), DC::D10);
}

TEST(DecimalComparison, Basic) {
    EXPECT_GT(Decimal(5), Decimal(4));
    EXPECT_GT(Decimal(500), Decimal(4));
}

TEST(DecimalComparison, BiggerPrecision) {
    EXPECT_GT(Decimal(694201337), Decimal(694201336));
}

TEST(DecimalComparison, TransitiveComparison) {
    Decimal a = Decimal(0.003);
    Decimal b = Decimal(0.1);
    Decimal c = Decimal(1.5);
    EXPECT_LT(a, b);
    EXPECT_LT(a, c);
    EXPECT_LT(b, c);
}

TEST(DecimalComparison, DifferentSigns) {
    EXPECT_LT(Decimal(-1), Decimal(1));
    EXPECT_LT(Decimal(-10), Decimal(1));
    EXPECT_LT(Decimal(-100), Decimal(1));
}

TEST(DecimalComparison, WithZero) {
    EXPECT_GT(Decimal(0.69), Decimal(0));
    EXPECT_LT(Decimal(-0.69), Decimal(0));
}

TEST(DecimalComparison, NegativeComparison) {
    EXPECT_LT(Decimal(-5), Decimal(-4));
    EXPECT_LT(Decimal(-500), Decimal(-4));
}

TEST(DecimalComparison, NonStrictComparison) {
    EXPECT_GE(Decimal(5), Decimal(4));
    EXPECT_GE(Decimal(4), Decimal(4));
    EXPECT_LE(Decimal(4), Decimal(4));
    EXPECT_LE(Decimal(3), Decimal(4));
}

TEST(DecimalMathematics, Basic) {
    Decimal a = Decimal(77);
    Decimal b = Decimal(33);
    EXPECT_EQ(a + b, Decimal(110));
    EXPECT_EQ(a - b, Decimal(44));
    EXPECT_EQ(a * b, Decimal(77 * 33));
    EXPECT_EQ(-a, Decimal(-77));
}

TEST(DecimalMathematics, Multiplication) {
    Decimal a = Decimal(69420);
    Decimal b = Decimal(1337);
    EXPECT_EQ(a * b, Decimal(69420 * 1337));
}

TEST(DecimalMathematics, BigNumberOperations) {
    Decimal a = Decimal(1, (int64_t) 100);
    Decimal b = Decimal(1, (int64_t) 150);
    EXPECT_EQ(a + b, Decimal(1, (int64_t) 150));
    EXPECT_EQ(a * b, Decimal(1, (int64_t) 250));
}

TEST(DecimalMathematics, Inversion) {
    EXPECT_EQ(Decimal::invert(Decimal(69)), Decimal(1.0 / 69));
    EXPECT_EQ(Decimal::invert(DC::D1E5), Decimal(1, (int64_t) -5));
}

TEST(DecimalMathematics, Division) {
    Decimal a = Decimal(1001);
    Decimal b = Decimal(7);
    EXPECT_EQ(a / b, Decimal(1001 / 7));
}

TEST(DecimalMathematics, Power) {
    Decimal nine = Decimal(9);
    EXPECT_EQ(Decimal::pow(nine, nine), Decimal(387420489));
}

TEST(DecimalMathematics, FloatPower) {
    Decimal root10 = Decimal::pow(Decimal(10), Decimal(0.5));
    EXPECT_GT(root10, Decimal(3.1622776));
    EXPECT_LT(root10, Decimal(3.1622777));
}

TEST(DecimalMathematics, BigPower) {
    EXPECT_GT(Decimal::pow(DC::D2, 1024), Decimal(1, (int64_t) 308));
}

TEST(DecimalMathematics, BiggerNumberOperations) {
    Decimal a = Decimal(1, (int64_t) 200);
    Decimal b = Decimal(1, (int64_t) 1000);
    EXPECT_EQ(a + b, b);
}
