#pragma once
#include <cstdint>
#include "../interfaces/interfaces.h"
#include <iostream>
using namespace std;


class Decimal : public ISerializable {
    private:
        double mantissa;
        int64_t exponent;
        
        // volatile, shouldn't be serialized
        
        double cachedValue = 0;
        bool isCached = false;

    public:
        inline static const double pow10_table[41] = {
            1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11,
            1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1,
            1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10,
            1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20
        };

        Decimal(double _mant, long long _exp);
        Decimal(double _mant, int64_t _exp);
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

        Decimal log10();

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
        void cache();

        static string toString(const Decimal& d, int precision);
        string toString(int precision);

        json to_json() override;
        void from_json(json& j) override;
};