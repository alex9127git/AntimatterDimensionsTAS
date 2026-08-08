#include "constants.h"
#include <cstdint>


Decimal DC::D0 = Decimal(0);
Decimal DC::D1 = Decimal(1);

Decimal DC::D1_01 = Decimal(1.01);
Decimal DC::D1_02 = Decimal(1.02);
Decimal DC::D1_03 = Decimal(1.03);
Decimal DC::D1_05 = Decimal(1.05);
Decimal DC::D1_1 = Decimal(1.1);
Decimal DC::D1_25 = Decimal(1.25);
Decimal DC::D1_4 = Decimal(1.4);
Decimal DC::D1_5 = Decimal(1.5);

Decimal DC::D2 = Decimal(2);
Decimal DC::D3 = Decimal(3);
Decimal DC::D4 = Decimal(4);
Decimal DC::D6 = Decimal(6);
Decimal DC::D10 = Decimal(10);
Decimal DC::D100 = Decimal(100);

Decimal DC::D1E3 = Decimal(1, (int64_t) 3LL);
Decimal DC::D5E3 = Decimal(5, (int64_t) 3LL);
Decimal DC::D1E4 = Decimal(1, (int64_t) 4LL);
Decimal DC::D1E5 = Decimal(1, (int64_t) 5LL);
Decimal DC::D5E5 = Decimal(5, (int64_t) 5LL);
Decimal DC::D1E6 = Decimal(1, (int64_t) 6LL);
Decimal DC::D1E7 = Decimal(1, (int64_t) 7LL);
Decimal DC::D1E8 = Decimal(1, (int64_t) 8LL);
Decimal DC::D1E9 = Decimal(1, (int64_t) 9LL);
Decimal DC::D1E10 = Decimal(1, (int64_t) 10LL);
Decimal DC::D5E10 = Decimal(5, (int64_t) 10LL);
Decimal DC::D1E12 = Decimal(1, (int64_t) 12LL);
Decimal DC::D1E13 = Decimal(1, (int64_t) 13LL);
Decimal DC::D1E15 = Decimal(1, (int64_t) 15LL);
Decimal DC::D1E18 = Decimal(1, (int64_t) 18LL);
Decimal DC::D1E24 = Decimal(1, (int64_t) 24LL);
Decimal DC::D5E25 = Decimal(5, (int64_t) 25LL);
Decimal DC::D1E26 = Decimal(1, (int64_t) 26LL);
Decimal DC::D1E31 = Decimal(1, (int64_t) 31LL);
Decimal DC::D1E63 = Decimal(1, (int64_t) 63LL);
Decimal DC::D1E80 = Decimal(1, (int64_t) 80LL);

Decimal DC::INF = Decimal::pow(DC::D2, 1024);
