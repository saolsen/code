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

Command ai_select_command_random_rollouts(Game *game, CommandSlice commands) {
    Arena *scratch = scratch_acquire();

    DoubleArray *scores = NULL;
    arr_setlen(scratch, scores, commands.len);

    // Bigger depth makes the ai a lot smarter. Going to use lots of rollouts with a high depth for training in RL.
    for (uint64_t i = 0; i < commands.len; i++) {
        Game rollout_game = *game;
        double score = ai_rollout(&rollout_game, commands.e[i], 100, 20);
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

// Next thing to try is MCTS. Where you keep a tree of game states and search through it with simulations that expand it
// and then backpropagate the results.
// This way the search is targeted towards promising moves instead of uniform over all moves.

typedef struct MCTSNode {
    Game state;
    Command move;         // Move that led to this state (or an "empty" move at root)
    int visits;
    double totalReward;
    struct MCTSNode **children;
    int numChildren;
    int numUntriedMoves;
    CommandSlice untriedMoves;
    struct MCTSNode *parent;
} MCTSNode;

typedef enum {
    NODE_MOVE,
    NODE_ROLL, // volley rolls.
} NodeKind;

// Tree nodes.
typedef struct {
    Game game;
    NodeKind kind;
    Command command;
    int visits;
    double total_reward;
    // pointers to child nodes.
    CommandSlice untried_moves;
    // pointer to parent. Where we came from via a move or a roll.
} Node;


// so, the next step is reinforcement learning.
// For this, I think I need a fixed command space.
// That is, an array of all possible commands.
// Instead of refering to the piece by id, I should refer to it by CPos.
// for each space
// * You can move to one of the spaces in range
// * You can volley one of the spaces in range
// * You can end your turn.
// Obvously for any game state, most of the commands in this list are illegal. But the fixed command space is important for the RL agent.

// there are 81 spaces in the board array because I store it in a 9x9 grid.
// (there are in reality only 61 tiles, not 81)
// the horse can move 4 in each direction, which means if he's in the center he can move to any other space.
// so that's 60 possible moves
// there are 18 volley targets per space.
// there is one end_turn action.

// so that's (60 + 18) * 61 + 1; 4759 possible commands.
// if we use the full array it's 6319 commands.
// 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576