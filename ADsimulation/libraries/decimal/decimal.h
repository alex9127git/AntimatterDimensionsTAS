#pragma once
#include <iostream>
using namespace std;


class Decimal {
    private:
        double mantissa;
        int exponent;

    public:
        Decimal(double _mant, int _exp);
        Decimal(double _val);
        Decimal operator+(const Decimal& b);
        Decimal& operator+=(const Decimal& b);
        Decimal operator-(const Decimal& b);
        Decimal& operator-=(const Decimal& b);
        Decimal operator*(const Decimal& b);
        Decimal operator-();
        bool operator>=(const Decimal& b);
        bool operator>(const Decimal& b);
        bool operator<=(const Decimal& b);
        bool operator<(const Decimal& b);
        bool operator==(const Decimal& b);
        friend ostream& operator<<(ostream& os, const Decimal& d);

        static Decimal add(const Decimal& a, const Decimal& b);
        static Decimal multiply(const Decimal& a, const Decimal& b);
        static Decimal unary_negate(const Decimal& a);

        static bool gte(const Decimal& a, const Decimal& b);
        static bool gt(const Decimal& a, const Decimal& b);

        void normalize();

        void print();
};