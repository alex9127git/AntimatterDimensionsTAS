#include <vector>
#include "../decimal/decimal.h"
using namespace std;


struct Dimension {
    Decimal scaling;
    Decimal production;
    Decimal cost;
    Decimal amountAuto;
    Decimal amountManual;
    bool unlocked;

    Dimension(
        Decimal _production, 
        Decimal _cost, 
        Decimal _scaling, 
        Decimal _AA, 
        Decimal _AM,
        bool _unlocked);
};

struct AntimatterDimensions {
    vector<Dimension> dims;

    AntimatterDimensions();

    Dimension& operator[](int index);
};