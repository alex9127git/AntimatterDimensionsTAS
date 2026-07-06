#include "../gamestate/gamestate.h"


GameState& run(GameState& st, function<bool(GameState&)> stopCondition);
