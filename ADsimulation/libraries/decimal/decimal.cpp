#include <iostream>
#include <cmath>
#include "decimal.h"
#include "../constants/constants.h"
using namespace std;


Decimal::Decimal(double _mant, int _exp) {
    this->mantissa = _mant;
    this->exponent = _exp;
    this->normalize();
};

Decimal::Decimal(double _mant, double _exp) {
    int expFloor = std::floor(_exp);
    this->mantissa = _mant * std::pow(10, _exp - expFloor);
    this->exponent = expFloor;
    this->normalize();
};

Decimal::Decimal(double _val) {
    this->mantissa = _val;
    this->exponent = 0;
    this->normalize();
};

Decimal Decimal::operator+(const Decimal& b) {
    return Decimal::add(*this, b);
};

Decimal& Decimal::operator+=(const Decimal& b) {
    *this = *this + b;
    return *this;
};

Decimal Decimal::operator-(const Decimal& b) {
    return Decimal::add(*this, Decimal::unary_negate(b));
};

Decimal& Decimal::operator-=(const Decimal& b) {
    *this = *this - b;
    return *this;
};

Decimal Decimal::operator*(const Decimal& b) {
    return Decimal::multiply(*this, b);
};

Decimal Decimal::operator/(const Decimal& b) {
    return Decimal::multiply(*this, Decimal::invert(b));
};

Decimal Decimal::operator-() {
    return Decimal::unary_negate(*this);
}

bool Decimal::operator>=(const Decimal& b) {
    return Decimal::gte(*this, b);
};

bool Decimal::operator>(const Decimal& b) {
    return Decimal::gt(*this, b);
};

bool Decimal::operator<=(const Decimal& b) {
    return Decimal::gte(b, *this);
};

bool Decimal::operator<(const Decimal& b) {
    return Decimal::gt(b, *this);
};

bool Decimal::operator==(const Decimal& b) {
    return Decimal::eq(*this, b);
};

ostream& operator<<(ostream& os, const Decimal& d) {
    char buffer[20];
    if (d.exponent < 3) {
        sprintf(buffer, "%.1f", Decimal::toNumber(d));
        os << buffer;
    } else {
        sprintf(buffer, "%.2f", d.mantissa);
        os << buffer << "e" << d.exponent;
    }
    return os;
};

double Decimal::mant() {
    return this->mantissa;
}

int Decimal::exp() {
    return this->exponent;
}

double Decimal::toNumber(const Decimal& d) {
    return d.mantissa * std::pow(10, d.exponent);
}

Decimal Decimal::add(const Decimal& a, const Decimal& b) {
    if (a.mantissa == 0) return b;
    if (b.mantissa == 0) return a;
    Decimal result = DC::D0;
    if (a.exponent >= b.exponent) {
        double scaled = b.mantissa * std::pow(10, b.exponent - a.exponent);
        result.mantissa = a.mantissa + scaled;
        result.exponent = a.exponent;
    } else {
        double scaled = a.mantissa * std::pow(10, a.exponent - b.exponent);
        result.mantissa = b.mantissa + scaled;
        result.exponent = b.exponent;
    };
    result.normalize();
    return result;
};

// note to Jade: 
// do not use recursive behavior in C++... dummy
// while loops are better.
Decimal Decimal::multiply(const Decimal& a, const Decimal& b) {
    if (a.mantissa == 0 || b.mantissa == 0) 
        return DC::D0;
    Decimal result(
        a.mantissa * b.mantissa,
        a.exponent + b.exponent
    );
    result.normalize();
    return result;
};

Decimal Decimal::unary_negate(const Decimal& a) {
    return Decimal(-a.mantissa, a.exponent);
};

Decimal Decimal::invert(const Decimal& a) {
    return Decimal(1.0 / a.mantissa, -a.exponent);
};

Decimal Decimal::pow(const Decimal& base, const Decimal& power) {
    return Decimal::pow(base, Decimal::toNumber(power));
};

Decimal Decimal::pow(const Decimal& base, const double& power) {
    return Decimal(
        std::pow(base.mantissa, power),
        base.exponent * power
    );
};

Decimal Decimal::max(const Decimal& a, const Decimal& b) {
    return Decimal::gt(a, b) ? a : b;
};

Decimal Decimal::min(const Decimal& a, const Decimal& b) {
    return Decimal::gt(a, b) ? b : a;
};

bool Decimal::gte(const Decimal& a, const Decimal& b) {
    return Decimal::gt(a, b) || Decimal::eq(a, b);
};

bool Decimal::gt(const Decimal& a, const Decimal& b) {
    if (Decimal::eq(a, DC::D0)) return b.mantissa < 0;
    if (Decimal::eq(b, DC::D0)) return a.mantissa > 0;
    if (a.mantissa > 0 && b.mantissa < 0) return true;
    if (b.mantissa > 0 && a.mantissa < 0) return false;
    if (a.exponent != b.exponent) return a.exponent > b.exponent; 
    return a.mantissa > b.mantissa; 
};

bool Decimal::eq(const Decimal& a, const Decimal& b) {
    return abs(a.mantissa - b.mantissa) < 1e-12 && a.exponent == b.exponent;
}

void Decimal::normalize() {
    bool isNegative = *this < DC::D0;
    if (isNegative) this->mantissa = -this->mantissa;
    while (this->mantissa >= 10) {
        this->mantissa /= 10.0;
        this->exponent++;
    }
    while (this->mantissa > 0 && this->mantissa < 1) {
        this->mantissa *= 10;
        this->exponent--; 
    }
    if (isNegative) this->mantissa = -this->mantissa;
}

void Decimal::print() {
    cout << "Decimal(" << this->mantissa << ", " << this->exponent << ")" << endl;
}
