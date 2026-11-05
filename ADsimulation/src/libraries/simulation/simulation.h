#pragma once
#include "searchnode.h"
#include "timer.h"
#include <functional>
#include <vector>
using Strategies = std::vector<std::vector<double>>;


struct SacrificeWindow {
    Decimal boundLow;
    Decimal boundHigh;
    Decimal precision;

    static SacrificeWindow refinementWindow() {
        return {DC::D1_01, DC::D1_01, DC::D1 + Decimal(1e-9)};
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
        
        static Progress untilAch23() {
            return Progress(
                [](const GameState& st) {return st.achievements().isUnlocked(23);}, 
                [](const GameState& st) {
                    return st.antimatterThisReset().log10value() / st.getAntimatterGoalForDimboost().log10value();
                }
            );
        }

        static Progress untilGalaxy() {
            return Progress(
                [](const GameState& st) {return st.canBuyNextGalaxy();}, 
                [](const GameState& st) {
                    return st.antimatterThisReset().log10value() / st.getAntimatterGoalForGalaxy().log10value();
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
        // input args
        Progress progress;
        bool verbose;
        
        // nodes and strategies
        std::vector<SearchNode> nodes;
        std::vector<SearchNode> newNodes;
        Strategies projectedSacrifices;

        // stats
        Stats stats;
        bool isFinished = false;
        int64_t ticks = 0;
        int statesBeforePurge = 1;
        int statesAfterPurge = 1;
        int adds = 0;
        int maxStates = 0;
        int states = 0;

        // progress rendering
        Timer updateTimer;
        int linesRendered = 0;
        int statesPurged = 0;
        double purgeProgress = 0.0;
        static constexpr double RENDER_INTERVAL_MS = 100.0;
        void renderProgressDisplay(bool purgeOngoing=false);

        // steps
        virtual void addBranches() = 0;
        void appendNewStates();
        void checkProgress();
        void tickAllStates();
        void purgeObsoleteStates();

        // sacrifice window behavior
        virtual SacrificeWindow sacrificeWindow(size_t, size_t) const {
            return SacrificeWindow::refinementWindow();
        }
        virtual bool hasSacrificeWindow(size_t, size_t) const {return true;}
        void prepareSacrifices(SearchNode& node, size_t routeIndex, size_t firstSacrifice) const;
        void addSacrificeWindowBranches();
        void scheduleSacrifice(SearchNode& st, const Decimal& target, double diff) const;

    public:
        Iteration(
            const Progress& _progress, 
            bool _verbose,
            const Strategies& _projectedSacrifices = {}
        );

        std::vector<SearchNode> run();
};

class PurchaseIteration : public Iteration {
    private:
        inline static constexpr double WINDOW_TICKS = 450;
        inline static constexpr double STEP_TICKS = 15;
        inline static const Decimal maxPrecision = DC::D1_01;
        Strategies sacrificeDiffs;
        int windowIndex;

        bool hasSacrificeWindow(size_t routeIndex, size_t sacrificeIndex) const override;
        SacrificeWindow sacrificeWindow(size_t routeIndex, size_t sacrificeIndex) const override;

        void addBranches() override;
        void computePurchaseBranches(SearchNode& node, std::vector<SearchNode>& branches);

    public:
        PurchaseIteration(
            GameState st, const Progress& progress,
            const Strategies& sacrificeStrategies, bool verbose,
            const Strategies& _sacrificeDiffs = {}, int _windowIndex = -1
        );
};

class ApproxSacrificeIteration : public Iteration {
    private:
        inline static const Decimal precision = DC::D1 + Decimal(1e-3);

        void addBranches() override;

    public:
        ApproxSacrificeIteration(
            GameState st, const Progress& progress,
            const Strategies& purchaseStrategies, bool verbose
        );
};

class PreciseSacrificeIteration : public Iteration {
    private:
        void addBranches() override;

    public:
        PreciseSacrificeIteration(
            GameState st, const Progress& progress,
            const Strategies& purchaseStrategies, 
            const Strategies& sacrificeStrategies, bool verbose
        );
};

void collectStrategyPairs(const std::vector<SearchNode>& nodes, Strategies& purchaseStrategies,
                          Strategies& sacrificeStrategies, bool addFinalTarget);
void collectSacrificeStrategies(const std::vector<SearchNode>& nodes, Strategies& sacrificeStrategies,
                                Strategies& sacrificeDiffs);

GameInstance runDimboost(GameState st, bool useSacrifice, bool verbose=true);
GameInstance runGalaxy(GameState st, bool useSacrifice, bool verbose=true);
GameInstance runUntil(GameState st, const Progress& progress, bool useSacrifice, bool verbose);
