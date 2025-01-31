#ifndef TAZAR_H
#define TAZAR_H

#define MAX(a, b) ((a) > (b) ? (a) : (b))

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
} Piece;

typedef struct {
    Piece board[81];
} Game;

void game_init_attrition_hex_field_small(Game *game);

#endif //TAZAR_H
