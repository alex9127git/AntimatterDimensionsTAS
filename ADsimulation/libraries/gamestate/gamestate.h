#include "../dimensions/dimensions.h"
#include "../tickspeed/tickspeed.h"


class GameState {
    private:
        Decimal _antimatter;
        AntimatterDimensions _AD;
        Tickspeed tickspeed;

    public:
        GameState();

        Decimal antimatter();

        AntimatterDimensions AD();
        vector<AntimatterDimension> getVectorAD();

        void tick(double diff);

        bool buyOneDimension(int dim);
        bool buyTickspeed();
};