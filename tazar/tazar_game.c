#include "steve.h"
#include "tazar.h"

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

bool cpos_eq(CPos a, CPos b) {
    return a.q == b.q && a.r == b.r && a.s == b.s;
}

CPos cpos_add(CPos a, CPos b) {
    return (CPos) {a.q + b.q, a.r + b.r, a.s + b.s};
}

static Piece piece_null = (Piece) {
        .kind = PIECE_NONE,
        .player = PLAYER_NONE,
        .id = 0,
};

// @note: Hardcoded to "Hex Field Small".
Piece *board_at(Game *game, CPos pos) {
    if ((pos.q + pos.r + pos.s != 0) ||
        (pos.q < -4 || pos.q > 4 || pos.r < -4 || pos.r > 4 || pos.s < -4 ||
         pos.s > 4)) {
        return &piece_null;
    }
    int index = ((pos.r + 4) * 9) + (pos.q + 4);
    return &game->board[index];
}

typedef struct {
    bool found;
    CPos cpos;
} FindPieceResult;

FindPieceResult find_piece(Game *game, Player player, PieceKind kind, int id) {
    for (int r = -4; r <= 4; r++) {
        for (int q = -4; q <= 4; q++) {
            CPos cpos = {q, r, -q - r};
            Piece *piece = board_at(game, cpos);
            if (piece->player == player && piece->kind == kind && piece->id == id) {
                return (FindPieceResult) {true, cpos};
            }
        }
    }
    return (FindPieceResult) {false, (CPos) {0, 0, 0}};
}

void game_init_attrition_hex_field_small(Game *game) {
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

    game->status = STATUS_IN_PROGRESS;
    game->active_player = PLAYER_RED;
    game->active_piece = (Piece) {.kind = PIECE_NONE};
    game->turn_actions_left = 1;
    game->winner = PLAYER_NONE;
}

typedef Array(CPos) CPosArray;

ActionSlice game_valid_actions(Arena *a, Game *game, Player player) {
    Arena *scratch = scratch_acquire();
    ActionArray *actions = NULL;

    for (int r = -4; r <= 4; r++) {
        for (int q = -4; q <= 4; q++) {
            CPos cpos = {q, r, -q - r};
            Piece *piece = board_at(game, cpos);
            if (piece->player != player) {
                continue;
            }
            // Figure out all the valid moves.
            // Figure out targets in charge range.
            // Figure out targets in volley range.
            switch (piece->kind) {
                case PIECE_CROWN: {
                    break;
                }
                case PIECE_PIKE: {
                    CPosArray *visited = NULL;
                    arr_push(scratch, visited, cpos);

                    uint64_t i = 0;
                    for (uint64_t steps = 0; steps < 2; steps++) {
                        uint64_t current_len = visited->len;
                        for (; i < current_len; i++) {
                            CPos current = visited->e[i];
                            Piece *current_piece = board_at(game, current);
                            // Check if this is a valid piece to keep searching from.
                            if (!(cpos_eq(current, cpos) || current_piece->kind == PIECE_EMPTY)) {
                                continue;
                            }
                            CPos neighbors[6] = {
                                    cpos_add(current, CPOS_RIGHT_UP),
                                    cpos_add(current, CPOS_RIGHT),
                                    cpos_add(current, CPOS_RIGHT_DOWN),
                                    cpos_add(current, CPOS_LEFT_DOWN),
                                    cpos_add(current, CPOS_LEFT),
                                    cpos_add(current, CPOS_LEFT_UP),
                            };
                            for (int n = 0; n < 6; n++) {
                                CPos neighbor = neighbors[n];
                                bool already_checked = false;
                                for (uint64_t j = 0; j < visited->len; j++) {
                                    if (cpos_eq(visited->e[j], neighbor)) {
                                        already_checked = true;
                                        break;
                                    }
                                }
                                if (already_checked) {
                                    continue;
                                }

                                Piece *neighbor_piece = board_at(game, neighbor);
                                if (neighbor_piece->kind == PIECE_EMPTY ||
                                    (neighbor_piece->kind != PIECE_NONE && neighbor_piece->kind != PIECE_PIKE &&
                                     neighbor_piece->player != player)) {
                                    arr_push(scratch, visited, neighbor);
                                    arr_push(a, actions, ((Action) {
                                            .kind = ACTION_MOVE,
                                            .piece = piece->kind,
                                            .piece_id = piece->id,
                                            .target = neighbor
                                    }));
                                }
                            }
                        }
                    }
                    break;
                }
                case PIECE_HORSE: {
                    break;
                }
                case PIECE_BOW: {
                    break;
                }
                default:
                    break;
            }
        }
    }

    scratch_release();
    if (actions == NULL) {
        return (ActionSlice) {0};
    } else {
        return (ActionSlice) arr_slice(actions);
    }
}

// Find valid actions.

// Check if an action is valid.

// Apply an action.
// Returns an error message if the action is invalid.
String game_apply_action(Arena *a, Game *game, Player player, Action action) {
    String error = {0};
    Arena *scratch = scratch_acquire();

    if (game->active_player != player) {
        error = str_format(a, "Not %s's turn", player == PLAYER_RED ? "Red" : "Blue");
        goto end;
    }

    FindPieceResult found_piece = find_piece(game, player, action.piece, action.piece_id);
    if (!found_piece.found) {
        error = str_format(a, "No piece %d of kind %d", action.piece_id, action.piece);
        goto end;
    }
    CPos start_cpos = found_piece.cpos;
    Piece *piece = board_at(game, found_piece.cpos);

    switch (action.kind) {
        case ACTION_MOVE: {
            CPos target = cpos_add(start_cpos, action.target);
            Piece *target_piece = board_at(game, target);
            if (target_piece->kind != PIECE_EMPTY) {
                error = str_format(a, "Can't move to %d,%d,%d, it's occupied or invalid.", target.q, target.r,
                                   target.s);
                goto end;
            }
            *target_piece = *piece;
            *piece = (Piece) {.kind = PIECE_EMPTY, .player = PLAYER_NONE, .id = 0};
            break;
        }
        default: {
            error = str_format(a, "Unimplemented action kind %d", action.kind);
            goto end;
        }
    }

    end:
    scratch_release();
    return error;
}