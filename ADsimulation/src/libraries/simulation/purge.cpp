#include "simulation.h"
#include "constants.h"
#include "purge.h"
#include <algorithm>
#include <numeric>
#include <map>
using namespace std;


void Grouping::initialize(vector<SearchNode>& nodes, vector<int>& indices) {
    size_t size = nodes.size();
    groupLookup.resize(size);
    map<Key, int> keyLookup;
    for (size_t i = 0; i < size; i++) {
        Key key = getPurchasesFrom(nodes[indices[i]].game());
        auto [entry, inserted] = keyLookup.try_emplace(key, keys.size());
        if (inserted) keys.push_back(key);
        groupLookup[i] = entry->second;
    }
    groupCnt = keys.size();
    
    // If there are too many groups, don't initialize the comparison table - will take way too long otherwise
    if (groupCnt > GROUP_THRESHOLD) return;
    size_t keySize = keys[0].size();
    groupCompares.resize(groupCnt * groupCnt);
    for (size_t i = 0; i < groupCnt; i++) {
        for (size_t j = 0; j < groupCnt; j++) {
            bool comparison = true;
            for (size_t k = 0; k < keySize; k++) comparison &= keys[i][k] >= keys[j][k];
            groupCompares[i * groupCnt + j] = comparison;
        }
    }
}

bool Grouping::canDominate(int a, int b) const {
    if (!groupCompares.empty()) return groupCompares[a * groupCnt + b];
    for (size_t k = 0; k < keys[a].size(); k++) {
        if (keys[a][k] < keys[b][k]) return false;
    }
    return true;
}

void appendValues(const GameState& st, vector<double>& values) {
    values.push_back(st.sacrifice().getSacrificeBonus().log10value());
    values.push_back(st.antimatter().log10value());
    for (int d = 1; d <= 8; d++) values.push_back(st.AD()[d].getAmount().log10value());
    values.push_back(st.tickspeed().perSecond().log10value());
    values.push_back(st.getAchievementBonus().log10value());
}

Key getPurchasesFrom(const GameState& st) {
    Key result;
    for (int d = 1; d <= 8; d++) {
        result[d] = st.AD()[d].getPurchases();
    }
    result[0] = st.tickspeed().getPurchases();
    return result;
}

int compare(const double* st1, const double* st2, size_t width, bool strictlyBetter) {
    // Compare the two game states together.
    // Returns 1 if all st1's values are bigger or equal than that of st2,
    // and at least one value is strictly bigger.
    // Returns 0 if all compared values are equal
    // Returns -1 otherwise.
    if (st1[0] < st2[0] - SAC_BONUS_EPSILON) return -1;
    if (st1[0] > st2[0] + SAC_BONUS_EPSILON) strictlyBetter = true;
    for (size_t i = 1; i < width; i++) {
        if (st1[i] < st2[i]) return -1;
        if (st1[i] > st2[i]) strictlyBetter = true;
    }
    return strictlyBetter ? 1 : 0;
}

bool isFutureEqual(const SearchNode& nodeA, const SearchNode& nodeB) {
    if (nodeA.queue().getQueuedPurchases() != nodeB.queue().getQueuedPurchases()) return false;
    if (nodeA.queue().getQueuedSacrifices() != nodeB.queue().getQueuedSacrifices()) return false;
    if (nodeA.getNextSacValue() != nodeB.getNextSacValue()) return false;
    if (nodeA.getProjRouteIndex() != nodeB.getProjRouteIndex()) return false;
    return true;
}

void purge(vector<SearchNode>& nodes, const function<void(int, double)>& onProgress) {
    size_t size = nodes.size();
    if (size == 0) return;

    // Sort savestates by decreasing AD1 amount
    vector<int> indices(size);
    iota(indices.begin(), indices.end(), 0);
    sort(indices.begin(), indices.end(), [&nodes](int i, int j) {
        return nodes[i].game().AD()[1].getAmount() > nodes[j].game().AD()[1].getAmount();
    });

    Grouping grouping;
    grouping.initialize(nodes, indices);

    vector<double> values;
    size_t width = 0;
    for (size_t i = 0; i < size; i++) {
        appendValues(nodes[indices[i]].game(), values);
        if (i == 0) {
            width = values.size();
            values.reserve(size * width);
        }
    }
    vector<char> removedCopy(size);
    int statesPurged = 0;
    double iterationsTotal = (double) (size) * (size - 1) / 2.0;
    #pragma omp parallel if(size > PARALLEL_THRESHOLD)
    for (size_t i = 0; i < size; i++) {
        #pragma omp single
        if (onProgress) {
            double iterationsDone = (double) i * size - (double) i * (i + 1) / 2;
            onProgress(statesPurged, iterationsDone / iterationsTotal);
        }
        if (removedCopy[i] == 1) continue;
        int groupI = grouping.groupLookup[i];
        #pragma omp for reduction(+:statesPurged)
        for (size_t j = i + 1; j < size; j++) {
            if (removedCopy[j] == 1) continue;
            int groupJ = grouping.groupLookup[j];
            if (!grouping.canDominate(groupI, groupJ)) continue;
            int compareResult = compare(&values[i * width], &values[j * width], width, groupI != groupJ);
            if (compareResult == 1) {
                removedCopy[j] = 1;
                statesPurged++;
            } else if (compareResult == 0) {
                const SearchNode& nodeA = nodes[indices[i]];
                const SearchNode& nodeB = nodes[indices[j]];
                if (isFutureEqual(nodeA, nodeB)) {
                    removedCopy[j] = 1;
                    statesPurged++;
                }
            }
        }
    }
    vector<char> removedOrig(size);
    for (size_t i = 0; i < size; i++) {
        removedOrig[indices[i]] = removedCopy[i];
    }
    vector<SearchNode> result;
    int i = 0;
    for (auto stIter = nodes.begin(); stIter != nodes.end(); stIter++) {
        if (removedOrig[i] == 0) {
            result.push_back(move(*stIter));
        }
        i++;
    }
    nodes.swap(result);
}
