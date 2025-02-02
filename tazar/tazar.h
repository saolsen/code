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

#define CPOS_RIGHT_UP (CPos){1,-1,0}
#define CPOS_RIGHT (CPos){1,0,-1}
#define CPOS_RIGHT_DOWN (CPos){0,1,-1}
#define CPOS_LEFT_DOWN (CPos){-1,1,0}
#define CPOS_LEFT (CPos){-1,0,1}
#define CPOS_LEFT_UP (CPos){0,-1,1}

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

typedef enum {
    ACTION_NONE = 0,
    ACTION_MOVE,
    ACTION_VOLLEY,
    ACTION_CHARGE,
    ACTION_END,
} ActionKind;

typedef struct {
    ActionKind kind;
    PieceKind piece;
    int piece_id;
    CPos target; // target is relative to the unit.
} Action;

typedef struct {
    Action action;
    String error;
} ParseAction;

String piece_str(Arena *a, Piece piece);

ParseAction action_parse(Arena *a, String action_str);

typedef enum {
    STATUS_NONE = 0,
    STATUS_IN_PROGRESS,
    STATUS_OVER,
} Status;

typedef struct {
    Piece board[81];
    Status status;
    Player active_player;
    Piece active_piece;
    int turn_actions_left;
    Player winner;
} Game;

void game_init_attrition_hex_field_small(Game *game);

// Apply an action.
// Returns an error message if the action is invalid.
String game_apply_action(Arena *a, Game *game, Player player, Action action);

#endif //TAZAR_H
