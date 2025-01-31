#include <string.h>

DPos dpos_from_cpos(CPos cpos) {
    int x = 2 * cpos.q + cpos.r;
    int y = cpos.r;
    return (DPos) {x, y};
}

CPos cpos_from_dpos(DPos dpos) {
    int q = (dpos.x - dpos.y) / 2;
    int r = dpos.y;
    int s = -q - r;
    return (CPos) {q, r, s};
}


CPos cpos_add(CPos a, CPos b) {
    return (CPos) {a.q + b.q, a.r + b.r, a.s + b.s};
}

static Piece piece_null = (Piece) {
        .kind = PIECE_NONE,
        .player = PLAYER_NONE,
        .id = 0,
};

// @note: Hardcoded to game mode "ATTRITION" and map "Hex Field Small".
Piece *board_at(Game *game, CPos pos) {
    if ((pos.q + pos.r + pos.s != 0) ||
        (pos.q < -4 || pos.q > 4 || pos.r < -4 || pos.r > 4 || pos.s < -4 ||
         pos.s > 4)) {
        return &piece_null;
    }
    int index = ((pos.r + 4) * 9) + (pos.q + 4);
    return &game->board[index];
}

// @note: Hardcoded to game mode "ATTRITION" and map "Hex Field Small".
void game_init(Game *game) {
    // Set up board.
    memset(game, 0, sizeof(Game));

    for (int q = -4; q <= 4; q++) {
        for (int r = -4; r <= 4; r++) {
            for (int s = -4; s <= 4; s++) {
                CPos cpos = {q, r, s};
                Piece *piece = board_at(game, cpos);
                if (piece != &piece_null) {
                    *piece = (Piece) {
                            .kind = PIECE_EMPTY,
                            .player = PLAYER_NONE,
                    };
                }
            }
        }
    }

    // Set up attrition.
    Piece red_crown = {
            .kind = PIECE_CROWN,
            .player = PLAYER_RED,
    };
    Piece red_pike = {
            .kind = PIECE_PIKE,
            .player = PLAYER_RED,
    };
    Piece red_horse = {
            .kind = PIECE_HORSE,
            .player = PLAYER_RED,
    };
    Piece red_bow = {
            .kind = PIECE_BOW,
            .player = PLAYER_RED,
    };
    CPos p = (CPos) {-4, 0, 4};
    *board_at(game, p) = red_crown;
    board_at(game, p)->id = 1;
    p = cpos_add(p, CPOS_RIGHT_UP);
    *board_at(game, p) = red_bow;
    board_at(game, p)->id = 1;
    p = cpos_add(p, CPOS_RIGHT_UP);
    *board_at(game, p) = red_horse;
    board_at(game, p)->id = 1;
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = 1;
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = 2;
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(game, p) = red_bow;
    board_at(game, p)->id = 2;
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = 3;
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = 4;
    p = cpos_add(p, CPOS_LEFT);
    *board_at(game, p) = red_bow;
    board_at(game, p)->id = 3;
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(game, p) = red_horse;
    board_at(game, p)->id = 2;
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = 5;

    Piece blue_crown = {
            .kind = PIECE_CROWN,
            .player = PLAYER_BLUE,
    };
    Piece blue_pike = {
            .kind = PIECE_PIKE,
            .player = PLAYER_BLUE,
    };
    Piece blue_horse = {
            .kind = PIECE_HORSE,
            .player = PLAYER_BLUE,
    };
    Piece blue_bow = {
            .kind = PIECE_BOW,
            .player = PLAYER_BLUE,
    };
    p = (CPos) {4, 0, -4};
    *board_at(game, p) = blue_crown;
    board_at(game, p)->id = 1;
    p = cpos_add(p, CPOS_LEFT_UP);
    *board_at(game, p) = blue_bow;
    board_at(game, p)->id = 1;
    p = cpos_add(p, CPOS_LEFT_UP);
    *board_at(game, p) = blue_horse;
    board_at(game, p)->id = 1;
    p = cpos_add(p, CPOS_LEFT);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = 1;
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = 2;
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(game, p) = blue_bow;
    board_at(game, p)->id = 2;
    p = cpos_add(p, CPOS_LEFT);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = 3;
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = 4;
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(game, p) = blue_bow;
    board_at(game, p)->id = 3;
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(game, p) = blue_horse;
    board_at(game, p)->id = 2;
    p = cpos_add(p, CPOS_LEFT);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = 5;
}