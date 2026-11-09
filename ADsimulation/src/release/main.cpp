#include "gameinstance.h"
#include "simulation.h"
#include "cmdutils.h"
#include "ioutils.h"
#include <algorithm>
#include <cstdlib>
#include <omp.h>
#include <map>
using namespace std;


int main(int argc, char* argv[]) {
    if (!std::getenv("OMP_NUM_THREADS")) {
        omp_set_num_threads(std::max(1, omp_get_num_procs() / 2));
    }
    enableAnsiEscapes();

    InputData data = getInputFromCmd(argc, argv);
    if (!data.isValid) return 0;

    switch (data.runType) {
        case NextDimboost: {
            GameInstance instance = runDimboost(data.gameState, data.useSacrifice);
            data.saveResult(instance);
            break;
        }
        case NextGalaxy: {
            map<int32_t, GameInstance> routes = runFullGalaxy(data.gameState, data.useSacrifice);
            data.saveResult(routes);
            break;
        }
        case ForceNextGalaxy: {
            GameInstance instance = runGalaxy(data.gameState, data.useSacrifice);
            data.saveResult(instance);
            break;
        }
    }
    return 0;
};
