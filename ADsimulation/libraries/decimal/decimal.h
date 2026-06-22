#pragma once
#include <iostream>
using namespace std;


class Decimal {
    private:
        double mantissa;
        int exponent;

    public:
        Decimal(double _mant, int _exp);
        Decimal(double _mant, double _exp);
        Decimal(double _val);

        Decimal operator+(const Decimal& b);
        Decimal& operator+=(const Decimal& b);
        Decimal operator-(const Decimal& b);
        Decimal& operator-=(const Decimal& b);
        Decimal operator*(const Decimal& b);
        Decimal& operator*=(const Decimal& b);
        Decimal operator/(const Decimal& b);
        Decimal& operator/=(const Decimal& b);
        Decimal operator-();
        bool operator>=(const Decimal& b);
        bool operator>(const Decimal& b);
        bool operator<=(const Decimal& b);
        bool operator<(const Decimal& b);
        bool operator==(const Decimal& b);
        friend ostream& operator<<(ostream& os, const Decimal& d);

        double mant();
        int exp();

        static double toNumber(const Decimal& d);

        static Decimal add(const Decimal& a, const Decimal& b);
        static Decimal multiply(const Decimal& a, const Decimal& b);
        static Decimal unary_negate(const Decimal& a);
        static Decimal invert(const Decimal& a);
        static Decimal pow(const Decimal& base, const Decimal& power);
        static Decimal pow(const Decimal& base, const double& power);

        static Decimal max(const Decimal& a, const Decimal& b);
        static Decimal min(const Decimal& a, const Decimal& b);

        static bool gte(const Decimal& a, const Decimal& b);
        static bool gt(const Decimal& a, const Decimal& b);
        static bool eq(const Decimal& a, const Decimal& b);

        void normalize();

        void print();
};