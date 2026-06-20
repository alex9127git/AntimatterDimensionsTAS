#include <cmath>
#include "decimal.h"


Decimal::Decimal(double _mant, int _exp) : mantissa(_mant), exponent(_exp) {};

Decimal Decimal::operator+(const Decimal& b) {
    return Decimal::add(*this, b);
};
Decimal Decimal::operator*(const Decimal& b) {
    return Decimal::multiply(*this, b);
};

// note to Jade: 
// do not use recursive behavior in C++... dummy
// while loops are better.
Decimal Decimal::multiply(const Decimal& a, const Decimal& b) {
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

Decimal Decimal::add(const Decimal& a, const Decimal& b) {
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

bool Decimal::gte(const Decimal& a, const Decimal& b) {
    if (a.exponent != b.exponent) 
        return a.exponent > b.exponent; 
    return a.mantissa >= b.mantissa; 
};
bool Decimal::gt(const Decimal& a, const Decimal& b) {
    if (a.exponent != b.exponent) 
        return a.exponent > b.exponent; 
    return a.mantissa > b.mantissa; 
};