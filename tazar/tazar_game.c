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

FindPieceResult find_piece(Game *game, int id) {
    for (int r = -4; r <= 4; r++) {
        for (int q = -4; q <= 4; q++) {
            CPos cpos = {q, r, -q - r};
            Piece *piece = board_at(game, cpos);
            if (piece->id == id) {
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
    int id = 1;
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
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT_UP);
    *board_at(game, p) = red_bow;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT_UP);
    *board_at(game, p) = red_horse;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(game, p) = red_bow;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT);
    *board_at(game, p) = red_bow;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(game, p) = red_horse;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(game, p) = red_pike;
    board_at(game, p)->id = id++;

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
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT_UP);
    *board_at(game, p) = blue_bow;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT_UP);
    *board_at(game, p) = blue_horse;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(game, p) = blue_bow;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(game, p) = blue_bow;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(game, p) = blue_horse;
    board_at(game, p)->id = id++;
    p = cpos_add(p, CPOS_LEFT);
    *board_at(game, p) = blue_pike;
    board_at(game, p)->id = id++;

    game->status = STATUS_IN_PROGRESS;
    game->active_player = PLAYER_RED;
    game->turn_pieces_left = 1; // I think this is a special case for the first turn.
    game->active_piece_id = 0;
    game->active_piece_did_move = false;
    game->active_piece_did_special = false;
    game->winner = PLAYER_NONE;
}

typedef Slice(CPos) CPosSlice;
typedef Array(CPos) CPosArray;

int piece_movement(PieceKind kind) {
    switch (kind) {
        case PIECE_CROWN:
            return 1;
        case PIECE_PIKE:
            return 2;
        case PIECE_HORSE:
            return 4;
        case PIECE_BOW:
            return 2;
        default:
            return 0;
    }
}

int piece_strength(PieceKind kind) {
    switch (kind) {
        case PIECE_CROWN:
            return 0;
        case PIECE_PIKE:
            return 3;
        case PIECE_HORSE:
            return 2;
        case PIECE_BOW:
            return 1;
        default:
            return -1;
    }
}

int piece_gold(PieceKind kind) {
    switch (kind) {
        case PIECE_CROWN:
            return 6;
        case PIECE_PIKE:
            return 1;
        case PIECE_HORSE:
            return 3;
        case PIECE_BOW:
            return 2;
        default:
            return 0;
    }
}

CPosSlice move_targets(Arena *a, Game *game, CPos from) {
    Piece *piece = board_at(game, from);
    int movement = piece_movement(piece->kind);

    CPosArray *visited = NULL;
    arr_push(a, visited, from);

    uint64_t i = 0;
    for (int steps = 0; steps < movement; steps++) {
        uint64_t current_len = visited->len;
        for (; i < current_len; i++) {
            CPos current = visited->e[i];
            Piece *current_piece = board_at(game, current);
            // Check if this is a valid piece to keep searching from.
            if (!(cpos_eq(current, from) || current_piece->kind == PIECE_EMPTY)) {
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
                if (neighbor_piece->kind == PIECE_NONE) {
                    continue;
                }
                if (neighbor_piece->player != piece->player &&
                    piece_strength(piece->kind) > piece_strength(neighbor_piece->kind)) {
                    arr_push(a, visited, neighbor);
                }
            }
        }
    }
    CPosSlice result = arr_slice(visited);
    // Drop the first one because it's the starting position.
    result.len--;
    result.e++;
    return result;
}

// Valid moves for each piece, taking into account it's strength.
// volley targets.
// charge targets.

ActionSlice game_valid_actions(Arena *a, Game *game) {
    Arena *scratch = scratch_acquire();
    ActionArray *actions = NULL;

    for (int r = -4; r <= 4; r++) {
        for (int q = -4; q <= 4; q++) {
            CPos cpos = {q, r, -q - r};
            Piece *piece = board_at(game, cpos);
            if (piece->player != game->active_player) {
                continue;
            }

            //if (piece->id == game->active_piece_id && !game->active_piece_did_move) {
            // Moves
            CPosSlice targets = move_targets(scratch, game, cpos);
            for (uint64_t i = 0; i < targets.len; i++) {
                CPos target = targets.e[i];
                arr_push(a, actions, ((Action) {
                        .kind = ACTION_MOVE,
                        .piece_id = piece->id,
                        .target = target,
                }));
            }
            //}

            // Figure out all the valid moves.
            // Figure out targets in charge range.
            // Figure out targets in volley range.
            switch (piece->kind) {
                case PIECE_CROWN: {

                    break;
                }
                case PIECE_PIKE: {

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

#include <stdio.h>

// Apply an action.
// Returns an error message if the action is invalid.
String game_apply_action(Arena *a, Game *game, Player player, Action action) {
    String error = {0};
    Arena *scratch = scratch_acquire();

    // validate action
    if (game->active_player != player) {
        error = str_format(a, "Not %s's turn", player == PLAYER_RED ? "Red" : "Blue");
        goto end;
    }

    FindPieceResult found_piece = find_piece(game, action.piece_id);
    if (!found_piece.found) {
        error = str_format(a, "No piece %d", action.piece_id);
        goto end;
    }
    Piece *piece = board_at(game, found_piece.cpos);

    bool is_first_piece_action;
    printf("active_piece_id: %d\n", game->active_piece_id);
    if (action.piece_id == game->active_piece_id) {
        // We have done an action with the piece already.
        is_first_piece_action = false;
        assert(game->active_piece_did_move || game->active_piece_did_special);

        if (game->active_piece_did_move && game->active_piece_did_special) {
            error = str_format(a, "Already did two actions with this piece.");
            goto end;
        }
        if (game->active_piece_did_move && action.kind == ACTION_MOVE) {
            error = str_format(a, "Already moved this piece.");
            goto end;
        }
        if (game->active_piece_did_special && (action.kind == ACTION_CHARGE || action.kind == ACTION_VOLLEY)) {
            error = str_format(a, "Already did a special action with this piece.");
            goto end;
        }
    } else {
        // This is the first action with this piece.
        is_first_piece_action = true;
        if (game->turn_pieces_left == 0) {
            error = str_format(a, "No more new pieces can be used this turn.");
            goto end;
        }
    }

    switch (action.kind) {
        case ACTION_MOVE: {
            if (!is_first_piece_action && game->active_piece_did_move) {
                error = str_format(a, "Already moved this piece.");
                goto end;
            }

            Piece *target_piece = board_at(game, action.target);
            if (target_piece->kind == PIECE_NONE) {
                error = str_format(a, "Can't move to %d,%d,%d, it's invalid.", action.target.q,
                                   action.target.r,
                                   action.target.s);
                goto end;
            }
            if (target_piece->player == player) {
                error = str_format(a, "Can't move to %d,%d,%d, it's occupied by your own piece.", action.target.q,
                                   action.target.r,
                                   action.target.s);
                goto end;
            }
            if (piece_strength(piece->kind) <= piece_strength(target_piece->kind)) {
                error = str_format(a, "Can't move to %d,%d,%d, it's occupied by a stronger piece.", action.target.q,
                                   action.target.r,
                                   action.target.s);
                goto end;
            }
            //int target_gold_amount = piece_gold(target_piece->kind);
            if (is_first_piece_action) {
                game->turn_pieces_left--;
                game->active_piece_id = action.piece_id;
                game->active_piece_did_move = true;
                game->active_piece_did_special = false;
            } else {
                game->active_piece_id = piece->id;
                game->active_piece_did_move = true;
            }

            if ((piece->kind == PIECE_PIKE && game->active_piece_did_move) ||
                (game->active_piece_did_move && game->active_piece_did_special)) {
                if (game->turn_pieces_left == 0) {
                    // next player's turn
                    game->active_player = game->active_player == PLAYER_RED ? PLAYER_BLUE : PLAYER_RED;
                    game->turn_pieces_left = 2;
                    game->active_piece_id = 0;
                    game->active_piece_did_move = false;
                    game->active_piece_did_special = false;
                }
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

// This feels way too complicated.
// I actually know all the actions are valid.