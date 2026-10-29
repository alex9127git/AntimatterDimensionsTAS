#include "searchnode.h"
#include <algorithm>
using namespace std;


SearchNode::SearchNode(const GameState& st) 
    :   instance(st)
    {}

bool SearchNode::canBranch() {
    if (!validated) {
        computeAllowedPurchases();
        nextPurchaseCost = game().costOf(allowedPurchases.front());
        validated = true;
    }
    return game().antimatter() >= nextPurchaseCost;
}

const vector<uint16_t>& SearchNode::getPurchaseBranches() const {
    return this->allowedPurchases;
}

void SearchNode::computeAllowedPurchases() {
    const GameState& st = game();
    allowedPurchases.clear();
    int frontier = 1;
    while (st.AD()[frontier].getPurchases() >= 10 && frontier < min(st.dimensionBoosts() + 4, 8)) {
        frontier++;
    }
    for (int i = 1; i <= frontier; i++) {
        if (st.AD()[i].getPurchases() > 10 && st.AD()[i].getPurchases() % 10 != 0) {
            allowedPurchases.push_back(i);
            return;
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
}

bool SearchNode::canSacBranch() const {
    return game().AD()[1].getAmount() >= this->nextSacrificeBranch;
}

void SearchNode::setNextSacBranching(Decimal sacValue) {
    this->nextSacrificeBranch = game().sacrifice().getReqForSac(sacValue);
}
