#pragma once
#include "searchnode.h"
#include "simconst.h"
#include "progress.h"
#include "stats.h"
#include "timer.h"
#include <vector>
#include <map>
using Strategies = std::vector<std::vector<double>>;


struct SacrificeWindow {
    Decimal boundLow;
    Decimal boundHigh;
    Decimal precision;

    static SacrificeWindow refinementWindow() {
        return {DC::D1_01, DC::D1_01, REFINEMENT_PRECISION};
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
        void purgeObsoleteStates(bool force=false);

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

class SacrificeRefinementIteration : public Iteration {
    private:
        int purgeTicks = 0;
        void addBranches() override;

    public:
        SacrificeRefinementIteration(
            GameState st, const Progress& progress,
            const Strategies& purchaseStrategies, 
            const Strategies& sacrificeStrategies, bool verbose
        );
};


namespace SimulationHelpers {
    struct SegmentSearch;
}


GameInstance runDimboost(GameState st, bool useSacrifice, bool verbose=true);
GameInstance runGalaxy(GameState st, bool useSacrifice, bool verbose=true,
    std::function<bool(const SimulationHelpers::SegmentSearch&)> earlyStopCondition=
        [](const SimulationHelpers::SegmentSearch&) {return false;});
GameInstance runUntil(GameState st, const Progress& progress, bool useSacrifice, bool verbose,
    std::function<bool(const SimulationHelpers::SegmentSearch&)> earlyStopCondition=
        [](const SimulationHelpers::SegmentSearch&) {return false;});

std::map<int32_t, GameInstance> runFullGalaxy(GameState st, bool useSacrifice, bool verbose=true);
