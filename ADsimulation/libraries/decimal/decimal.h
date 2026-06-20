struct Decimal {
    double mantissa;
    int exponent;

    Decimal(double _mant, int _exp);
    Decimal operator+(const Decimal& b);
    Decimal operator*(const Decimal& b);
    static Decimal add(const Decimal& a, const Decimal& b);
    static Decimal multiply(const Decimal& a, const Decimal& b);
    static bool gte(const Decimal& a, const Decimal& b);
    static bool gt(const Decimal& a, const Decimal& b);
};