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

double ai_rollout(Game game, Command command, int depth) {
    Arena *scratch = scratch_acquire();

    Player scoring_player = game.turn.player;
    Player scoring_opponent = scoring_player == PLAYER_RED ? PLAYER_BLUE : PLAYER_RED;
    int scoring_player_gold = game.gold[scoring_player];
    int scoring_opponent_gold = game.gold[scoring_opponent];

    game_apply_command(&game, game.turn.player, command);

    while (game.status == STATUS_IN_PROGRESS && depth > 0) {
        CommandSlice commands = game_valid_commands(scratch, &game);
        Command picked_command = ai_select_command_random(&game, commands);
        game_apply_command(&game, game.turn.player, picked_command);
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

    // Bigger depth makes the ai a lot smarter. Going to use lots of rollouts with a high depth for training in RL.
    for (uint64_t i = 0; i < commands.len; i++) {
        double total_score = 0.5;
        for (int rollout = 0; rollout < 100; rollout++) {
            Game rollout_game = *game;
            double score = ai_rollout(rollout_game, commands.e[i], 20);
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

// Next thing to try is MCTS. Where you keep a tree of game states and search through it with simulations that expand it
// and then backpropagate the results.
// This way the search is targeted towards promising moves instead of uniform over all moves.

// If we wanna re-use the tree across turns, which we do, it'd be nice to have an easy way to prune it. Drop all nodes
// not referenced by the root and re-use that memory for new nodes.

typedef enum {
    NODE_NONE,
    NODE_MOVE,
    NODE_ROLL, // volley rolls.
} NodeKind;

typedef struct {
    Game game;
    Command command;
} NodeInfo;

typedef struct {
    NodeKind kind;      // This is literally a single bit of information, waste.
    uint32_t parent_i;
    uint32_t first_child_i;
    uint32_t num_children;
    uint32_t visits;
    double total_reward;
} Node;

typedef Array(NodeInfo) NodeInfoArray;
typedef Array(Node) NodeArray;

typedef struct {
    uint32_t root;
    NodeArray *nodes;
    NodeInfoArray *nodes_info;
} MCTSState;

// I think I probably want to make this a little more complex.
// Each frame the AI should be able to think for a certain amount of time.
// Then after some number of frames, it should be asked to pick a move.
// Putting the search on a separate thread would make that even easier because then a budget could be in
// iterations instead of in seconds.

Command ai_select_command_mcts(Arena *arena, void **ai_state, Game *game, CommandSlice commands) {
    MCTSState *mcts;
    if (*ai_state == NULL) {
        mcts = arena_alloc(arena, MCTSState);
        // Initialize index 0 as a zero node.
        // Makes it so nodes and nodes_info are not NULL so we don't have to check for that.
        arr_push(arena, mcts->nodes, ((Node) {
                .kind = NODE_NONE,
                .parent_i = 0,
                .first_child_i = 0,
                .num_children = 0,
                .visits = 0,
                .total_reward = 0,
        }));
        arr_push(arena, mcts->nodes_info, ((NodeInfo) {
                .game = (Game) {0},
                .command = (Command) {0},
        }));

        // Push game and set it as the root node.
        arr_push(arena, mcts->nodes, ((Node) {
                .kind = NODE_NONE,
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

        // expand the root node to start.
        // the passed in commands are the commands for the root node.
        Node *root = mcts->nodes->e + mcts->root;
        NodeInfo *root_node_info = mcts->nodes_info->e + mcts->root;
        root->first_child_i = mcts->nodes->len;
        for (int i = 0; i < commands.len; i++) {
            // todo: deal with volley nodes.
            Game child_game = root_node_info->game;
            game_apply_command(&child_game, child_game.turn.player, commands.e[i]);
            arr_push(arena, mcts->nodes, ((Node) {
                    .kind = NODE_MOVE,
                    .parent_i = mcts->root,
                    .first_child_i = 0,
                    .num_children = 0,
                    .visits = 0,
                    .total_reward = 0,
            }));
            arr_push(arena, mcts->nodes_info, ((NodeInfo) {
                    .game = child_game,
                    .command = commands.e[i],
            }));
            root->num_children++;
        }
        assert(root->num_children == commands.len);

        // simulate the root.
        for (int i = 0; i < 100; i++) {

        }

        *ai_state = mcts;
    } else {
        mcts = (MCTSState *) *ai_state;
    }

    Node *root = mcts->nodes->e + mcts->root;



    // a node is "fully expanded" when all it's children have at least 1 visit. When doing selection you stop if
    // the node you hit isn't fully expanded.
    // expand a node is to pick the next child that doesn't have a visit.
    // you simulate that node, which is to do a rollout for it.
    // - this is when you allocate it's children too.
    // then it's visit gets updated during backpropigation, and it's now a target for selection (but won't be selected
    // until it's parent is fully expanded.)

    // So we start by simulating the root node. This allocates it's children and gives it a first visit and value.




    // If game != root game, then we're reusing the tree on a new turn.
    // Do a bfs to try and find the new root. If we find it set it, otherwise we can start a new tree.
    // One problem is that we can't really reclaim all the memory from the no longer connected branches.
    // This is unfortunate, this is the kind of thing that would be good with persistent data structures
    // and garbage collection. We could do manual garbage collection ourselves if we used a general purpose allocator,
    // but we're using an arena so we can't do that right now.
    // Could work to just copy the subtree out of the old arena and reset the old one. I really like that idea actually.

    // First pass at this I'm just gonna throw it away after selecting a move.

    for (int pass = 0; pass < 100; pass++) {
        // Selection.
        // Expansion.
        // Simulation.
        // Back-Propagation.
    }

    // Return the command for the child of the root with the most visits.

    return (Command) {.kind = COMMAND_END_TURN, .piece_id = 0, .target = (CPos) {0, 0, 0}};
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