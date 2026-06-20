#include "dimensions.h"
#include "../constants/constants.h"


Dimension::Dimension(Decimal _production, 
    Decimal _cost, 
    Decimal _scaling, 
    Decimal _AA, 
    Decimal _AM, 
    bool _unlocked) 
        :   production(_production), 
            cost(_cost), 
            scaling(_scaling),
            amountAuto(_AA),
            amountManual(_AM),
            unlocked(_unlocked) 
            {};

AntimatterDimensions::AntimatterDimensions() 
    :   dims({
        Dimension(DC::D0,    DC::D10,    DC::D1E3,   DC::D0,     DC::D0,     true),
        Dimension(DC::D0,    DC::D100,   DC::D1E4,   DC::D0,     DC::D0,     false),
        Dimension(DC::D0,    DC::D1E4,   DC::D1E5,   DC::D0,     DC::D0,     false),
        Dimension(DC::D0,    DC::D1E6,   DC::D1E6,   DC::D0,     DC::D0,     false),
        Dimension(DC::D0,    DC::D1E9,   DC::D1E8,   DC::D0,     DC::D0,     false),
        Dimension(DC::D0,    DC::D1E13,  DC::D1E10,  DC::D0,     DC::D0,     false),
        Dimension(DC::D0,    DC::D1E18,  DC::D1E12,  DC::D0,     DC::D0,     false),
        Dimension(DC::D0,    DC::D1E24,  DC::D1E15,  DC::D0,     DC::D0,     false)
        })
    {};

Dimension& AntimatterDimensions::operator[](int index) {
    return dims[index];
}