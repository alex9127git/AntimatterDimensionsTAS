#pragma once
#include "../gamestate/gamestate.h"
#include <iostream>
#include <algorithm>
#include <functional>
#include <chrono>
#include <vector>


class Timer {
    private:
        std::chrono::time_point<std::chrono::steady_clock> start;

    public:
        Timer() : start(std::chrono::steady_clock::now()) {}

        void reset() {
            double result = peek();
            start = std::chrono::steady_clock::now();
            std::cout << result << " ms" << std::endl;
        }

        double silentReset() {
            double result = peek();
            start = std::chrono::steady_clock::now();
            return result;
        }

        double peek() const {
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            return elapsed.count();
        }
};

class Progress {
    private:
        std::function<bool(const GameState&)> stopCondition;
        std::function<double(const GameState&)> progressFunction;
        double progressFraction = 0.0;

    public:
        Progress(std::function<bool(const GameState&)> stopFunc, std::function<double(const GameState&)> progressFunc);
        void update(const GameState& st) {
            progressFraction = std::clamp(progressFunction(st), 0.0, 1.0);
        }
        bool isFinished(const GameState& st) const {
            return stopCondition(st);
        }

        static Progress untilDimboost() {
            return Progress(
                [](const GameState& st) {return st.canBuyNextDimboost();}, 
                [](const GameState& st) {
                    return st.antimatterThisReset().log10value() / st.getAntimatterGoalForDimboost().log10value();
                }
            );
        }

        double getFraction() const {return progressFraction;}
        void finish() {
            progressFraction = 1.0;
        }
};

class Stats {
    private:
        Timer timer;
        Timer branchTimer;
        Timer checkTimer;
        Timer tickTimer;
        Timer purgeTimer;
        double branchTime = 0;
        double checkTime = 0;
        double tickTime = 0;
        double purgeTime = 0;

    public:
        void resetGlobalTimer() {timer.reset();}
        void resetBranchTimer() {branchTimer.silentReset();}
        void resetCheckTimer() {checkTimer.silentReset();}
        void resetTickTimer() {tickTimer.silentReset();}
        void resetPurgeTimer() {purgeTimer.silentReset();}
        void updateBranchTime() {branchTime += branchTimer.silentReset();}
        void updateCheckTime() {checkTime += checkTimer.silentReset();}
        void updateTickTime() {tickTime += tickTimer.silentReset();}
        void updatePurgeTime() {purgeTime += purgeTimer.silentReset();}
        double getTimeRunning() {return timer.peek();}
        void printStats(int maxStates) {
            std::cout << std::endl;
            std::cout << "Finished! Total time elapsed: ";
            timer.reset();
            std::cout << "Max States: " << maxStates << std::endl;
            std::cout << "branch: " << branchTime << " ms" << std::endl;
            std::cout << "check: " << checkTime << " ms" << std::endl;
            std::cout << "tick: " << tickTime << " ms" << std::endl;
            std::cout << "purge: " << purgeTime << " ms" << std::endl;
        }
};

class Iteration {
    protected:
        Progress progress;
        bool verbose;
        
        std::vector<GameState> gameStates;
        std::vector<GameState> newGameStates;
        Stats stats;
        bool isFinished = false;
        int ticks = 0;
        int statesBeforePurge = 1;
        int statesAfterPurge = 1;
        int adds = 0;
        int maxStates = 0;
        int states = 0;

        Timer updateTimer;
        int linesRendered = 0;
        int statesPurged = 0;
        double purgeProgress = 0.0;
        static constexpr double RENDER_INTERVAL_MS = 100.0;

        virtual void addBranches() = 0;
        void appendNewStates();
        void checkProgress();
        void tickAllStates();
        void purgeObsoleteStates();

        void renderProgressDisplay(bool purgeOngoing=false);

    public:
        Iteration(const Progress& _progress, bool _verbose);

        std::vector<GameState> run();
};

class PurchaseIteration : public Iteration {
    private:
        void addBranches() override;

    public:
        PurchaseIteration(
            GameState st, const Progress& progress,
            const std::vector<std::vector<double>>& sacrificeStrategies, bool verbose
        );
};

class ApproxSacrificeIteration : public Iteration {
    private:
        inline static const Decimal precision = DC::D1 + Decimal(1e-3);

        void addBranches() override;

    public:
        ApproxSacrificeIteration(
            GameState st, const Progress& progress,
            const std::vector<std::vector<double>>& purchaseStrategies, bool verbose
        );
};

class PreciseSacrificeIteration : public Iteration {
    private:
        inline static const Decimal tolerance = DC::D1_01;
        inline static const Decimal precision = DC::D1 + Decimal(1e-9);
        std::vector<double> projectedSacrifices;

        void addBranches() override;
        void scheduleSacrifice(GameState& st, const Decimal& target) const;

    public:
        PreciseSacrificeIteration(
            GameState st, const Progress& progress,
            const std::vector<std::vector<double>>& purchaseStrategies, 
            const std::vector<std::vector<double>>& sacrificeStrategies, bool verbose
        );
};

GameState runDimboost(GameState st, bool useSacrifice, bool verbose=true);
GameState runUntil(GameState st, const Progress& progress, bool useSacrifice, bool verbose);

void computePurchaseBranches(GameState& gameState, std::vector<GameState>& branches);

bool compare(const double* st1, const double* st2, size_t width);
void purge(std::vector<GameState>& gamestates, const std::function<void(int, double)>& onProgress={});
