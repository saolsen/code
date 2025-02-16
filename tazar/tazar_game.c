#include "tazar.h"

#include <assert.h>

// Set up a new game.

void game_init(Game *game, GameMode game_mode, Map map) {
    game->game_mode = game_mode;
    game->map = map;
    game->status = STATUS_IN_PROGRESS;
    game->winner = PLAYER_NONE;

    for (uint32_t i = 0; i < 3; i++) {
        game->gold[i] = 0;
        switch (game_mode) {
            case GAME_MODE_TOURNAMENT: {
                game->gold[i] = 10;
                break;
            }
            case GAME_MODE_ATTRITION: {
                for (uint32_t j = 0; j < 5; j++) {
                    game->reserves[i][j] = 0;
                }
                break;
            }
            default: {
                assert(0);
            }
        }
    }
}