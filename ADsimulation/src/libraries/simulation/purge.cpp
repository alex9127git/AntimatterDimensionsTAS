#include "simulation.h"
#include "constants.h"
#include "purge.h"
#include <algorithm>
#include <numeric>
using namespace std;


void appendValues(const GameState& st, vector<double>& values) {
    values.push_back(st.antimatter().log10value());
    for (int d = 1; d <= 8; d++) {
        values.push_back(st.AD()[d].getAmount().log10value());
        values.push_back(st.AD()[d].getPurchases());
    }
    values.push_back(st.sacrifice().getSacrificeBonus().log10value());
    values.push_back(st.tickspeed().getPurchases());
    values.push_back(st.tickspeed().perSecond().log10value());
    values.push_back(st.getAchievementBonus().log10value());
}


bool compare(const double* st1, const double* st2, size_t width) {
    // Compare the two game states together.
    // Returns true if all st1's values are bigger or equal than that of st2,
    // and at least one value is strictly bigger.
    // Otherwise, return false.
    bool strictlyBetter = false;
    for (size_t i = 0; i < width; i++) {
        if (st1[i] < st2[i]) return false;
        if (st1[i] > st2[i]) strictlyBetter = true;
    }
    return strictlyBetter;
}

void purge(vector<SearchNode>& nodes, const function<void(int, double)>& onProgress) {
    size_t size = nodes.size();
    if (size == 0) return;
    vector<int> indices(size);
    iota(indices.begin(), indices.end(), 0);
    sort(indices.begin(), indices.end(), [&nodes](int i, int j) {
        return nodes[i].game().AD()[1].getAmount() > nodes[j].game().AD()[1].getAmount();
    });
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
        #pragma omp for reduction(+:statesPurged)
        for (size_t j = i + 1; j < size; j++) {
            if (removedCopy[j] == 1) continue;
            if (compare(&values[i * width], &values[j * width], width)) {
                removedCopy[j] = 1;
                statesPurged++;
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
