#include <iostream>
#include <string>
#include <format>
#include <cmath>
#include "decimal.h"
#include "../constants/constants.h"
using namespace std;


Decimal::Decimal(double _mant, long long _exp) {
    this->mantissa = _mant;
    this->exponent = _exp;
    this->normalize();
}

Decimal::Decimal(double _mant, double _exp) {
    int expFloor = std::floor(_exp);
    this->mantissa = _mant * std::pow(10, _exp - expFloor);
    this->exponent = expFloor;
    this->normalize();
}

Decimal::Decimal(double _val) {
    this->mantissa = _val;
    this->exponent = 0;
    this->normalize();
}

Decimal::Decimal(json& j) {
    this->from_json(j);
    this->normalize();
}

Decimal Decimal::operator+(const Decimal& b) {
    return Decimal::add(*this, b);
}

Decimal& Decimal::operator+=(const Decimal& b) {
    if (b.mantissa == 0) {
        return *this;
    }
    if (this->mantissa == 0) {
        this->mantissa = b.mantissa;
        this->exponent = b.exponent;
    } else if (abs(this->exponent - b.exponent) > 30) {
        const Decimal& bigger = Decimal::max(*this, b);
        this->mantissa = bigger.mantissa;
        this->exponent = bigger.exponent;
    } else if (this->exponent >= b.exponent) {
        double scaled = b.mantissa * std::pow(10, b.exponent - this->exponent);
        this->mantissa = this->mantissa + scaled;
    } else {
        double scaled = this->mantissa * std::pow(10, this->exponent - b.exponent);
        this->mantissa = b.mantissa + scaled;
        this->exponent = b.exponent;
    }
    this->normalize();
    return *this;
}

Decimal Decimal::operator-(const Decimal& b) {
    return Decimal::add(*this, Decimal::unary_negate(b));
}

Decimal& Decimal::operator-=(const Decimal& b) {
    return *this += Decimal::unary_negate(b);
}

Decimal Decimal::operator*(const Decimal& b) {
    return Decimal::multiply(*this, b);
}

Decimal& Decimal::operator*=(const Decimal& b) {
    this->mantissa *= b.mantissa;
    this->exponent += b.exponent;
    this->normalize();
    return *this;
}

Decimal Decimal::operator/(const Decimal& b) {
    return Decimal::multiply(*this, Decimal::invert(b));
}

Decimal& Decimal::operator/=(const Decimal& b) {
    return *this *= Decimal::invert(b);
}

Decimal Decimal::operator-() {
    return Decimal::unary_negate(*this);
}

bool Decimal::operator>=(const Decimal& b) {
    return Decimal::gte(*this, b);
}

bool Decimal::operator>(const Decimal& b) {
    return Decimal::gt(*this, b);
}

bool Decimal::operator<=(const Decimal& b) {
    return Decimal::gte(b, *this);
}

bool Decimal::operator<(const Decimal& b) {
    return Decimal::gt(b, *this);
}

bool Decimal::operator==(const Decimal& b) const {
    return Decimal::eq(*this, b);
}

ostream& operator<<(ostream& os, const Decimal& d) {
    os << Decimal::toString(d, 2);
    return os;
}

Decimal Decimal::log10() {
    return Decimal(this->exponent + std::log10(this->mantissa));
}

double Decimal::toNumber(const Decimal& d) {
    return d.mantissa * std::pow(10, d.exponent);
}

Decimal Decimal::add(const Decimal& a, const Decimal& b) {
    if (a.mantissa == 0) return b;
    if (b.mantissa == 0) return a;
    if (abs(a.exponent - b.exponent) > 30) return Decimal::max(a, b);
    if (a.exponent >= b.exponent) {
        double scaled = b.mantissa * std::pow(10, b.exponent - a.exponent);
        return Decimal(a.mantissa + scaled, a.exponent);
    } else {
        double scaled = a.mantissa * std::pow(10, a.exponent - b.exponent);
        return Decimal(b.mantissa + scaled, b.exponent);
    }
}

// note to Jade: 
// do not use recursive behavior in C++... dummy
// while loops are better.
Decimal Decimal::multiply(const Decimal& a, const Decimal& b) {
    Decimal result(
        a.mantissa * b.mantissa,
        a.exponent + b.exponent
    );
    return result;
}

Decimal Decimal::unary_negate(const Decimal& a) {
    return Decimal(-a.mantissa, a.exponent);
}

Decimal Decimal::invert(const Decimal& a) {
    return Decimal(1.0 / a.mantissa, -a.exponent);
}

Decimal Decimal::pow(const Decimal base, const Decimal power) {
    return Decimal::pow(base, Decimal::toNumber(power));
}

Decimal Decimal::pow(const Decimal base, const double power) {
    if (power <= 2) {
        return Decimal(
            std::pow(base.mantissa, power),
            base.exponent * power
        );
    } else {
        return Decimal::pow(
            Decimal::pow(base, 2),
            power / 2
        );
    }
}

Decimal Decimal::max(const Decimal& a, const Decimal& b) {
    return Decimal::gt(a, b) ? a : b;
}

Decimal Decimal::min(const Decimal& a, const Decimal& b) {
    return Decimal::gt(a, b) ? b : a;
}

bool Decimal::gte(const Decimal& a, const Decimal& b) {
    return Decimal::gt(a, b) || Decimal::eq(a, b);
}

bool Decimal::gt(const Decimal& a, const Decimal& b) {
    if (a.isCached && b.isCached) return a.cachedValue > b.cachedValue;
    if (a.mantissa == 0) return b.mantissa < 0;
    if (b.mantissa == 0) return a.mantissa > 0;
    if (a.exponent == b.exponent) return a.mantissa > b.mantissa;
    if (a.mantissa > 0) return b.mantissa < 0 || a.exponent > b.exponent;
    return b.mantissa < 0 && a.exponent < b.exponent;
}

bool Decimal::eq(const Decimal& a, const Decimal& b) {
    if (a.mantissa == 0 && b.mantissa == 0) return true;
    return abs(a.mantissa - b.mantissa) < 1e-12 && a.exponent == b.exponent;
}

void Decimal::normalize() {
    if (isinf(this->mantissa)) {
        throw runtime_error("Mantissa isn't supposed to be infinity, you absolute buffoon");
    }
    bool isNegative = this->mantissa < 0;
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

void Decimal::repr() {
    cout << "Decimal(" << this->mantissa << ", " << this->exponent << ")" << endl;
}

void Decimal::cache() {
    this->cachedValue = this->mantissa * std::pow(10, this->exponent);
    this->isCached = !isinf(this->cachedValue);
}

string Decimal::toString(const Decimal& d, int precision) {
    string buffer;
    string fmt;
    if (d.exponent < 3) {
        fmt = "{:." + to_string(precision) + "f}";
        double output = Decimal::toNumber(d);
        buffer = vformat(fmt, make_format_args(output));
    } else {
        fmt = "{:." + to_string(precision) + "f}e{:d}";
        buffer = vformat(fmt, make_format_args(d.mantissa, d.exponent));
    }
    return buffer;
}

string Decimal::toString(int precision) {
    return Decimal::toString(*this, precision);
}

json Decimal::to_json() {
    json j;
    j["mantissa"] = this->mantissa;
    j["exponent"] = this->exponent;
    return j;
}

void Decimal::from_json(json& j) {
    this->mantissa = j["mantissa"];
    this->exponent = j["exponent"];
}
