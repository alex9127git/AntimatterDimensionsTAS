#include "gameinstance.h"
#include "simulation.h"
#include "cmdutils.h"
#include "ioutils.h"
#include <algorithm>
#include <omp.h>
using namespace std;


int main(int argc, char* argv[]) {
    if (!std::getenv("OMP_NUM_THREADS")) {
        omp_set_num_threads(std::max(1, omp_get_num_procs() / 2));
    }
    enableAnsiEscapes();

    InputData data = getInputFromCmd(argc, argv);
    if (!data.isValid) return 0;

    GameInstance instance;
    switch (data.runType) {
        case NextDimboost:
            instance = runDimboost(data.gameState, data.useSacrifice);
            break;
        case NextGalaxy:
            cout << "not implemented yet" << endl;
            return 0;
        case ForceNextGalaxy:
            instance = runGalaxy(data.gameState, data.useSacrifice);
            break;
    }
    data.saveResult(instance);
    return 0;
};
