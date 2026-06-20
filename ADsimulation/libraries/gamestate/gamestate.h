#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"


struct GameState {
    Decimal antimatter;
    AntimatterDimensions antimatterDimensions;
    Tickspeed tickspeed;

    GameState();

    void tick();
};