#include "../gamestate/gamestate.h"
#include "chrono"
#include "vector"


class Timer {
    private:
        chrono::time_point<chrono::steady_clock> start;

    public:
        Timer() : start(chrono::steady_clock::now()) {}

        void reset() {
            auto end = chrono::steady_clock::now();
            chrono::duration<double, std::milli> elapsed = end - start;
            start = end;
            std::cout << elapsed.count() << " ms" << endl;
        }

        double silentReset() {
            auto end = chrono::steady_clock::now();
            chrono::duration<double, std::milli> elapsed = end - start;
            start = end;
            return elapsed.count();
        }
};

GameState runDimboost(GameState st, bool useSacrifice);
GameState runDimboost(GameState st, bool useSacrifice, int precision);
GameState runDimboost(GameState st, bool useSacrifice, int precision, bool verbose);
GameState runUntil(
    GameState st, 
    function<bool(GameState&)> stopCondition, 
    bool useSacrifice, int precision, bool verbose);
vector<GameState> purchaseRun(
    GameState st, 
    function<bool(GameState&)> stopCondition, 
    vector<vector<double>> sacrificeStrategies, bool verbose);
vector<GameState> sacrificeRun(
    GameState st, 
    function<bool(GameState&)> stopCondition, int precision, 
    vector<vector<double>> purchaseStrategies, bool verbose);
vector<GameState> deepSacRun(
    GameState st, 
    function<bool(GameState&)> stopCondition, 
    vector<vector<double>> purchaseStrategies, 
    vector<vector<double>> sacrificeStrategies, bool verbose);

void computePurchaseBranches(GameState& gameState, vector<GameState>& branches);

//void check(GameStates& gameStates, bool& isFinished);

bool compare(const double* st1, const double* st2, size_t width);
void purge(vector<GameState>& gamestates, bool verbose);
