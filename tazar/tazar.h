#ifndef TAZAR_H
#define TAZAR_H

#include "steve.h"

// Hex position in double coordinates.
// * Useful coordinates for game logic.
// * These are also axial coordinates if you ignore s.
//   I chose to just always track s instead of calculating it.
typedef struct {
    int q;
    int r;
    int s;
} CPos;

#define CPOS_RIGHT_UP (CPos){1, -1, 0}
#define CPOS_RIGHT (CPos){1, 0, -1}
#define CPOS_RIGHT_DOWN (CPos){0, 1, -1}
#define CPOS_LEFT_DOWN (CPos){-1, 1, 0}
#define CPOS_LEFT (CPos){-1, 0, 1}
#define CPOS_LEFT_UP (CPos){0, -1, 1}

bool cpos_eq(CPos a, CPos b);

CPos cpos_add(CPos a, CPos b);

// Hex position in double coordinates.
// * Useful coordinates for drawing.
typedef struct {
    int x;
    int y;
} DPos;

DPos dpos_from_cpos(CPos cpos);

CPos cpos_from_dpos(DPos dpos);

typedef enum {
    PIECE_NONE = 0,
    PIECE_EMPTY,
    PIECE_CROWN,
    PIECE_PIKE,
    PIECE_HORSE,
    PIECE_BOW,
} PieceKind;

typedef enum {
    PLAYER_NONE = 0,
    PLAYER_RED,
    PLAYER_BLUE,
} Player;

typedef struct {
    PieceKind kind;
    Player player;
    int id;
} Piece;

int piece_gold(PieceKind kind);

typedef enum {
    ORDER_NONE = 0,
    ORDER_MOVE,
    ORDER_VOLLEY,
    // muster
} OrderKind;

typedef struct {
    OrderKind kind;
    CPos target;
} Order;

typedef struct {
    int piece_id;
    Order orders[2];
    int order_i;
} Activation;

typedef struct {
    Player player;
    Activation activations[2];
    int activation_i;
} Turn;

typedef enum {
    STATUS_NONE = 0,
    STATUS_IN_PROGRESS,
    STATUS_OVER,
} Status;

typedef struct {
    Piece board[81];
    Status status;
    Player winner;
    // hardcoded to 2 players
    int gold[3]; // indexed by player, ignore gold[0].
    Turn turn;
} Game;

bool game_eq(Game *a, Game *b);

Piece *board_at(Game *game, CPos pos);

void game_init_attrition_hex_field_small(Game *game);

// commands are you telling the game what to do. these are the things to generate I think.
typedef enum {
    COMMAND_NONE = 0,
    COMMAND_MOVE,
    COMMAND_VOLLEY,
    // muster
    COMMAND_END_TURN,
} CommandKind;

// note: Probably want to specify the piece by CPos so that all possible commands
// can be packed into a single array. Will need that for RL.
typedef struct {
    CommandKind kind;
    int piece_id;
    CPos target;
} Command;

bool command_eq(Command a, Command b);

typedef Slice(Command) CommandSlice;
typedef Array(Command) CommandArray;

CommandSlice game_valid_commands(Arena *a, Game *game);

typedef enum {
    VOLLEY_ROLL,
    VOLLEY_HIT,
    VOLLEY_MISS,
} VolleyResult;

void game_apply_command(Game *game, Player player, Command command, VolleyResult volley_result);

Command ai_select_command_heuristic(Game *game, CommandSlice commands);

Command ai_select_command_uniform_rollouts(Game *game, CommandSlice commands);

typedef enum {
    NODE_NONE,
    NODE_DECISION,
    NODE_CHANCE,
    NODE_OVER,
} NodeKind;

typedef struct {
    NodeKind kind;
    Game game;
    Command command;
    uint32_t parent_i;
    uint32_t first_child_i;
    uint32_t num_children;
    uint32_t num_children_to_expand;
    uint32_t visits;
    double total_reward;
    double probability;
} Node;

typedef struct {
    uint32_t root;
    Node *nodes;
    uintptr_t nodes_len;
    uintptr_t nodes_cap;
} MCTSState;

Command ai_select_command_mcts(MCTSState *ai_state, Game *game, CommandSlice commands);

int ai_test(void);

int ui_main(void);

#endif // TAZAR_H
