#include <permutations.h>
#include <iostream>
using namespace std;


vector<vector<int>> getPermutations(map<int, int>& items) {
    vector<vector<int>> result;
    vector<int> stack;
    vector<int> keys;
    vector<int> values;
    int total = 0;
    for (pair<int, int> pair : items) {
        if (pair.second != 0) {
            keys.push_back(pair.first);
            values.push_back(pair.second);
            total += pair.second;
        }
    }
    getPermutations(keys, values, stack, result, total);
    return result;
}

void getPermutations(
    vector<int>& keys,
    vector<int>& values,
    vector<int>& stack,
    vector<vector<int>>& result,
    int remaining
) {
    if (remaining == 0) {
        result.push_back(stack);
        return;
    }
    for (int i = 0; i < keys.size(); i++) {
        int k = keys[i];
        int v = values[i];
        if (v == 0) continue;
        stack.push_back(k);
        values[i]--;
        getPermutations(keys, values, stack, result, remaining - 1);
        values[i]++;
        stack.pop_back();
    }
}
