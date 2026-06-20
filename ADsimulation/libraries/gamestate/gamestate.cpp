#include "gamestate.h"
#include "../constants/constants.h"


GameState::GameState()
    :   antimatter(DC::D10),
        antimatterDimensions(AntimatterDimensions()),
        tickspeed(Tickspeed())
    {};

void GameState::tick() {
    // apply resources
    for (int i = (int) antimatterDimensions.dims.size() - 1; i >= 0; i--) {
        if (i == 0) {
                // note to jade: + operator is overwritten short hand for .add() Decimal method.
                // it accepts the `this` which is the value to the left, 
                // and the value on the right.
            antimatter = antimatter + antimatterDimensions.dims[0].production;
        } else if (antimatterDimensions.dims[i].unlocked == true) {
            antimatterDimensions.dims[i - 1].amountAuto = antimatterDimensions.dims[i - 1].amountAuto + antimatterDimensions.dims[i].production;
        };
    };
};