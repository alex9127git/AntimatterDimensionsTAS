#include "../decimal/decimal.h"


struct Tickspeed {
    Decimal cost;
    Decimal effect;
    Decimal costScaling;
    Decimal amount;
    Tickspeed();
};