#include <iostream>
#include <vector>
#include <cmath>

using namespace std;


struct Decimal {
    double mantissa;
    int exponent;
    Decimal(double _mant, int _exp) : mantissa(_mant), exponent(_exp) {};

    Decimal operator+(const Decimal& b) {
        return Decimal::add(*this, b);
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
    static void buyDimension(Dimension& a, Decimal _amount) {
        a.amountManual = a.amountManual + _amount;
    }
};

struct Player {
    Decimal antimatter;

    Player(Decimal am)
        : antimatter(am) {};
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
    double cost;
    double effect;
    int costScaling = 10;
    Tickspeed(double _cost, double _effect) : cost(_cost), effect(_effect) {};
    
    void buyTickspeed() {
        cost *= costScaling;
    };
};

struct Galaxy {
    double cost;
    int requirement;

    Galaxy(double _cost, int _req) : cost(_cost), requirement(_req) {};
};

bool buyGalaxy(Decimal& a, Decimal& b) {
    bool bought = false;
    if (a > )
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