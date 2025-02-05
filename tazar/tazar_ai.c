#include "steve.h"
#include "tazar.h"

#include <stdlib.h>
#include <stdio.h>

typedef Array(Game) GameArray;
typedef Array(double) DoubleArray;

int rand_in_range(int min, int max) {
    // todo: better random number generation
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

Command ai_select_command_random(Game *game, CommandSlice commands) {
    int selected = rand_in_range(0, (int) (commands.len - 1));
    return commands.e[selected];
}

uint64_t ai_simulate_game(Game *game, int depth) {
    Arena *scratch = scratch_acquire();

    while (game->status == STATUS_IN_PROGRESS && depth > 0) {
        CommandSlice commands = game_valid_commands(scratch, game);
        Command command = ai_select_command_random(game, commands);
        game_apply_command(game, game->turn.player, command);
        depth--;
    }

    scratch_release();
}

double ai_rollout(Game *game, Command command, int rollouts, int depth) {
    Arena *scratch = scratch_acquire();

    // Gold you would have if you killed every piece.
    int max_gold = 1 * 6 + 4 * 1 + 2 * 3 + 3 * 2;

    Player starting_player = game->turn.player;
    Player starting_opponent = starting_player == PLAYER_RED ? PLAYER_BLUE : PLAYER_RED;
    int starting_player_gold = game->gold[starting_player];
    int starting_opponent_gold = game->gold[starting_opponent];

    // Apply the first command.
    game_apply_command(game, game->turn.player, command);

    double avg_score = 0.5;
    for (int i = 0; i < rollouts; i++) {
        Game sim_game = *game;
        ai_simulate_game(&sim_game, depth);

        double score = 0.5;
        int gold_gained = sim_game.gold[starting_player] - starting_player_gold;
        int opponent_gold_gained = sim_game.gold[starting_opponent] - starting_opponent_gold;
        int gold_diff = gold_gained - opponent_gold_gained + max_gold;
        score = (double) gold_diff / (2 * max_gold);

        if (sim_game.status == STATUS_OVER) {
            if (sim_game.winner == starting_player) {
                score = 1;
            } else {
                score = 0;
            }
        }
        avg_score += score;
    }

    scratch_release();
    return avg_score / (double) rollouts;
}

Command ai_select_command_mcts(Game *game, CommandSlice commands) {
    Arena *scratch = scratch_acquire();

    DoubleArray *scores = NULL;
    arr_setlen(scratch, scores, commands.len);

    for (uint64_t i = 0; i < commands.len; i++) {
        Game rollout_game = *game;
        double score = ai_rollout(&rollout_game, commands.e[i], 300, 20);
        scores->e[i] = score;
    }

    double max_score = 0;
    uint64_t max_score_i = 0;
    for (uint64_t i = 0; i < commands.len; i++) {
        if (scores->e[i] > max_score) {
            max_score = scores->e[i];
            max_score_i = i;
        }
    }

    Command result = commands.e[max_score_i];
    scratch_release();
    return result;
}
