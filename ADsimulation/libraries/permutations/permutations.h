#include <iterator>
#include <cstddef>
#include <map>
#include <vector>
using namespace std;


vector<vector<int>> getPermutations(map<int, int>& items);
void getPermutations(
    vector<int>& keys,
    vector<int>& values,
    vector<int>& stack,
    vector<vector<int>>& result,
    int remaining
);