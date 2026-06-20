#include "tickspeed.h"
#include "../constants/constants.h"


Tickspeed::Tickspeed() 
    :   cost(DC::D1E3), 
        effect(DC::D1),
        costScaling(DC::D10),
        amount(DC::D0) 
    {};
