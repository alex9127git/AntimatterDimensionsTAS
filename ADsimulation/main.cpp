#include <iostream>
#include <vector>
#include "libraries/constants/constants.h"


/*  NOTE:
*   When using numbers that interact with any Decimal value
*   intialize it as a Decimal type.
*/
using namespace std;


struct Dimension {
    Decimal scaling;
    Decimal production;
    Decimal cost;
    Decimal amountAuto;
    Decimal amountManual;
    bool unlocked;

    Dimension(Decimal _production, Decimal _cost, Decimal _scaling, Decimal _AA, Decimal _AM, bool _unlocked) 
        :   production(_production), 
            cost(_cost), 
            scaling(_scaling),
            amountAuto(_AA),
            amountManual(_AM),
            unlocked(_unlocked) 
            {};
};


struct Dimensions {
    vector<Dimension> dims = {
        {DC::D0,    DC::D10,    DC::D1E3,   DC::D0,     DC::D0,     true},
        {DC::D0,    DC::D100,   DC::D1E4,   DC::D0,     DC::D0,     false},
        {DC::D0,    DC::D1E4,   DC::D1E5,   DC::D0,     DC::D0,     false},
        {DC::D0,    DC::D1E6,   DC::D1E6,   DC::D0,     DC::D0,     false},
        {DC::D0,    DC::D1E9,   DC::D1E8,   DC::D0,     DC::D0,     false},
        {DC::D0,    DC::D1E13,  DC::D1E10,  DC::D0,     DC::D0,     false},
        {DC::D0,    DC::D1E18,  DC::D1E12,  DC::D0,     DC::D0,     false},
        {DC::D0,    DC::D1E24,  DC::D1E15,  DC::D0,     DC::D0,     false}
    };
};

struct Player {
    Decimal antimatter;

    Player(Decimal am) : antimatter(am) {};
    
    bool buyDimension(Dimension& a, Decimal _amount) {
        if (Decimal::gte(antimatter, a.cost)) {
            Decimal::add(a.amountManual, {1,0});
            a.cost = a.cost * a.scaling;
            return true;
        };
        return false;
    };
};

struct Tickspeed {
    Decimal cost;
    Decimal effect;
    Decimal costScaling = {1,1};
    Decimal amount;
    Tickspeed(Decimal _cost, Decimal _effect, Decimal _amount) : cost(_cost), effect(_effect), amount(_amount) {};
    
    bool buyTickspeed(Player& a) {
        if (Decimal::gte(a.antimatter, cost)) {
            amount = amount + DC::D1;
            cost = cost * costScaling;
            return true;
        }; 
        return false;
    };
};

struct Galaxy {
    bool requirement;
    Decimal effect = {1,1};
    Decimal amount = DC::D0;

    Galaxy(Decimal _effect) : effect(_effect) {};

    bool buyGalaxy(Player& a) {
        bool bought = false;
        if (requirement) {
            amount = amount + DC::D1;
            return true;
        };
        return false;
    };
};

int main() {
    Player player(Decimal(1,1));
    Dimensions dims;
    for (const auto& d : dims.dims) {
        cout << "Currency cost(" << d.cost.mantissa << ", " << d.cost.exponent << "), scaling(" << d.scaling.mantissa << ", " << d.scaling.exponent << ") "  << "\n";
    };
    return 0;
};

void tick(Player& player, Dimensions& dims) {



    // apply resources
    for (int i = (int)dims.dims.size() - 1; i >= 0; i--) {
        if (i == 0) {
                // note to jade: + operator is overwritten short hand for .add() Decimal method.
                // it accepts the `this` which is the value to the left, 
                // and the value on the right.
            player.antimatter = player.antimatter + dims.dims[0].production;
        } else if (dims.dims[i].unlocked == true) {
            dims.dims[i - 1].amountAuto = dims.dims[i - 1].amountAuto + dims.dims[i].production;
        };
    };
};