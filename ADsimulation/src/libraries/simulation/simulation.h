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

GameState runDimboost(GameState st);
GameState runDimboost(GameState st, int precision);
GameState runDimboost(GameState st, int precision, bool verbose);
vector<GameState> purchaseRun(GameState st, function<bool(GameState&)> stopCondition, bool verbose);
vector<GameState> sacrificeRun(GameState st, function<bool(GameState&)> stopCondition, int precision, vector<vector<double>> purchaseStrategies, bool verbose);

int compare(GameState& st1, GameState& st2);
void compareValues(Decimal v1, Decimal v2, int& score, int& totalFeatures);
void compareValues(int v1, int v2, int& score, int& totalFeatures);

vector<GameState> purge(vector<GameState>& gamestates, bool verbose);
