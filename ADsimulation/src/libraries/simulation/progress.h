#pragma once
#include "gamestate.h"
#include <functional>
#include <algorithm>


class Progress {
    private:
        std::function<bool(const GameState&)> stopCondition;
        std::function<double(const GameState&)> progressFunction;
        double progressFraction = 0.0;

    public:
        Progress(
            std::function<bool(const GameState&)> stopFunc, 
            std::function<double(const GameState&)> progressFunc
        )   :   stopCondition(stopFunc),
                progressFunction(progressFunc) 
            {}

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

        static Progress untilInfinity() {
            return Progress(
                [](const GameState& st) {return st.canCrunch();}, 
                [](const GameState& st) {
                    return st.antimatterThisReset().log10value() / DC::INF.log10value();
                }
            );
        }

        double getFraction() const {return progressFraction;}
        void finish() {
            progressFraction = 1.0;
        }
};
