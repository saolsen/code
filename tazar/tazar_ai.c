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
    Command picked_command = commands.e[0];
    for (uint64_t i = 1; i < commands.len; i++) {
        r -= policy.e[i];
        if (r <= 0) {
            picked_command = commands.e[i];
            break;
        }
    }
    scratch_release();
    return picked_command;
}

double ai_rollout(Game *game, Command command, int depth) {
    Arena *scratch = scratch_acquire();

    Player scoring_player = game->turn.player;
    game_apply_command(game, game->turn.player, command, VOLLEY_ROLL);

    while (game->status == STATUS_IN_PROGRESS && depth > 0) {
        CommandSlice commands = game_valid_commands(scratch, game);
        Command next_command = ai_select_command_heuristic(game, commands);
        game_apply_command(game, game->turn.player, next_command, VOLLEY_ROLL);
        depth--;
    }

    double score = heuristic_value(game, scoring_player);
    scratch_release();
    return score;
}

Command ai_select_command_uniform_rollouts(Game *game, CommandSlice commands) {
    Arena *scratch = scratch_acquire();

    DoubleArray *scores = NULL;
    arr_setlen(scratch, scores, commands.len);

    for (uint64_t i = 0; i < commands.len; i++) {
        double total_score = 0;
        int passes = 100;
        for (int rollout = 0; rollout < passes; rollout++) {
            Game rollout_game = *game;
            double score = ai_rollout(&rollout_game, commands.e[i], 299);
            double adjusted_score = (score + 46) / (46 * 2);

            total_score += adjusted_score;
        }
        double avg_score = total_score / 100;

        scores->e[i] = avg_score;
    }

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

typedef struct {
    Game game;
    Command command;
} NodeInfo;

typedef Array(Node) NodeArray;

Node zero_node = (Node) {
        .kind = NODE_NONE,
        .game = (Game) {0},
        .command = (Command) {0},
        .parent_i = 0,
        .first_child_i = 0,
        .num_children = 0,
        .num_children_to_expand = 0,
        .visits = 0,
        .total_reward = 0,
        .probability = 0,
};

void push_node(Node **buf, uintptr_t *len, uintptr_t *cap, Node n) {
    // allocate more free space if needed
    if (*len >= *cap) {
        if (*cap == 0) {
            *cap = 1024;
        } else {
            *cap *= 2;
        }
        *buf = realloc(*buf, *cap * sizeof(**buf));
    }
    (*buf)[*len] = n;
    *len += 1;
}

void push_command(Command **buf, uintptr_t *len, uintptr_t *cap, Command command) {
    // allocate more free space if needed
    if (*len >= *cap) {
        if (*cap == 0) {
            *cap = 1024;
        } else {
            *cap *= 2;
        }
        *buf = realloc(*buf, *cap * sizeof(**buf));
    }
    (*buf)[*len] = command;
    *len += 1;
}

Command ai_select_command_mcts(MCTSState *ai_state, Game *game, CommandSlice commands) {
    Arena *scratch = scratch_acquire();

    Node *nodes = NULL;
    uintptr_t nodes_len = 0;
    uintptr_t nodes_cap = 0;

    push_node(&nodes, &nodes_len, &nodes_cap, zero_node);
    push_node(&nodes, &nodes_len, &nodes_cap, (Node) {
            .kind = NODE_DECISION,
            .game = *game,
            .command = (Command) {0},
            .parent_i = 0,
            .first_child_i = 0,
            .num_children = 0,
            .num_children_to_expand = (uint32_t) commands.len,
            .visits = 0,
            .total_reward = 0,
            .probability = 1.0,
    });
    assert(nodes_len == 2);

    uint32_t root_i = 1;

    Command *unexpanded_commands = NULL;
    uintptr_t unexpanded_commands_len = 0;
    uintptr_t unexpanded_commands_cap = 0;

    double c = sqrt(2);
    double dpw_k = 1.0; // @note: tuneable
    double dpw_alpha = 0.5;

    uint32_t passes = 100 * (uint32_t) commands.len + 1;
    for (uint32_t pass = 0; pass < passes; pass++) {
        // Selection
        uint32_t node_i = root_i;
        //while (nodes[node_i].kind != NODE_OVER && nodes[node_i].num_children_to_expand == 0) {
        while (true) {
            if (nodes[node_i].kind == NODE_OVER) {
                break;
            }

            if (nodes[node_i].kind == NODE_CHANCE) {
                double r = (double) rand() / RAND_MAX;  // random value in [0,1)
                double cumulative = 0.0;
                uint32_t child_i = 0;
                bool found = false;
                assert(nodes[node_i].num_children == 2);
                for (uint32_t i = 0; i < nodes[node_i].num_children; i++) {
                    child_i = nodes[node_i].first_child_i + i;
                    cumulative += nodes[child_i].probability;
                    if (r <= cumulative) {
                        found = true;
                        break;
                    }
                }
                assert(found);
                node_i = child_i;
                continue;
            }

            assert(nodes[node_i].kind == NODE_DECISION);
            double allowed_children = dpw_k * pow((double) nodes[node_i].visits, dpw_alpha) + 1;
            if (nodes[node_i].num_children_to_expand > 0 &&
                nodes[node_i].num_children < allowed_children) {
                // Expand a new child.
                break;
            }

            // Pick child with highest UCT.
            assert(nodes[node_i].num_children > 0);
            double highest_uct = -INFINITY;
            uint32_t highest_uct_i = 0;
            for (uint32_t i = 0; i < nodes[node_i].num_children; i++) {
                uint32_t child_i = nodes[node_i].first_child_i + i;
                double child_uct = (nodes[child_i].total_reward / nodes[child_i].visits) +
                                   c * sqrt(log(nodes[node_i].visits) / nodes[child_i].visits);
                if (child_uct > highest_uct) {
                    highest_uct = child_uct;
                    highest_uct_i = child_i;
                }
            }
            node_i = highest_uct_i;
        }

        uint32_t nodes_to_simulate[2] = {node_i, 0};

        // Expansion
        if (nodes[node_i].kind != NODE_OVER) {
            assert(nodes[node_i].num_children_to_expand > 0);

            // If this is the first child, allocate the buffer.
            if (nodes[node_i].num_children == 0) {
                assert(nodes[node_i].first_child_i == 0);
                nodes[node_i].first_child_i = (uint32_t) nodes_len;
                for (uint32_t i = 0; i < nodes[node_i].num_children_to_expand; i++) {
                    push_node(&nodes, &nodes_len, &nodes_cap, zero_node);
                }
                assert(nodes_len == nodes[node_i].first_child_i + nodes[node_i].num_children_to_expand);
            }

            CommandSlice node_commands = game_valid_commands(scratch, &nodes[node_i].game);
            assert(node_commands.len == nodes[node_i].num_children_to_expand + nodes[node_i].num_children);
            unexpanded_commands_len = 0;
            for (uint64_t i = 0; i < node_commands.len; i++) {
                bool found = false;
                for (uint32_t j = 0; j < nodes[node_i].num_children; j++) {
                    uint32_t child_i = nodes[node_i].first_child_i + j;
                    if (command_eq(node_commands.e[i], nodes[child_i].command)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    push_command(&unexpanded_commands, &unexpanded_commands_len, &unexpanded_commands_cap,
                                 node_commands.e[i]);
                }
            }
            assert(unexpanded_commands_len > 0);
            Command child_command = ai_select_command_heuristic(&nodes[node_i].game,
                                                                (CommandSlice) {.len = unexpanded_commands_len,
                                                                        .e = unexpanded_commands});

            uint32_t next_child_i = nodes[node_i].first_child_i + nodes[node_i].num_children;
            Game child_game = nodes[node_i].game;

            if (child_command.kind == COMMAND_VOLLEY) {
                Game hit_game = child_game;
                Game miss_game = child_game;
                game_apply_command(&hit_game, hit_game.turn.player, child_command, VOLLEY_HIT);
                game_apply_command(&miss_game, miss_game.turn.player, child_command, VOLLEY_MISS);
                CommandSlice hit_next_commands = game_valid_commands(scratch, &hit_game);
                CommandSlice miss_next_commands = game_valid_commands(scratch, &miss_game);

                uint32_t hit_child_i = (uint32_t) nodes_len;
                uint32_t miss_child_i = hit_child_i + 1;

                nodes[next_child_i] = (Node) {
                        .kind = NODE_CHANCE,
                        .game = child_game,
                        .command = child_command,
                        .parent_i = node_i,
                        .first_child_i = hit_child_i,
                        .num_children = 2,
                        .num_children_to_expand = 0,
                        .visits = 0,
                        .total_reward = 0,
                        .probability = 1.0,
                };
                nodes[node_i].num_children++;
                nodes[node_i].num_children_to_expand--;

                push_node(&nodes, &nodes_len, &nodes_cap, (Node) {
                        .kind = NODE_DECISION,
                        .game = hit_game,
                        .command = child_command,
                        .parent_i = next_child_i,
                        .first_child_i = 0,
                        .num_children = 0,
                        .num_children_to_expand = (uint32_t) hit_next_commands.len,
                        .visits = 0,
                        .total_reward = 0,
                        .probability = 0.4167,
                });
                push_node(&nodes, &nodes_len, &nodes_cap, (Node) {
                        .kind = NODE_DECISION,
                        .game = miss_game,
                        .command = child_command,
                        .parent_i = next_child_i,
                        .first_child_i = 0,
                        .num_children = 0,
                        .num_children_to_expand = (uint32_t) miss_next_commands.len,
                        .visits = 0,
                        .total_reward = 0,
                        .probability = 1 - 0.4167,
                });
                nodes_to_simulate[0] = hit_child_i;
                nodes_to_simulate[1] = miss_child_i;
            } else {
                game_apply_command(&child_game, child_game.turn.player, child_command, VOLLEY_ROLL);
                CommandSlice next_commands = game_valid_commands(scratch, &child_game);

                nodes[next_child_i] = (Node) {
                        .kind = child_game.status == STATUS_OVER ? NODE_OVER : NODE_DECISION,
                        .game = child_game,
                        .command = child_command,
                        .parent_i = node_i,
                        .first_child_i = 0,
                        .num_children = 0,
                        .num_children_to_expand = (uint32_t) next_commands.len,
                        .visits = 0,
                        .total_reward = 0,
                        .probability = 1.0,
                };
                nodes[node_i].num_children++;
                nodes[node_i].num_children_to_expand--;
                nodes_to_simulate[0] = next_child_i;
                nodes_to_simulate[1] = 0;
            }
        }

        for (uint32_t i = 0; i < 2; i++) {
            uint32_t sim_i = nodes_to_simulate[i];
            if (sim_i == 0) {
                continue;
            }

            // Scored player is the player that will be scored at the end of the simulation.
            Player scored_player = nodes[root_i].game.turn.player;
            if (nodes[sim_i].parent_i != 0) {
                scored_player = nodes[nodes[sim_i].parent_i].game.turn.player;
            }

            // Simulation
            double score;
            if (nodes[sim_i].kind != NODE_OVER) {
                Game sim_game = nodes[sim_i].game;
                ai_rollout(&sim_game, (Command) {0}, 300);
                score = heuristic_value(&sim_game, scored_player);
            } else {
                score = heuristic_value(&nodes[sim_i].game, scored_player);
            }

            // Backpropagation
            while (sim_i != 0) {
                if ((sim_i == root_i && scored_player == game->turn.player) ||
                    (nodes[nodes[sim_i].parent_i].game.turn.player == scored_player)) {
                    nodes[sim_i].total_reward += score;
                } else {
                    nodes[sim_i].total_reward -= score;
                }
                nodes[sim_i].visits++;
                sim_i = nodes[sim_i].parent_i;
            }
        }
    }

    // Select best command.
    uint32_t most_visits = 0;
    uint32_t best_child_i = 0;

    assert(nodes[root_i].num_children > 0);
    assert(nodes[root_i].num_children == commands.len);
    assert(nodes[root_i].num_children_to_expand == 0);
    for (uint32_t i = 0; i < nodes[root_i].num_children; i++) {
        uint32_t child_i = nodes[root_i].first_child_i + i;
        if (nodes[child_i].visits >= most_visits) {
            most_visits = nodes[child_i].visits;
            best_child_i = child_i;
        }
    }

    Command result = nodes[best_child_i].command;

//    ai_state->root = root_i;
//    ai_state->nodes = nodes;
//    ai_state->nodes_len = nodes_len;
//    ai_state->nodes_cap = nodes_cap;

    free(nodes);
    free(unexpanded_commands);
    scratch_release();

    return result;
}

int ai_test(void) {
    for (int g = 0; g < 10; g++) {
        printf("game %d\n", g);
        MCTSState ai_state = {0};

        Game game = {0};
        game_init_attrition_hex_field_small(&game);

        int turn = 0;
        while (game.status == STATUS_IN_PROGRESS) {
            Arena *scratch = scratch_acquire();
            CommandSlice commands = game_valid_commands(scratch, &game);

            Command command;
            if (game.turn.player == PLAYER_RED) {
                //command = ai_select_command_heuristic(&game, commands);
                command = ai_select_command_uniform_rollouts(&game, commands);
            } else {
                //command = ai_select_command_uniform_rollouts(&game, commands);
                command = ai_select_command_mcts(&ai_state, &game, commands);
            }
            game_apply_command(&game, game.turn.player, command, VOLLEY_ROLL);
            scratch_release();
        }
        printf("winner %s\n", game.winner == PLAYER_RED ? "red" : "blue");
        scratch_free();
    }

    return 0;
}
