#pragma once
#include "../interfaces/interfaces.h"
#include <iostream>
using namespace std;


class Decimal : public ISerializable {
    private:
        double mantissa;
        long long exponent;

    public:
        Decimal(double _mant, long long _exp);
        Decimal(double _mant, double _exp);
        Decimal(double _val);
        Decimal(json& j);

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
        bool operator==(const Decimal& b) const;
        friend ostream& operator<<(ostream& os, const Decimal& d);

        double mant();
        long long exp();

        static double toNumber(const Decimal& d);

        static Decimal add(const Decimal& a, const Decimal& b);
        static Decimal multiply(const Decimal& a, const Decimal& b);
        static Decimal unary_negate(const Decimal& a);
        static Decimal invert(const Decimal& a);
        static Decimal pow(const Decimal base, const Decimal power);
        static Decimal pow(const Decimal base, const double power);

        static Decimal max(const Decimal& a, const Decimal& b);
        static Decimal min(const Decimal& a, const Decimal& b);

        static bool gte(const Decimal& a, const Decimal& b);
        static bool gt(const Decimal& a, const Decimal& b);
        static bool eq(const Decimal& a, const Decimal& b);

        void normalize();

        void repr();

        static string toString(const Decimal& d, int precision);
        string toString(int precision);

        json to_json() override;
        void from_json(json& j) override;
};