#include "steve.h"
#include "tazar.h"

#include <math.h>
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


double ai_rollout(Game game, Command command, int depth) {
    Arena *scratch = scratch_acquire();

    Player scoring_player = game.turn.player;
    Player scoring_opponent = scoring_player == PLAYER_RED ? PLAYER_BLUE : PLAYER_RED;
    int scoring_player_gold = game.gold[scoring_player];
    int scoring_opponent_gold = game.gold[scoring_opponent];

    game_apply_command(&game, game.turn.player, command, VOLLEY_ROLL);

    while (game.status == STATUS_IN_PROGRESS && depth > 0) {
        CommandSlice commands = game_valid_commands(scratch, &game);
        Command picked_command = ai_select_command_random(&game, commands);
        game_apply_command(&game, game.turn.player, picked_command, VOLLEY_ROLL);
        depth--;
    }

    // Gold you would have if you killed every piece.
    int max_gold = 1 * 6 + 4 * 1 + 2 * 3 + 3 * 2;

    double score = 0.5;
    int gold_gained = game.gold[scoring_player] - scoring_player_gold;
    int opponent_gold_gained = game.gold[scoring_opponent] - scoring_opponent_gold;
    int gold_diff = gold_gained - opponent_gold_gained + max_gold;
    score = (double) gold_diff / (2 * max_gold);

    if (game.status == STATUS_OVER) {
        if (game.winner == scoring_player) {
            score = 1;
        } else {
            score = 0;
        }
    }

    scratch_release();
    return score;
}

Command ai_select_command_random_rollouts(Game *game, CommandSlice commands) {
    Arena *scratch = scratch_acquire();

    DoubleArray *scores = NULL;
    arr_setlen(scratch, scores, commands.len);
    Player scoring_player = game->turn.player;

    for (uint64_t i = 0; i < commands.len; i++) {
        double total_score = 0.5;
        for (int rollout = 0; rollout < 100; rollout++) {
            Game rollout_game = *game;
            double score = ai_rollout(rollout_game, commands.e[i], 300);
            total_score += score;
        }
        double avg_score = total_score / 100;

        scores->e[i] = avg_score;
    }

    double max_score = 0;
    uint64_t max_score_i = 0;
    for (uint64_t i = 0; i < commands.len; i++) {
        double s = scores->e[i];
        if (s > max_score) {
            max_score = scores->e[i];
            max_score_i = i;
        }
    }

    Command result = commands.e[max_score_i];
    scratch_release();
    return result;
}

typedef enum {
    NODE_NONE,
    NODE_DECISION,
    NODE_CHANCE,
    NODE_OVER,
} NodeKind;

typedef struct {
    Game game;
    Command command;
} NodeInfo;

typedef struct {
    NodeKind kind;
    Player player;
    uint32_t parent_i;
    uint32_t first_child_i;
    uint32_t num_children;
    uint32_t visits;
    double total_reward;
    double probability;
} Node;

typedef Array(NodeInfo) NodeInfoArray;
typedef Array(Node) NodeArray;

typedef struct {
    uint32_t root;
    NodeArray *nodes;
    NodeInfoArray *nodes_info;
} MCTSState;

Node zero_node = (Node) {
        .kind = NODE_NONE,
        .player = PLAYER_NONE,
        .parent_i = 0,
        .first_child_i = 0,
        .num_children = 0,
        .visits = 0,
        .total_reward = 0,
        .probability = 0,
};
NodeInfo zero_node_info = (NodeInfo) {
        .game = (Game) {0},
        .command = (Command) {0},
};

Command ai_select_command_mcts(Arena *arena, void **ai_state, Game *game, CommandSlice commands) {
    MCTSState *mcts;
    uint32_t new_root = 0;
    if (*ai_state != NULL) {
        // Try and find the new root in the old tree.
        mcts = (MCTSState *) *ai_state;

        for (uint32_t i = 0; i < mcts->nodes->len; i++) {
            if (mcts->nodes->e[i].kind == NODE_DECISION && game_eq(&mcts->nodes_info->e[i].game, game)) {
                new_root = i;
                break;
            }
        }
    }
    if (new_root == 0) {
        printf("reinitializing mcts\n");
        arena_reset(arena);
        mcts = arena_alloc(arena, MCTSState);
        // Initialize index 0 as a zero node.
        // Makes it so nodes and nodes_info are not NULL so we don't have to check for that.
        arr_push(arena, mcts->nodes, zero_node);
        arr_push(arena, mcts->nodes_info, zero_node_info);

        // Push game and set it as the root node.
        arr_push(arena, mcts->nodes, ((Node) {
                .kind = NODE_DECISION,
                .player = game->turn.player,
                .parent_i = 0,
                .first_child_i = 0,
                .num_children = 0,
                .visits = 0,
                .total_reward = 0,
        }));
        arr_push(arena, mcts->nodes_info, ((NodeInfo) {
                .game = *game,
                .command = (Command) {0},
        }));
        mcts->root = 1;
        *ai_state = mcts;
    } else {
        printf("reusing mcts\n");
        mcts->root = new_root;
    }

    double c = sqrt(2);
    for (int pass = 0; pass < 10000; pass++) {
        // Selection.
        uint32_t node_i = mcts->root;

        bool found = false;
        while (!found) {
            uint32_t highest_uct_i = 0;
            if (mcts->nodes->e[node_i].kind == NODE_OVER || mcts->nodes->e[node_i].visits == 0) {
                break;
            } else if (mcts->nodes->e[node_i].kind == NODE_DECISION) {
                // Select node from children.
                double highest_uct = -INFINITY;
                for (uint32_t child_i = mcts->nodes->e[node_i].first_child_i;
                     child_i < mcts->nodes->e[node_i].first_child_i + mcts->nodes->e[node_i].num_children; child_i++) {
                    if (mcts->nodes->e[child_i].visits == 0) {
                        // Unexpanded child.
                        node_i = child_i;
                        found = true;
                        break;
                    } else {
                        double child_uct =
                                (mcts->nodes->e[child_i].total_reward / mcts->nodes->e[child_i].visits) +
                                c * sqrt(log(mcts->nodes->e[node_i].visits) / mcts->nodes->e[child_i].visits);
                        if (child_uct > highest_uct) {
                            highest_uct = child_uct;
                            highest_uct_i = child_i;
                        }
                    }
                }
            } else if (mcts->nodes->e[node_i].kind == NODE_CHANCE) {
                // Select node from children.
                double r = (double) rand() / RAND_MAX;  // random value in [0,1)
                double cumulative = 0.0;
                for (uint32_t child_i = mcts->nodes->e[node_i].first_child_i;
                     child_i < mcts->nodes->e[node_i].first_child_i + mcts->nodes->e[node_i].num_children; child_i++) {
                    if (mcts->nodes->e[child_i].visits == 0) {
                        // Unexpanded child.
                        node_i = child_i;
                        found = true;
                        break;
                    } else {
                        cumulative += mcts->nodes->e[child_i].probability;
                        if (r < cumulative) {
                            node_i = child_i;
                            break;
                        }
                    }
                }
            } else {
                assert(false);
            }

            if (!found) {
                // Select the child with the highest uct.
                node_i = highest_uct_i;
            }
        }

        // Expansion.
        if (mcts->nodes->e[node_i].kind == NODE_DECISION) {
            // Create child nodes.
            CommandSlice commands = game_valid_commands(arena, &mcts->nodes_info->e[node_i].game);
            mcts->nodes->e[node_i].first_child_i = mcts->nodes->len;
            if (mcts->nodes->e[node_i].num_children != 0) {
                printf("why?");
            }
            for (int i = 0; i < commands.len; i++) {
                Game child_game = mcts->nodes_info->e[node_i].game;
                NodeKind kind;
                if (commands.e[i].kind != COMMAND_VOLLEY) {
                    game_apply_command(&child_game, child_game.turn.player, commands.e[i], VOLLEY_ROLL);
                    kind = child_game.status == STATUS_IN_PROGRESS ? NODE_DECISION : NODE_OVER;
                } else {
                    kind = NODE_CHANCE;
                }
                arr_push(arena, mcts->nodes, ((Node) {
                        .kind = kind,
                        .player = child_game.turn.player,
                        .parent_i = node_i,
                        .first_child_i = 0,
                        .num_children = 0,
                        .visits = 0,
                        .total_reward = 0,
                        .probability = 1,
                }));
                arr_push(arena, mcts->nodes_info, ((NodeInfo) {
                        .game = child_game,
                        .command = commands.e[i],
                }));
                mcts->nodes->e[node_i].num_children++;
            }
            assert(mcts->nodes->e[node_i].num_children == commands.len);
        } else if (mcts->nodes->e[node_i].kind == NODE_CHANCE) {
            assert(mcts->nodes_info->e[node_i].command.kind == COMMAND_VOLLEY);
            mcts->nodes->e[node_i].first_child_i = mcts->nodes->len;

            Game hit_game = mcts->nodes_info->e[node_i].game;
            game_apply_command(&hit_game, hit_game.turn.player, mcts->nodes_info->e[node_i].command, VOLLEY_HIT);
            arr_push(arena, mcts->nodes, ((Node) {
                    .kind = hit_game.status == STATUS_IN_PROGRESS ? NODE_DECISION : NODE_OVER,
                    .player = hit_game.turn.player,
                    .parent_i = node_i,
                    .first_child_i = 0,
                    .num_children = 0,
                    .visits = 0,
                    .total_reward = 0,
                    .probability = 0.4167,
            }));
            arr_push(arena, mcts->nodes_info, ((NodeInfo) {
                    .game = hit_game,
                    .command = mcts->nodes_info->e[node_i].command,
            }));
            mcts->nodes->e[node_i].num_children++;

            Game miss_game = mcts->nodes_info->e[node_i].game;
            game_apply_command(&miss_game, miss_game.turn.player, mcts->nodes_info->e[node_i].command, VOLLEY_MISS);
            arr_push(arena, mcts->nodes, ((Node) {
                    .kind = miss_game.status == STATUS_IN_PROGRESS ? NODE_DECISION : NODE_OVER,
                    .player = miss_game.turn.player,
                    .parent_i = node_i,
                    .first_child_i = 0,
                    .num_children = 0,
                    .visits = 0,
                    .total_reward = 0,
                    .probability = 1.0 - 0.4167,
            }));
            arr_push(arena, mcts->nodes_info, ((NodeInfo) {
                    .game = miss_game,
                    .command = mcts->nodes_info->e[node_i].command,
            }));
            mcts->nodes->e[node_i].num_children++;
        }

        // simulate node.
        double score = ai_rollout(mcts->nodes_info->e[node_i].game, (Command) {0}, 300);

        // backprop
        Player apply_player = mcts->nodes->e[node_i].player;
        uint32_t apply_i = node_i;
        while (apply_i != 0) {
            Node *apply_node = mcts->nodes->e + apply_i;
            apply_node->visits++;
            if (apply_player == apply_node->player) {
                apply_node->total_reward += score;
            } else {
                apply_node->total_reward -= score;
            }
            apply_i = apply_node->parent_i;
        }
    }

    uint32_t most_visits = 0;
    uint32_t best_child_i = 0;
    for (uint32_t child_i = mcts->nodes->e[mcts->root].first_child_i;
         child_i < mcts->nodes->e[mcts->root].first_child_i + mcts->nodes->e[mcts->root].num_children; child_i++) {
        if (mcts->nodes->e[child_i].visits >= most_visits) {
            most_visits = mcts->nodes->e[child_i].visits;
            best_child_i = child_i;
        }
    }
    return mcts->nodes_info->e[best_child_i].command;
}

// ok, big changes to make next.
// model volleys with chance nodes so it can explore what happens in both options.
// To do that I need to add in an a way to explicitly say if a volley hits or not when applying a command.






// Tree nodes.
// Wanna optimize the size of these as much as I can so we can have a lot per cache line.
// Traversing them happens most during selection and backprop, so stuff we need for only for expansion shouldn't be in here.
// Traversing down you do look at every child, so makes sense to have those allocated together.

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