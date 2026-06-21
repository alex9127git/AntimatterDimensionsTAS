#pragma once
#include "../decimal/decimal.h"


class Tickspeed {
    private:
        Decimal cost;
        Decimal effect;
        Decimal costScaling;
        Decimal amount;
    
    public:
        Tickspeed();
};