#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"


struct GameState {
    Decimal antimatter;
    AntimatterDimensions AD;
    Tickspeed tickspeed;

    GameState();

    void tick();

    vector<Dimension> getVectorAD();
};