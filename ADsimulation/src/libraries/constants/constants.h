#pragma once
#include "decimal.h"
#include <cstdint>


struct DC {
    inline static const Decimal D0 = Decimal(0);
    inline static const Decimal D0_2 = Decimal(0.2);
    inline static const Decimal D1 = Decimal(1);

    inline static const Decimal D1_01 = Decimal(1.01);
    inline static const Decimal D1_02 = Decimal(1.02);
    inline static const Decimal D1_03 = Decimal(1.03);
    inline static const Decimal D1_05 = Decimal(1.05);
    inline static const Decimal D1_1 = Decimal(1.1);
    inline static const Decimal D1_25 = Decimal(1.25);
    inline static const Decimal D1_4 = Decimal(1.4);
    inline static const Decimal D1_5 = Decimal(1.5);

    inline static const Decimal D2 = Decimal(2);
    inline static const Decimal D3 = Decimal(3);
    inline static const Decimal D4 = Decimal(4);
    inline static const Decimal D6 = Decimal(6);
    inline static const Decimal D10 = Decimal(10);
    inline static const Decimal D20 = Decimal(20);
    inline static const Decimal D100 = Decimal(100);
    inline static const Decimal D600 = Decimal(600);

    inline static const Decimal D1E3 = Decimal(1, (int64_t) 3LL);
    inline static const Decimal D5E3 = Decimal(5, (int64_t) 3LL);
    inline static const Decimal D1E4 = Decimal(1, (int64_t) 4LL);
    inline static const Decimal D1E5 = Decimal(1, (int64_t) 5LL);
    inline static const Decimal D5E5 = Decimal(5, (int64_t) 5LL);
    inline static const Decimal D1E6 = Decimal(1, (int64_t) 6LL);
    inline static const Decimal D1E7 = Decimal(1, (int64_t) 7LL);
    inline static const Decimal D1E8 = Decimal(1, (int64_t) 8LL);
    inline static const Decimal D1E9 = Decimal(1, (int64_t) 9LL);

    inline static const Decimal D1E10 = Decimal(1, (int64_t) 10LL);
    inline static const Decimal D5E10 = Decimal(5, (int64_t) 10LL);
    inline static const Decimal D1E12 = Decimal(1, (int64_t) 12LL);
    inline static const Decimal D1E13 = Decimal(1, (int64_t) 13LL);
    inline static const Decimal D1E15 = Decimal(1, (int64_t) 15LL);
    inline static const Decimal D1E18 = Decimal(1, (int64_t) 18LL);
    inline static const Decimal D1E24 = Decimal(1, (int64_t) 24LL);
    inline static const Decimal D5E25 = Decimal(5, (int64_t) 25LL);
    inline static const Decimal D1E28 = Decimal(1, (int64_t) 28LL);
    inline static const Decimal D1E31 = Decimal(1, (int64_t) 31LL);
    inline static const Decimal D1E63 = Decimal(1, (int64_t) 63LL);
    inline static const Decimal D1E80 = Decimal(1, (int64_t) 80LL);

    inline static const Decimal D1E150 = Decimal(1, (int64_t) 150LL);

    inline static const Decimal INF = Decimal::pow(D2, 1024);
};


inline constexpr size_t PARALLEL_THRESHOLD = 1000;
inline constexpr int64_t MAX_TICKS = 5000000;
