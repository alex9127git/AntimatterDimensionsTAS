#include "gamestate.h"
#include "../constants/constants.h"


GameState::GameState()
    :   antimatter(DC::D10),
        AD(AntimatterDimensions()),
        tickspeed(Tickspeed())
    {};

void GameState::tick() {
    // apply resources
    for (int i = (int) getVectorAD().size() - 1; i >= 0; i--) {
        if (i == 0) {
                // note to jade: + operator is overwritten short hand for .add() Decimal method.
                // it accepts the `this` which is the value to the left, 
                // and the value on the right.
            antimatter = antimatter + AD[0].production;
        } else if (getVectorAD()[i].unlocked == true) {
            AD[i - 1].amountAuto = AD[i - 1].amountAuto + AD[i].production;
        };
    };
};

vector<Dimension> GameState::getVectorAD() {
    return AD.dims;
}