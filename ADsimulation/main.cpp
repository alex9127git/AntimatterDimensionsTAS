#include <iostream>
#include <vector>
#include <cmath>


/*  NOTE:
*   When using numbers that interact with any Decimal value
*   intialize it as a Decimal type.
*/
using namespace std;


struct Decimal {
    double mantissa;
    int exponent;
    Decimal(double _mant, int _exp) : mantissa(_mant), exponent(_exp) {};

    Decimal operator+(const Decimal& b) {
        return Decimal::add(*this, b);
    };
    Decimal operator*(const Decimal& b) {
        return Decimal::multiply(*this, b);
    };

    // note to Jade: 
    // do not use recursive behavior in C++... dummy
    // while loops are better.
    static Decimal multiply(const Decimal& a, const Decimal& b) {
        if (a.mantissa == 0 || b.mantissa == 0) 
            return Decimal(0,0);
        Decimal result(
            a.mantissa * b.mantissa,
            a.exponent + b.exponent
        );
        while (result.mantissa >= 10) {
            result.mantissa /= 10;
            result.exponent++;
        };
        while (result.mantissa > 0 && result.mantissa < 1) {
            result.mantissa *= 10;
            result.exponent--; 
        };
        return result;
    };

    static Decimal add(const Decimal& a, const Decimal& b) {
        if (a.mantissa == 0) return b;
        if (b.mantissa == 0) return a;
        Decimal result(0, 0);
        if (a.exponent >= b.exponent) {
            double scaled = b.mantissa * pow(10, b.exponent - a.exponent);
            result.mantissa = a.mantissa + scaled;
            result.exponent = a.exponent;
        } else {
            double scaled = a.mantissa * pow(10, a.exponent - b.exponent);

            result.mantissa = b.mantissa + scaled;
            result.exponent = b.exponent;
        };
        while (result.mantissa >= 10.0) {
            result.mantissa /= 10.0;
            result.exponent++;
        };
        return result;
    };

    static bool gte(const Decimal& a, const Decimal& b) {
        if (a.exponent != b.exponent) 
            return a.exponent > b.exponent; 
        return a.mantissa >= b.mantissa; 
    };
    static bool gt(const Decimal& a, const Decimal& b) {
        if (a.exponent != b.exponent) 
            return a.exponent > b.exponent; 
        return a.mantissa > b.mantissa; 
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
        {Decimal(0,0), Decimal(1,1), Decimal(1,3), Decimal(0,0), Decimal(0,0), true},
        {Decimal(0,0), Decimal(1,3), Decimal(1,4), Decimal(0,0), Decimal(0,0), false},
        {Decimal(0,0), Decimal(1,5), Decimal(1,5), Decimal(0,0), Decimal(0,0), false},
        {Decimal(0,0), Decimal(1,7), Decimal(1,6), Decimal(0,0), Decimal(0,0), false},
        {Decimal(0,0), Decimal(1,10), Decimal(1,8), Decimal(0,0), Decimal(0,0), false},
        {Decimal(0,0), Decimal(1,14), Decimal(1,10), Decimal(0,0), Decimal(0,0), false},
        {Decimal(0,0), Decimal(1,19), Decimal(1,12), Decimal(0,0), Decimal(0,0), false},
        {Decimal(0,0), Decimal(1,25), Decimal(1,15), Decimal(0,0), Decimal(0,0), false}
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
            amount = amount + {1,0};
            cost = cost * costScaling;
            return true;
        }; 
        return false;
    };
};

struct Galaxy {
    bool requirement;
    Decimal effect = {1,1};
    Decimal amount = {0,0};

    Galaxy(Decimal _effect) : effect(_effect) {};

    bool buyGalaxy(Player& a) {
        bool bought = false;
        if (requirement) {
            amount = amount + {1,0};
            return true;
        };
        return false;
    };
};

int main() {
    Player player(Decimal(1,1));
    Dimensions dims;
    for (const auto& d : dims.dims) {
        cout << "Currenct cost(" << d.cost.mantissa << ", " << d.cost.exponent << "), scaling(" << d.scaling.mantissa << ", " << d.scaling.exponent << ") "  << "\n";
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