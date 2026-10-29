#include "search.h"
#include "../gamestate/gamestate.h"
#include "../constants/constants.h"
#include <algorithm>
using namespace std;


vector<uint16_t> computeAllowedPurchases(const GameState& st) {
    vector<uint16_t> allowedPurchases;
    int frontier = 1;
    while (st.AD()[frontier].getPurchases() >= 10 && frontier < min(st.dimensionBoosts() + 4, 8)) {
        frontier++;
    }
    for (int i = 1; i <= frontier; i++) {
        if (st.AD()[i].getPurchases() > 10 && st.AD()[i].getPurchases() % 10 != 0) {
            allowedPurchases.push_back(i);
            return allowedPurchases;
        }
    }
    Decimal minPrice = st.tickspeed().getCost();
    for (int i = 1; i < frontier; i++) {
        minPrice = min(st.AD()[i].getCost() * DC::D10, minPrice);
    }
    Decimal frontierPrice = st.AD()[frontier].getCost() * DC::D10;
    if (frontierPrice > DC::D0_2 * minPrice) {
        if (st.tickspeed().getCost() == minPrice) allowedPurchases.push_back(9);
        for (int i = 1; i < frontier; i++) {
            if (st.AD()[i].getCost() * DC::D10 == minPrice) allowedPurchases.push_back(i);
        }
    }
    if (frontierPrice < DC::D20 * minPrice) {
        allowedPurchases.push_back(frontier);
    }
    stable_sort(allowedPurchases.begin(), allowedPurchases.end(),
        [&](uint16_t a, uint16_t b) {
            Decimal a_cost = st.costOf(a);
            Decimal b_cost = st.costOf(b);
            return a_cost < b_cost;
        }
    );
    return allowedPurchases;
}