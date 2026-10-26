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

class Progress {
    private:
        function<bool(const GameState&)> stopCondition;
        function<double(const GameState&)> progressFunction;

    public:
        Progress(function<bool(const GameState&)> stopFunc, function<double(const GameState&)> progressFunc);
        double calculate(const GameState& st) const {
            return clamp(progressFunction(st), 0.0, 1.0);
        };
        bool isFinished(const GameState& st) const {
            return stopCondition(st);
        };

        static Progress untilDimboost()  {
            return Progress(
                [](const GameState& st) {return st.canBuyNextDimboost();}, 
                [](const GameState& st) {
                    return st.antimatterThisReset().log10value() / st.getAntimatterGoalForDimboost().log10value();
                }
            );
        };
};

class Iteration {
    protected:
        Progress progress;
        bool verbose;
        
        vector<GameState> gameStates;
        vector<GameState> newGameStates;
        Timer timer;
        Timer branchTimer;
        Timer checkTimer;
        Timer tickTimer;
        Timer purgeTimer;
        double branchTime = 0;
        double checkTime = 0;
        double tickTime = 0;
        double purgeTime = 0;
        bool isFinished = false;
        int ticks = 0;
        int statesAfterPurge = 1;
        int adds = 0;
        int maxStates = 0;
        int states = 0;

    public:
        Iteration(const Progress& _progress, bool _verbose);

        virtual void addBranches() = 0;
        void appendNewStates();
        void checkProgress();
        void tickAllStates();
        void purgeObsoleteStates();

        vector<GameState> run();
};

class PurchaseIteration : public Iteration {
    public:
        PurchaseIteration(
            GameState st, const Progress& progress,
            const vector<vector<double>>& sacrificeStrategies, bool verbose
        );
        void addBranches() override;
};

class ApproxSacrificeIteration : public Iteration {
    private:
        inline static const Decimal precision = DC::D1 + Decimal(1e-3);

    public:
        ApproxSacrificeIteration(
            GameState st, const Progress& progress,
            const vector<vector<double>>& purchaseStrategies, bool verbose
        );
        void addBranches() override;
};

class PreciseSacrificeIteration : public Iteration {
    private:
        inline static const Decimal tolerance = DC::D1_01;
        inline static const Decimal precision = DC::D1 + Decimal(1e-9);
        vector<double> projectedSacrifices;

    public:
        PreciseSacrificeIteration(
            GameState st, const Progress& progress,
            const vector<vector<double>>& purchaseStrategies, 
            const vector<vector<double>>& sacrificeStrategies, bool verbose
        );
        void addBranches() override;
};

GameState runDimboost(GameState st, bool useSacrifice, bool verbose=true);
GameState runUntil(GameState st, const Progress& progress, bool useSacrifice, bool verbose);

void computePurchaseBranches(GameState& gameState, vector<GameState>& branches);

bool compare(const double* st1, const double* st2, size_t width);
void purge(vector<GameState>& gamestates, bool verbose);
