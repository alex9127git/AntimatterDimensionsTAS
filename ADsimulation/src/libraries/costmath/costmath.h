#pragma once
#include "decimal.h"


class ExponentialCostScaling {
    private:
        double logBaseCost;
        double logBaseIncrease;
        double logCostScale;
        int32_t purchasesBeforeScaling;

    public:
        ExponentialCostScaling(
            Decimal _baseCost,
            Decimal _baseIncrease, 
            Decimal _costScale,
            int32_t _purchasesBeforeScaling);
        ExponentialCostScaling(
            Decimal _baseCost,
            Decimal _baseIncrease, 
            Decimal _costScale,
            Decimal _scalingCostThreshold);
        Decimal baseCost() const {return Decimal(1, logBaseCost);}
        Decimal baseIncrease() const {return Decimal(1, logBaseIncrease);}
        Decimal baseCostScale() const {return Decimal(1, logCostScale);}
        Decimal getCost(int32_t purchases) const;
};