#include "steve.h"
#include "tazar.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

typedef Array(Game) GameArray;
typedef Slice(double) DoubleSlice;
typedef Array(double) DoubleArray;

int rand_in_range(int min, int max) {
    // todo: better random number generation
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

// Heuristic versions of a policy and value which guide the non RL versions of the AI.
// Returns a value in range of -46 to 46.
double heuristic_value(Game *game, Player player) {
    Player opponent = (player == PLAYER_RED ? PLAYER_BLUE : PLAYER_RED);
    int player_gold = game->gold[player];
    int player_value = 0;
    int opponent_gold = game->gold[opponent];
    int opponent_value = 0;

    int max_gold = 1 * 6 + 5 * 1 + 2 * 3 + 3 * 2;

    // Iterate over all positions on the board.
    for (int r = -4; r <= 4; r++) {
        for (int q = -4; q <= 4; q++) {
            CPos cpos = {q, r, -q - r};
            Piece *p = board_at((Game *) game, cpos);
            if (p->kind == PIECE_NONE) { continue; }
            if (p->player == player) {
                player_value += piece_gold(p->kind);
            } else if (p->player == opponent) {
                opponent_value += piece_gold(p->kind);
            }
        }
    }

    // Full value for winning dispite number of pieces left.
    if (game->status == STATUS_OVER) {
        if (game->winner == player) {
            player_gold = max_gold;
            player_value = max_gold;
            opponent_gold = 0;
            opponent_value = 0;
        } else {
            opponent_gold = max_gold;
            opponent_value = max_gold;
            player_gold = 0;
            player_value = 0;
        }
    }

    double eval = (player_gold + player_value) - (opponent_gold + opponent_value);
    return eval;
}

// probability distribution over commands that could be chosen.
DoubleSlice heuristic_policy(Arena *arena, Game *game, CommandSlice commands) {
    double temperature = 2;
    double current_value = heuristic_value(game, game->turn.player);

    double total_weight = 0.0;
    DoubleArray *weights = NULL;


    for (uint64_t i = 0; i < commands.len; i++) {
        Game new_game = *game;
        game_apply_command(&new_game, new_game.turn.player, commands.e[i], VOLLEY_HIT);
        double new_value = heuristic_value(&new_game, game->turn.player);
        double delta = new_value - current_value;
        double weight = exp(delta / temperature);
        arr_push(arena, weights, weight);
        total_weight += weight;
    }
    if (weights != NULL) {
        for (uint64_t i = 0; i < weights->len; i++) {
            weights->e[i] /= total_weight;
        }
        return (DoubleSlice) arr_slice(weights);
    } else {
        return (DoubleSlice) {0, 0};
    }
}

Command ai_select_command_heuristic(Game *game, CommandSlice commands) {
    Arena *scratch = scratch_acquire();
    DoubleSlice policy = heuristic_policy(scratch, game, commands);

    double r = ((double) rand() / RAND_MAX);
    Command picked_command = commands.e[commands.len - 1];
    for (uint64_t i = 0; i < commands.len; i++) {
        r -= policy.e[i];
        if (r <= 0) {
            picked_command = commands.e[i];
            break;
        }
    }
    return picked_command;
}

double ai_rollout(Game game, Command command, int depth) {
    Arena *scratch = scratch_acquire();

    Player scoring_player = game.turn.player;
    game_apply_command(&game, game.turn.player, command, VOLLEY_ROLL);

    while (game.status == STATUS_IN_PROGRESS && depth > 0) {
        CommandSlice commands = game_valid_commands(scratch, &game);
        Command next_command = ai_select_command_heuristic(&game, commands);
        game_apply_command(&game, game.turn.player, next_command, VOLLEY_ROLL);
        depth--;
    }

    double score = heuristic_value(&game, scoring_player);
    scratch_release();
    return score;
}

Command ai_select_command_uniform_rollouts(Game *game, CommandSlice commands) {
    Arena *scratch = scratch_acquire();

    DoubleArray *scores = NULL;
    arr_setlen(scratch, scores, commands.len);

    for (uint64_t i = 0; i < commands.len; i++) {
        double total_score = 0;
        for (int rollout = 0; rollout < 100; rollout++) {
            Game rollout_game = *game;
            double score = ai_rollout(rollout_game, commands.e[i], 299);
            double adjusted_score = (score + 46) / (46 * 2);

            //total_score += score;
            total_score += adjusted_score;
        }
        double avg_score = total_score / 100;

        scores->e[i] = avg_score;
    }

//    printf("scores\n");
//    for (int i = 0; i < commands.len; i++) {
//        if (commands.e[i].kind == COMMAND_VOLLEY) {
//            printf("volley\n");
//        }
//        printf("scores[%d] = %f\n", i, scores->e[i]);
//    }

    double max_score = -INFINITY;
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
    Game game;
    Command command;
    uint32_t parent_i;
    uint32_t num_unexpanded_children;
    uint32_t first_child_i;
    uint32_t next_child_i;
    uint32_t visits;
    double total_reward;
    double probability;
} Node;

typedef Array(Node) NodeArray;

typedef struct {
    Arena *arena;
    uint32_t root;
    NodeArray *nodes;
} MCTSState;

Node zero_node = (Node) {
        .kind = NODE_NONE,
        .game = (Game) {0},
        .command = (Command) {0},
        .parent_i = 0,
        .num_unexpanded_children = 0,
        .first_child_i = 0,
        .next_child_i = 0,
        .visits = 0,
        .total_reward = 0,
        .probability = 0,
};
//NodeInfo zero_node_info = (NodeInfo) {
//
//        .command = (Command) {0},
//};

// This version is simpler than the old one, but much less cache friendly because children nodes are scattered
// and nodes are much bigger.

Command ai_select_command_mcts(Arena *arena, void **ai_state, Game *game, CommandSlice commands) {
    Arena *scratch = scratch_acquire();

    // Initialize
    MCTSState *mcts;
    arena_reset(arena);
    mcts = arena_alloc(arena, MCTSState);
    arr_push(arena, mcts->nodes, zero_node);
    arr_push(arena, mcts->nodes, ((Node) {
            .kind = NODE_DECISION,
            .game = *game,
            .command = (Command) {0},
            .parent_i = 0,
            .num_unexpanded_children = (uint32_t) commands.len,
            .first_child_i = 0,
            .next_child_i = 0,
            .visits = 0,
            .total_reward = 0,
    }));
    mcts->root = 1;

    CommandArray *child_commands = NULL;
    arr_push(scratch, child_commands, (Command) {0});
    CommandArray *unexpanded_commands = NULL;
    arr_push(scratch, unexpanded_commands, (Command) {0});

    double c = sqrt(2);
    //uint32_t passes = 100 * (uint32_t) commands.len + 1;
    uint32_t passes = 500;
    for (uint32_t pass = 0; pass < passes; pass++) {
        if (pass == 9) {
            printf("break;");
        }
        // Selection
        uint32_t node_i = mcts->root;
        while (mcts->nodes->e[node_i].kind != NODE_OVER && mcts->nodes->e[node_i].num_unexpanded_children == 0) {
            // select node from children
            double highest_uct = -INFINITY;
            uint32_t highest_uct_i = 0;
            uint32_t child_i = mcts->nodes->e[node_i].first_child_i;
            while (child_i != 0) {
                double child_uct = (mcts->nodes->e[child_i].total_reward / mcts->nodes->e[child_i].visits) +
                                   c * sqrt(log(mcts->nodes->e[node_i].visits) / mcts->nodes->e[child_i].visits);
                if (child_uct > highest_uct) {
                    highest_uct = child_uct;
                    highest_uct_i = child_i;
                }
                child_i = mcts->nodes->e[child_i].next_child_i;
            }
            node_i = highest_uct_i;
        }

        // Expansion
        if (mcts->nodes->e[node_i].kind != NODE_OVER) {
            assert(mcts->nodes->e[node_i].num_unexpanded_children > 0);

            child_commands->len = 0;
            uint32_t last_child_i = 0;
            uint32_t child_i = mcts->nodes->e[node_i].first_child_i;
            while (child_i != 0) {
                arr_push(scratch, child_commands, mcts->nodes->e[child_i].command);
                last_child_i = child_i;
                child_i = mcts->nodes->e[child_i].next_child_i;
            }

            unexpanded_commands->len = 0;
            CommandSlice node_commands = game_valid_commands(scratch, &mcts->nodes->e[node_i].game);
            for (uint64_t i = 0; i < node_commands.len; i++) {
                bool found = false;
                for (uint64_t j = 0; j < child_commands->len; j++) {
                    if (command_eq(node_commands.e[i], child_commands->e[j])) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    arr_push(scratch, unexpanded_commands, node_commands.e[i]);
                }
            }
            assert(unexpanded_commands->len > 0);
            Command child_command = ai_select_command_heuristic(&mcts->nodes->e[node_i].game,
                                                                (CommandSlice) arr_slice(unexpanded_commands));

            // todo handle volleys with chance nodes.

            Game child_game = mcts->nodes->e[node_i].game;
            game_apply_command(&child_game, child_game.turn.player, child_command, VOLLEY_ROLL);
            CommandSlice next_commands = game_valid_commands(scratch, &child_game);

            uint32_t new_child_i = (uint32_t) mcts->nodes->len;
            arr_push(arena, (mcts->nodes), ((Node) {
                    .kind = child_game.status == STATUS_OVER ? NODE_OVER : NODE_DECISION,
                    .game = child_game,
                    .command = child_command,
                    .parent_i = node_i,
                    .num_unexpanded_children = (uint32_t) next_commands.len,
                    .first_child_i = 0,
                    .next_child_i = 0,
                    .visits = 0,
                    .total_reward = 0,
                    .probability = 1.0,
            }));
            if (last_child_i == 0) {
                mcts->nodes->e[node_i].first_child_i = new_child_i;
            } else {
                mcts->nodes->e[last_child_i].next_child_i = new_child_i;
            }
            mcts->nodes->e[node_i].num_unexpanded_children--;
            node_i = new_child_i;
        }

        Player scored_player = mcts->nodes->e[mcts->root].game.turn.player;
        if (mcts->nodes->e[node_i].parent_i != 0) {
            scored_player = mcts->nodes->e[mcts->nodes->e[node_i].parent_i].game.turn.player;
        }

        // Simulation
        double score;
        if (mcts->nodes->e[node_i].kind != NODE_OVER) {
            Game sim_game = mcts->nodes->e[node_i].game;
            ai_rollout(sim_game, (Command) {0}, 300);
            score = heuristic_value(&sim_game, scored_player);
        } else {
            score = heuristic_value(&mcts->nodes->e[node_i].game, scored_player);
        }

        // Backpropagation
        while (node_i != 0) {
            if ((node_i == 1 && scored_player == game->turn.player) ||
                (mcts->nodes->e[mcts->nodes->e[node_i].parent_i].game.turn.player == scored_player)) {
                mcts->nodes->e[node_i].total_reward += score;
            } else {
                mcts->nodes->e[node_i].total_reward -= score;
            }
            mcts->nodes->e[node_i].visits++;
            node_i = mcts->nodes->e[node_i].parent_i;
        }
    }

    // Select command.

    uint32_t most_visits = 0;
    uint32_t best_child_i = 0;
    uint32_t child_i = mcts->nodes->e[mcts->root].first_child_i;
    while (child_i != 0) {
        if (mcts->nodes->e[child_i].visits >= most_visits) {
            most_visits = mcts->nodes->e[child_i].visits;
            best_child_i = child_i;
        }
        child_i = mcts->nodes->e[child_i].next_child_i;
    }

    scratch_release();
    return mcts->nodes->e[best_child_i].command;
}

#if 0
// lots of problems with this still
// should use a pool of nodes probably instead so I can drop the ones that are not needed.
// makes this less cache friendly though, since children won't necessarily be packed together.

Command ai_select_command_mcts_old(Arena *arena, void **ai_state, Game *game, CommandSlice commands) {
    Arena *scratch = scratch_acquire();

    MCTSState *mcts;
    uint32_t new_root = 0;
    if (*ai_state != NULL) {
        // Try and find the new root in the old tree.
        mcts = (MCTSState *) *ai_state;

        for (uint32_t i = 0; i < mcts->nodes->len; i++) {
            if (mcts->nodes->e[i].kind == NODE_DECISION && game_eq(&mcts->nodes_info->e[i].game, game)) {
                //new_root = i;
                break;
            }

        }
    }
    if (new_root == 0) {
        //printf("reinitializing mcts\n");
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

    CommandArray *unexpanded_commands = NULL;

    int passes = 100 * commands.len + 1;
    double c = sqrt(2);
    for (int pass = 0; pass < passes; pass++) {
        // Selection.
        uint32_t node_i = mcts->root;

        bool found = false;
        while (!found) {
            uint32_t highest_uct_i = 0;
            if (unexpanded_commands != NULL) {
                unexpanded_commands->len = 0;
            }
            if (mcts->nodes->e[node_i].kind == NODE_OVER || mcts->nodes->e[node_i].visits == 0) {
                break;
            } else if (mcts->nodes->e[node_i].kind == NODE_DECISION) {
                double highest_uct = -INFINITY;
                for (uint32_t child_i = mcts->nodes->e[node_i].first_child_i;
                     child_i < mcts->nodes->e[node_i].first_child_i + mcts->nodes->e[node_i].num_children; child_i++) {
                    if (mcts->nodes->e[child_i].visits == 0) {
                        arr_push(scratch, unexpanded_commands, mcts->nodes_info->e[child_i].command);
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
                        arr_push(scratch, unexpanded_commands, mcts->nodes_info->e[child_i].command);
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

            if (unexpanded_commands != NULL && unexpanded_commands->len > 0) {
                // select best unexpanded command.
                Command next_command = ai_select_command_heuristic(&mcts->nodes_info->e[node_i].game,
                                                                   (CommandSlice) arr_slice(unexpanded_commands));
                for (uint32_t child_i = mcts->nodes->e[node_i].first_child_i;
                     child_i < mcts->nodes->e[node_i].first_child_i + mcts->nodes->e[node_i].num_children; child_i++) {
                    if (mcts->nodes_info->e[child_i].command.kind == next_command.kind
                        && mcts->nodes_info->e[child_i].command.piece_id == next_command.piece_id
                        && cpos_eq(mcts->nodes_info->e[child_i].command.target, next_command.target)) {
                        node_i = child_i;
                        found = true;
                        break;
                    }
                }
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
        double adjusted_score = (score + 46) / (46 * 2);

//        if (adjusted_score > 0 && adjusted_score < 1 && (adjusted_score < 0.5 || adjusted_score > 0.5)) {
//            printf("score = %f\n", score);
//            printf("adjusted_score = %f\n", adjusted_score);
//        }

        // backprop
        Player apply_player = mcts->nodes->e[node_i].player;
        uint32_t apply_i = node_i;
        while (apply_i != 0) {
            Node *apply_node = mcts->nodes->e + apply_i;
            apply_node->visits++;
            if (apply_player == apply_node->player) {
                apply_node->total_reward += adjusted_score;
            } else {
                apply_node->total_reward += 1.0 - adjusted_score;
            }
            apply_i = apply_node->parent_i;
        }
    }

    uint32_t most_visits = 0;
    uint32_t best_child_i = 0;
    for (uint32_t child_i = mcts->nodes->e[mcts->root].first_child_i;
         child_i < mcts->nodes->e[mcts->root].first_child_i + mcts->nodes->e[mcts->root].num_children; child_i++) {
//        printf("child_i = %d\n", child_i);
//        printf("visits = %d\n", mcts->nodes->e[child_i].visits);
        if (mcts->nodes->e[child_i].visits >= most_visits) {
            most_visits = mcts->nodes->e[child_i].visits;
            best_child_i = child_i;
        }
    }

    scratch_release();
    return mcts->nodes_info->e[best_child_i].command;
}
#endif

int ai_test(void) {


    for (int g = 0; g < 10; g++) {
        printf("game %d\n", g);
        Arena *a = arena_new();
        Arena *ai_arena = arena_new();
        void *ai_state = NULL;

        Game game = {0};
        game_init_attrition_hex_field_small(&game);

        int turn = 0;
        while (game.status == STATUS_IN_PROGRESS) {

            CommandSlice commands = game_valid_commands(a, &game);

            Command command;
            if (game.turn.player == PLAYER_RED) {
                //command = ai_select_command_heuristic(&game, commands);
                command = ai_select_command_uniform_rollouts(&game, commands);
            } else {
                //command = ai_select_command_uniform_rollouts(&game, commands);
                command = ai_select_command_mcts(ai_arena, &ai_state, &game, commands);
            }
            game_apply_command(&game, game.turn.player, command, VOLLEY_ROLL);
        }
        printf("winner %s\n", game.winner == PLAYER_RED ? "red" : "blue");
        arena_free(a);
        arena_free(ai_arena);
    }

    return 0;
}



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