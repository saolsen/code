#include "steve.h"
#include "tazar.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

bool game_eq(Game *a, Game *b) {
    if (a->status != b->status || a->winner != b->winner) {
        return false;
    }
    for (int i = 0; i < 3; i++) {
        if (a->gold[i] != b->gold[i]) {
            return false;
        }
    }
    if (a->turn.player != b->turn.player || a->turn.activation_i != b->turn.activation_i) {
        return false;
    }
    for (int i = 0; i < 2; i++) {
        if (a->turn.activations[i].piece_id != b->turn.activations[i].piece_id ||
            a->turn.activations[i].order_i != b->turn.activations[i].order_i) {
            return false;
        }
        for (int j = 0; j < 2; j++) {
            if (a->turn.activations[i].orders[j].kind != b->turn.activations[i].orders[j].kind ||
                !cpos_eq(a->turn.activations[i].orders[j].target, b->turn.activations[i].orders[j].target)) {
                return false;
            }
        }
    }
    for (int i = 0; i < 81; i++) {
        if (a->board[i].kind != b->board[i].kind || a->board[i].player != b->board[i].player ||
            a->board[i].id != b->board[i].id) {
            return false;
        }
    }
    return true;
}

static Piece piece_null = {0};

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
    game->winner = PLAYER_NONE;
    for (int i = 0; i < 3; i++) {
        game->gold[i] = 0;
    }
    game->turn.player = PLAYER_RED;

    for (int i = 0; i < 2; i++) {
        game->turn.activations[i].piece_id = 0;
        for (int ii = 0; ii < 2; ii++) {
            game->turn.activations[i].orders[ii].kind = ORDER_NONE;
            game->turn.activations[i].orders[ii].target = (CPos) {0, 0, 0};
        }
        game->turn.activations[i].order_i = 0;
    }
    game->turn.activation_i = 1; // Special case for attrition.
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

CPosSlice volley_targets(Arena *a, Game *game, CPos from) {
    Piece *piece = board_at(game, from);

    CPosArray *result = NULL;

    // Hardcoded for hex field small.
    for (int r = -4; r <= 4; r++) {
        for (int q = -4; q <= 4; q++) {
            CPos cpos = {q, r, -q - r};
            Piece *target_piece = board_at(game, cpos);
            if (target_piece->player != 0 && target_piece->player != piece->player) {
                // Check if the target is in range.
                CPos vec = {cpos.q - from.q, cpos.r - from.r, cpos.s - from.s};
                if ((ABS(vec.q) + ABS(vec.r) + ABS(vec.s)) / 2 <= 2) {
                    arr_push(a, result, cpos);
                }
            }
        }
    }

    if (result == NULL) {
        return (CPosSlice) {0, 0};
    } else {
        return (CPosSlice) arr_slice(result);
    }
}

int move_targets(CPos *targets, int max_targets, Game *game, CPos from) {
    Piece *piece = board_at(game, from);
    int movement = piece_movement(piece->kind);
    int max_strength = piece_strength(piece->kind) - 1;
    if (piece->kind == PIECE_HORSE) {
        max_strength = 3;
    }
    // todo: pending response from the bros, max_strength for crown might be 0.

    CPos visited[64];
    int visited_len = 0;
    visited[visited_len++] = from;

    int i = 0;
    for (int steps = 0; steps < movement; steps++) {
        int current_len = visited_len;
        for (; i < current_len; i++) {
            CPos current = visited[i];
            Piece *current_piece = board_at(game, current);
            // Check if this is a valid piece to keep searching from.
            if (!(current_piece->kind == PIECE_EMPTY || cpos_eq(current, from))) {
                continue;
            }
            // Instead of expanding neighbors at each piece, we should expand a whole level at once, that way
            // we don't keep checking and not continuing from the same pieces over and over.
            // The problem with that, is that we don't want to go through walls. So we'd have to check
            // neighbors anyway to find one that is in visited. Is that faster than this? I think so.
            // eh, I'm not sure actually.
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
                for (int j = 0; j < visited_len; j++) {
                    if (cpos_eq(visited[j], neighbor)) {
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
                    piece_strength(neighbor_piece->kind) <= max_strength) {
                    visited[visited_len++] = neighbor;
                }
            }
        }
    }

    // Skip first target since it's the starting position.
    assert(visited_len <= max_targets);
    for (int t = 1; t < visited_len; t++) {
        targets[t - 1] = visited[t];
    }
    return visited_len - 1;
}


CommandSlice game_valid_commands(Arena *a, Game *game) {
    Arena *scratch = scratch_acquire();

    CommandArray *commands = NULL;
    // You can always end your turn.
    arr_push(a, commands, ((Command) {
            .kind = COMMAND_END_TURN,
            .piece_id = 0,
            .target = (CPos) {0, 0, 0},
    }));

    Activation *activation = &(game->turn.activations[game->turn.activation_i]);

    // Hardcoded for hex field small.
    for (int r = -4; r <= 4; r++) {
        for (int q = -4; q <= 4; q++) {
            CPos cpos = {q, r, -q - r};
            Piece *piece = board_at(game, cpos);
            if (piece->player != game->turn.player) {
                continue;
            }

            // Can't use any piece from a previous activation.
            bool piece_already_used = false;
            for (int i = 0; i < game->turn.activation_i; i++) {
                if (game->turn.activations[i].piece_id == piece->id) {
                    piece_already_used = true;
                    break;
                }
            }
            if (piece_already_used) {
                continue;
            }

            bool piece_can_move = true;
            bool piece_can_action = true;

            // Can only do orders that we haven't done on a piece from the current activation.
            if (activation->piece_id == piece->id) {
                // If piece_id is set we must have done something.
                assert(activation->order_i > 0);
                // We can only do any not already done orders.
                for (int i = 0; i < activation->order_i; i++) {
                    if (activation->orders[i].kind == ORDER_MOVE) {
                        piece_can_move = false;
                    } else if (activation->orders[i].kind == ORDER_VOLLEY) {
                        piece_can_action = false;
                    }
                }
            } else if (activation->piece_id != 0) {
                // Can only use a new piece if there are more activations left.
                if (game->turn.activation_i + 1 >= 2) {
                    continue;
                }
            }

            if (piece->kind == PIECE_PIKE || piece->kind == PIECE_HORSE) {
                piece_can_action = false;
            }

            if (piece_can_move) {
                CPos targets[64];
                int targets_len = move_targets(&(targets[0]), 64, game, cpos);
                for (int i = 0; i < targets_len; i++) {
                    CPos target = targets[i];
                    arr_push(a, commands, ((Command) {
                            .kind = COMMAND_MOVE,
                            .piece_id = piece->id,
                            .target = target,
                    }));
                }
            }

            if (piece_can_action) {
                if (piece->kind == PIECE_BOW) {
                    CPosSlice targets = volley_targets(scratch, game, cpos);
                    for (uint64_t i = 0; i < targets.len; i++) {
                        CPos target = targets.e[i];
                        arr_push(a, commands, ((Command) {
                                .kind = COMMAND_VOLLEY,
                                .piece_id = piece->id,
                                .target = target,
                        }));
                    }
                }
            }
        }
    }

    scratch_release();
    return (CommandSlice) arr_slice(commands);
}

void game_apply_command(Game *game, Player player, Command command, VolleyResult volley_result) {
    if (command.kind == COMMAND_NONE) {
        return;
    }

    if (game->turn.player != player) {
        printf("Not %s's turn\n", player == PLAYER_RED ? "Red" : "Blue");
        return;
    }

    if (command.kind == COMMAND_END_TURN) {
        game->turn.activation_i = 2;
        goto end_turn;
    }

    FindPieceResult found_piece = find_piece(game, command.piece_id);
    if (!found_piece.found) {
        printf("No piece %d\n", command.piece_id);
        return;
    }
    Piece *piece = board_at(game, found_piece.cpos);

    bool piece_can_move = true;
    bool piece_can_action = true;
    bool increment_activation = false;
    Activation *activation = &(game->turn.activations[game->turn.activation_i]);
    if (activation->piece_id != 0 && activation->piece_id == piece->id) {
        assert(activation->order_i > 0);
        assert(activation->order_i < 2);
        // We can only do any not already done actions.
        for (int i = 0; i < activation->order_i; i++) {
            if (activation->orders[i].kind == ORDER_MOVE) {
                piece_can_move = false;
            } else if (activation->orders[i].kind == ORDER_VOLLEY) {
                piece_can_action = false;
            }
        }
    } else if (activation->piece_id != 0 && activation->piece_id != piece->id) {
        // We didn't finish the previous activation but are ordering a new piece.
        // Check if we have any activations left.
        if (game->turn.activation_i + 1 >= 2) {
            printf("No more activations left.\n");
            return;
        }
        increment_activation = true;
    } else if (activation->piece_id == 0) {
        assert(activation->order_i == 0);
        // This is a new activation, can move or action.
    }

    Piece *target_piece = board_at(game, command.target);

    OrderKind order_kind = ORDER_NONE;
    int aquired_gold = 0;
    Piece new_piece = *piece;
    Piece new_target_piece = *target_piece;

    // Note: Not validating the commands right now, just assuming they are valid.
    switch (command.kind) {
        case COMMAND_MOVE: {
            if (!piece_can_move) {
                printf("Can't move\n");
                return;
            }
            order_kind = ORDER_MOVE;
            aquired_gold = piece_gold(target_piece->kind);
            new_piece = (Piece) {.kind = PIECE_EMPTY, .player = PLAYER_NONE, .id = 0};

            if (piece->kind == PIECE_HORSE && piece_strength(target_piece->kind) >= piece_strength(piece->kind)) {
                // Horse charge, both die.
                new_target_piece = (Piece) {.kind = PIECE_EMPTY, .player = PLAYER_NONE, .id = 0};
            } else {
                new_target_piece = *piece;
            }
            break;
        }
        case COMMAND_VOLLEY: {
            // todo: for now always hits
            if (!piece_can_action) {
                printf("Can't action\n");
                return;
            }
            order_kind = ORDER_VOLLEY;
            bool volley_hits;
            switch (volley_result) {
                case VOLLEY_HIT: {
                    volley_hits = true;
                    break;
                }
                case VOLLEY_MISS: {
                    volley_hits = false;
                    break;
                }
                case VOLLEY_ROLL: {
                    int die_1 = 1 + rand() / (RAND_MAX / (6 - 1 + 1) + 1);
                    int die_2 = 1 + rand() / (RAND_MAX / (6 - 1 + 1) + 1);
                    int roll = die_1 + die_2;
                    volley_hits = roll < 7;
                    break;
                }
            }
            if (volley_hits) {
                aquired_gold = piece_gold(target_piece->kind);
                new_target_piece = (Piece) {.kind = PIECE_EMPTY, .player = PLAYER_NONE, .id = 0};
            }
            break;
        }
        default: {
            break;
        }
    }

    if (increment_activation) {
        game->turn.activation_i++;
    }

    // Update Game.
    activation = &(game->turn.activations[game->turn.activation_i]);
    activation->piece_id = piece->id;
    activation->orders[activation->order_i].kind = order_kind;
    activation->orders[activation->order_i].target = command.target;
    activation->order_i++;

    if (activation->order_i >= 2 || piece->kind != PIECE_BOW) {
        game->turn.activation_i++;
    }

    game->gold[game->turn.player] += aquired_gold;
    *piece = new_piece;
    *target_piece = new_target_piece;

    end_turn:
    if (game->turn.activation_i >= 2) {
        // Turn is over.
        game->turn.player = game->turn.player == PLAYER_RED ? PLAYER_BLUE : PLAYER_RED;
        for (int i = 0; i < 2; i++) {
            game->turn.activations[i].piece_id = 0;
            for (int ii = 0; ii < 2; ii++) {
                game->turn.activations[i].orders[ii].kind = ORDER_NONE;
                game->turn.activations[i].orders[ii].target = (CPos) {0, 0, 0};
            }
            game->turn.activations[i].order_i = 0;
        }
        game->turn.activation_i = 0;
    }

    // Check for game over.
    int red_crowns = 0;
    int blue_crowns = 0;
    for (int r = -4; r <= 4; r++) {
        for (int q = -4; q <= 4; q++) {
            CPos cpos = {q, r, -q - r};
            Piece *p = board_at(game, cpos);
            if (p->kind == PIECE_CROWN) {
                if (p->player == PLAYER_RED) {
                    red_crowns++;
                } else if (p->player == PLAYER_BLUE) {
                    blue_crowns++;
                }
            }
        }
    }
    if (red_crowns == 0) {
        game->status = STATUS_OVER;
        game->winner = PLAYER_BLUE;
    } else if (blue_crowns == 0) {
        game->status = STATUS_OVER;
        game->winner = PLAYER_RED;
    }
}
