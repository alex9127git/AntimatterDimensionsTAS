#include "costmath.h"
#include "constants.h"


ExponentialCostScaling::ExponentialCostScaling(
    Decimal _baseCost,
    Decimal _baseIncrease, 
    Decimal _costScale,
    int32_t _purchasesBeforeScaling
)   :   logBaseCost(_baseCost.log10value()),
        logBaseIncrease(_baseIncrease.log10value()),
        logCostScale(_costScale.log10value()),
        purchasesBeforeScaling(_purchasesBeforeScaling)
    {}

ExponentialCostScaling::ExponentialCostScaling(
    Decimal _baseCost,
    Decimal _baseIncrease, 
    Decimal _costScale,
    Decimal _scalingCostThreshold
)   :   logBaseCost(_baseCost.log10value()),
        logBaseIncrease(_baseIncrease.log10value()),
        logCostScale(_costScale.log10value()),
        purchasesBeforeScaling(
            ceil((_scalingCostThreshold.log10value() - _baseCost.log10value()) / _baseIncrease.log10value())
        )
    {}

Decimal ExponentialCostScaling::getCost(int32_t purchases) const {
    int32_t excess = purchases - purchasesBeforeScaling;
    double logPrice = logBaseCost + logBaseIncrease * purchases;
    if (excess > 0) logPrice += 0.5 * excess * (excess + 1) * logCostScale;
    return Decimal::pow(DC::D10, logPrice);
}