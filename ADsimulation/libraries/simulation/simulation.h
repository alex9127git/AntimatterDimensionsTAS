#include "../gamestate/gamestate.h"
#include "chrono"


class Timer {
    private:
        chrono::time_point<chrono::steady_clock> start;

    public:
        Timer() : start(chrono::steady_clock::now()) {}

        void reset() {
            auto end = chrono::steady_clock::now();
            chrono::duration<double, std::milli> elapsed = end - start;
            start = end;
            std::cout << "Elapsed: " << elapsed.count() << " ms" << endl;
        }
};


//GameState run(GameState st, function<bool(GameState&)> stopCondition);
GameState run(GameState& st, function<bool(GameState&)> stopCondition);
