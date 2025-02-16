#ifndef TAZAR_H
#define TAZAR_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t x;
    int32_t y;
} V2;

typedef struct {
    int32_t q;
    int32_t r;
    int32_t s;
} CPos;

#define CPOS_RIGHT_UP (CPos){1, -1, 0}
#define CPOS_RIGHT (CPos){1, 0, -1}
#define CPOS_RIGHT_DOWN (CPos){0, 1, -1}
#define CPOS_LEFT_DOWN (CPos){-1, 1, 0}
#define CPOS_LEFT (CPos){-1, 0, 1}
#define CPOS_LEFT_UP (CPos){0, -1, 1}

bool cpos_eq(CPos a, CPos b);
CPos cpos_add(CPos a, CPos b);
V2 v2_from_cpos(CPos cpos);
CPos cpos_from_v2(V2 dpos);

typedef enum { TILE_NONE = 0, TILE_NORMAL, TILE_HILL, TILE_MARSH } Tile;

typedef enum {
    PIECE_NONE = 0,
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
    CPos pos;
    int32_t id;
} Piece;

int piece_gold(PieceKind kind);

typedef enum {
    ORDER_NONE = 0,
    ORDER_MOVE,
    ORDER_VOLLEY,
    ORDER_MUSTER,
} OrderKind;

typedef struct {
    OrderKind kind;
    CPos target;
} Order;

typedef struct {
    int32_t piece_id;
    Order orders[2];
    int32_t order_i;
} Activation;

// A player's turn is broken up into activations and orders.
// You can activate up to two pieces per turn.
// Each activated piece can be given up to two orders.
typedef struct {
    Player player;
    Activation activations[2];
    int32_t activation_i;
} Turn;

typedef enum {
    STATUS_NONE = 0,
    STATUS_IN_PROGRESS,
    STATUS_OVER,
} Status;

typedef enum {
    GAME_MODE_NONE = 0,
    GAME_MODE_TOURNAMENT,
    GAME_MODE_ATTRITION,
} GameMode;

typedef enum {
    MAP_NONE = 0,
    MAP_HEX_FIELD_SMALL,
} Map;

typedef struct {
    // Tiles of the board and pieces on the board.
    // Indexed by q and r but offset so that 0,0 is in the center.
    // So 0,0,0 is in slot board[2048][2048]
    // This is certainly a lot of empty space but a uniform representation
    // like this makes it easier to pass the same shape to the (future) nn.
    Tile board[4096];
    Piece pieces[4096];

    // The height and width of the board in double positions.
    // Used by the UI to know how large to draw the board.
    // @todo: Use these when scanning board and pieces so you don't have
    //        to scan the whole thing.
    int32_t dpos_width;
    int32_t dpos_height;

    GameMode game_mode;
    Map map;
    Status status;
    Player winner;

    // @note: hardcoded to 2 players
    int32_t gold[3];        // indexed by player.
    int32_t reserves[3][5]; // indexed by player and piece kind.

    Turn turn;
} Game;

bool game_eq(Game *a, Game *b);
Tile *board_at(Game *game, CPos pos);
Piece *piece_at(Game *game, CPos pos);

void game_init(Game *game, GameMode game_mode, Map map);

typedef enum {
    COMMAND_NONE = 0,
    COMMAND_MOVE,
    COMMAND_VOLLEY,
    COMMAND_MUSTER,
    COMMAND_END_TURN,
} CommandKind;

// Orders are the moves that a piece makes.
// Commands represent what the player "says" to do.
// They are almost the same thing, except that commands have slightly
// more information like the type of piece to muster and there is an
// additional command "end turn".
// The `game_valid_commands` function, and the logic in `game_apply_command`
// ensure you can only command valid orders, and rules like orders per activation
// and number of activations are enforced.
typedef struct {
    CommandKind kind;
    CPos piece_pos;
    CPos target_pos;
    PieceKind muster_piece_kind;
} Command;

bool command_eq(Command *a, Command *b);

typedef struct {
    Command *commands;
    int32_t commands_count;
    int32_t commands_cap;
} CommandBuf;

void game_valid_commands(CommandBuf *command_buf, Game *game);

typedef enum {
    VOLLEY_ROLL,
    VOLLEY_HIT,
    VOLLEY_MISS,
} VolleyResult;

void game_apply_command(Game *game, Player player, Command command, VolleyResult volley_result);

#endif // TAZAR_H