// A really basic implementation of Tazar and an AI player, so I have someone to play with.
// https://kelleherbros.itch.io/tazar
// Been running this alongside and inputting all the moves into the real game, that's why it
// asks for the results of Volley rolls. Not really meant to be "played" standalone.
#include "tazar.h"

#include "raylib.h"
#include <math.h>

typedef enum {
    UI_STATE_NONE,
    UI_STATE_WAITING_FOR_SELECTION,
    UI_STATE_WAITING_FOR_COMMAND,
    UI_STATE_AI_TURN,
    UI_STATE_GAME_OVER,
} UIState;

int ui_main(void) {
    Game game;
    game_init_attrition_hex_field_small(&game);

    const int screen_width = 800;
    const int screen_height = 450;

    Rectangle game_area = {220, 10, (float) screen_width - 230.0f, (float) screen_height - 20.0f};
    Vector2 game_center = {game_area.x + game_area.width / 2, game_area.y + game_area.height / 2};

    Rectangle left_panel = {game_area.x + 5, game_area.y + 5, 100, 115};
    Rectangle right_panel = {game_area.x + game_area.width - 105, game_area.y + 5, 100, 115};
    Rectangle end_turn_button = {game_area.x + 10, 36, 60, 20};

    float hexes_across = 12.0f;
    float hex_radius = floorf(game_area.width * sqrtf(3.0f) / (3 * hexes_across));
    float horizontal_offset = sqrtf(hex_radius * hex_radius - (hex_radius / 2) * (hex_radius / 2));
    float vertical_offset = hex_radius * 1.5f;
    DPos screen_center = {10, 4};

    InitWindow(screen_width, screen_height, "Tazar Bot");

    SetTargetFPS(60);
    SetExitKey(0);

    Arena *frame_arena = arena_new();

    UIState ui_state = UI_STATE_WAITING_FOR_SELECTION;
    int selected_piece_id = 0;

    int num_ai_turn_lag_frames = 30;
    int ai_turn_lag_frames_left = 0;
    Command chosen_ai_command = {0};

    while (!WindowShouldClose()) {
        arena_reset(frame_arena);

        CommandSlice commands = game_valid_commands(frame_arena, &game);

        bool mouse_in_game_area = CheckCollisionPointRec(GetMousePosition(), game_area) &&
                                  !(CheckCollisionPointRec(GetMousePosition(), left_panel) ||
                                    CheckCollisionPointRec(GetMousePosition(), right_panel));
        bool mouse_in_end_turn_button = CheckCollisionPointRec(GetMousePosition(), end_turn_button);
        bool mouse_in_board = false;
        CPos mouse_cpos = {0, 0, 0};
        bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (game.status == STATUS_OVER) {
            ui_state = UI_STATE_GAME_OVER;
        }

        // Process Input
        for (int row = 0; row <= 8; row++) {
            for (int col = 0; col <= 20; col++) {
                if ((row % 2 == 0 && col % 2 != 0) || (row % 2 == 1 && col % 2 != 1)) {
                    // Not a valid double coordinate.
                    continue;
                }
                DPos dpos = {col - screen_center.x, row - screen_center.y};
                CPos cpos = cpos_from_dpos(dpos);
                Piece piece = *board_at(&game, cpos);
                if (piece.kind == PIECE_NONE) {
                    continue;
                }
                // Check Mouse Position.
                if (CheckCollisionPointCircle(GetMousePosition(),
                                              (Vector2) {game_center.x + horizontal_offset * dpos.x,
                                                         game_center.y + vertical_offset * dpos.y},
                                              horizontal_offset)) {
                    mouse_in_board = true;
                    mouse_cpos = cpos;
                }
            }
        }

        // Update
        if (mouse_clicked && mouse_in_end_turn_button) {
            game_apply_command(frame_arena, &game, game.turn.player,
                               ((Command) {
                                       .kind = COMMAND_END_TURN,
                                       .piece_id = 0,
                                       .target = (CPos) {0, 0, 0},
                               }));
            commands = game_valid_commands(frame_arena, &game);
            ui_state = UI_STATE_WAITING_FOR_SELECTION;
        }
        if (ui_state == UI_STATE_WAITING_FOR_COMMAND) {
            if (mouse_clicked && mouse_in_game_area && !mouse_in_board) {
                selected_piece_id = 0;
                ui_state = UI_STATE_WAITING_FOR_SELECTION;
            } else if (mouse_clicked && mouse_in_board) {
                bool matched_command = false;
                for (uint64_t i = 0; i < commands.len; i++) {
                    Command command = commands.e[i];
                    if (command.piece_id == selected_piece_id &&
                        cpos_eq(command.target, mouse_cpos)) {
                        matched_command = true;
                        game_apply_command(frame_arena, &game, game.turn.player, command);
                        commands = game_valid_commands(frame_arena, &game);
                        break;
                    }
                }
                if (matched_command) {
                    bool has_another_command = false;
                    for (uint64_t i = 0; i < commands.len; i++) {
                        Command command = commands.e[i];
                        if (command.piece_id == selected_piece_id) {
                            has_another_command = true;
                            break;
                        }
                    }
                    if (!has_another_command) {
                        selected_piece_id = 0;
                        ui_state = UI_STATE_WAITING_FOR_SELECTION;
                    }
                    // If there are no more commands for this player, end the turn.
                    if (commands.len == 1) {
                        assert(commands.e[0].kind == COMMAND_END_TURN);
                        game_apply_command(frame_arena, &game, game.turn.player,
                                           ((Command) {
                                                   .kind = COMMAND_END_TURN,
                                                   .piece_id = 0,
                                                   .target = (CPos) {0, 0, 0},
                                           }));
                        commands = game_valid_commands(frame_arena, &game);
                        ui_state = UI_STATE_WAITING_FOR_SELECTION;
                    }
                } else {
                    selected_piece_id = 0;
                    ui_state = UI_STATE_WAITING_FOR_SELECTION;
                }
            }
        } else if (ui_state == UI_STATE_WAITING_FOR_SELECTION) {
            if (mouse_clicked && mouse_in_board) {
                Piece selected_piece = *board_at(&game, mouse_cpos);
                if (selected_piece.player == game.turn.player) {
                    for (uint64_t i = 0; i < commands.len; i++) {
                        Command command = commands.e[i];
                        if (command.piece_id == selected_piece.id) {
                            selected_piece_id = selected_piece.id;
                            ui_state = UI_STATE_WAITING_FOR_COMMAND;
                            break;
                        }
                    }
                }
            }
        }

        // AI Turn
        if (ui_state != UI_STATE_AI_TURN && game.status != STATUS_OVER && game.turn.player == PLAYER_BLUE) {
            ui_state = UI_STATE_AI_TURN;
            ai_turn_lag_frames_left = 0;
            chosen_ai_command = (Command) {0};
        }

        if (ui_state == UI_STATE_AI_TURN && ai_turn_lag_frames_left-- <= 0) {
            // Apply the command.
            if (chosen_ai_command.kind != COMMAND_NONE) {
                game_apply_command(frame_arena, &game, game.turn.player, chosen_ai_command);
                commands = game_valid_commands(frame_arena, &game);
            }

            if (game.status == STATUS_OVER) {
                ui_state = UI_STATE_GAME_OVER;
            } else {
                if (game.turn.player == PLAYER_BLUE) {
                    // First hacky ai, just pick a random command.
                    int ai_command_selected = GetRandomValue(0, commands.len - 1);
                    chosen_ai_command = commands.e[ai_command_selected];
                    selected_piece_id = chosen_ai_command.piece_id;
                    ai_turn_lag_frames_left = num_ai_turn_lag_frames;
                } else {
                    selected_piece_id = 0;
                    ui_state = UI_STATE_WAITING_FOR_SELECTION;
                }
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Actions List (todo)
        DrawRectangleLines(10, 29, 200, screen_height - 39, GRAY);
        DrawText("ACTIONS", 12, 15, 10, GRAY);
        if (ui_state == UI_STATE_WAITING_FOR_COMMAND) {
            DrawText("SELECT COMMAND", 12, 35, 10, GRAY);
        } else if (ui_state == UI_STATE_WAITING_FOR_SELECTION) {
            DrawText("SELECT PIECE", 12, 35, 10, GRAY);
        }

        // Game View
        DrawRectangleRec(game_area, DARKGRAY);

        // Left Panel
        DrawRectangleRec(left_panel, RAYWHITE);

        if (game.winner != PLAYER_NONE) {
            DrawText("GAME OVER", game_area.x + 10, 18, 10, GRAY);
            if (game.winner == PLAYER_RED) {
                DrawText("RED WINS", game_area.x + 10, 41, 10, GRAY);
            } else {
                DrawText("BLUE WINS", game_area.x + 10, 41, 10, GRAY);
            }
        } else if (game.turn.player == PLAYER_RED) {
            DrawText("RED's TURN", game_area.x + 10, 18, 10, GRAY);
        } else {
            DrawText("BLUE's TURN", game_area.x + 10, 18, 10, GRAY);
        }

        // end turn button

        if (ui_state == UI_STATE_WAITING_FOR_COMMAND || ui_state == UI_STATE_WAITING_FOR_SELECTION) {
            DrawRectangleRec(end_turn_button, LIGHTGRAY);
            DrawText("END TURN", game_area.x + 13, 41, 10, RAYWHITE);
            if (mouse_in_end_turn_button) {
                DrawRectangleLines(end_turn_button.x, end_turn_button.y, end_turn_button.width,
                                   end_turn_button.height, PINK);
                DrawText("END TURN", game_area.x + 13, 41, 10, PINK);
            }
        }

        // Right Panel
        DrawRectangleRec(right_panel, RAYWHITE);
        DrawText("OPTIONS", game_area.x + game_area.width - 100, 18, 10, GRAY);

        // Game Board
        // @note: Hardcoded to hex field small.
        for (int row = 0; row <= 8; row++) {
            for (int col = 0; col <= 20; col++) {
                if ((row % 2 == 0 && col % 2 != 0) || (row % 2 == 1 && col % 2 != 1)) {
                    // Not a valid double coordinate.
                    continue;
                }

                DPos dpos = {col - screen_center.x, row - screen_center.y};
                CPos cpos = cpos_from_dpos(dpos);
                Piece piece = *board_at(&game, cpos);
                if (piece.kind == PIECE_NONE) {
                    continue;
                }

                Vector2 screen_pos = (Vector2) {game_center.x + (horizontal_offset * dpos.x),
                                                game_center.y + vertical_offset * dpos.y};

                DrawPoly(screen_pos, 6, hex_radius, 90, LIGHTGRAY);
                DrawPolyLines(screen_pos, 6, hex_radius, 90, BLACK);
                Color color;
                if (piece.player == PLAYER_RED) {
                    color = RED;
                } else if (piece.player == PLAYER_BLUE) {
                    color = BLUE;
                } else {
                    color = PINK; // draw pink for invalid stuff, helps with debugging.
                }

                if (/*ui_state == UI_STATE_WAITING_FOR_COMMAND && */piece.id == selected_piece_id) {
                    color = RAYWHITE;
                }

                switch (piece.kind) {
                    case PIECE_CROWN: {
                        DrawTriangle(
                                (Vector2) {screen_pos.x, screen_pos.y - hex_radius / 2},
                                (Vector2) {screen_pos.x - hex_radius / 2, screen_pos.y + hex_radius / 4},
                                (Vector2) {screen_pos.x + hex_radius / 2, screen_pos.y + hex_radius / 4},
                                color);
                        DrawTriangle(
                                (Vector2) {screen_pos.x - hex_radius / 2, screen_pos.y - hex_radius / 4},
                                (Vector2) {screen_pos.x, screen_pos.y + hex_radius / 2},
                                (Vector2) {screen_pos.x + hex_radius / 2, screen_pos.y - hex_radius / 4},
                                color);
                        break;
                    }
                    case PIECE_PIKE:
                        DrawRectangleV(
                                (Vector2) {screen_pos.x - hex_radius / 2, screen_pos.y - hex_radius / 2},
                                (Vector2) {hex_radius, hex_radius}, color);
                        break;
                    case PIECE_HORSE:
                        DrawTriangle(
                                (Vector2) {screen_pos.x, screen_pos.y - hex_radius / 2},
                                (Vector2) {screen_pos.x - hex_radius / 2, screen_pos.y + hex_radius / 4},
                                (Vector2) {screen_pos.x + hex_radius / 2, screen_pos.y + hex_radius / 4},
                                color);
                        break;
                    case PIECE_BOW:
                        DrawCircleV(screen_pos, hex_radius / 2, color);
                        break;
                    default:
                        break;
                }
            }
        }
        //if (ui_state == UI_STATE_WAITING_FOR_COMMAND) {
        for (uint64_t i = 0; i < commands.len; i++) {
            Command command = commands.e[i];
            if (command.piece_id == selected_piece_id) {
                DPos target_dpos = dpos_from_cpos(command.target);
                Vector2 target_pos =
                        (Vector2) {game_center.x + (horizontal_offset * target_dpos.x),
                                   game_center.y + vertical_offset * target_dpos.y};
                if (command.kind == COMMAND_MOVE) {
                    DrawPolyLinesEx(target_pos, 6, hex_radius - 1, 90, 4, RAYWHITE);
                } else if (command.kind == COMMAND_VOLLEY) {
                    DrawCircle(target_pos.x, target_pos.y, hex_radius / 4, RAYWHITE);
                }
            }
        }
        //}
        if (mouse_in_board && ui_state != UI_STATE_GAME_OVER && ui_state != UI_STATE_AI_TURN) {
            Piece hovered_piece = *board_at(&game, mouse_cpos);
            if (!(ui_state == UI_STATE_WAITING_FOR_COMMAND && hovered_piece.id == selected_piece_id) &&
                !(hovered_piece.kind == PIECE_NONE || hovered_piece.kind == PIECE_EMPTY) &&
                hovered_piece.player == game.turn.player) {
                Color color = hovered_piece.player == PLAYER_RED ? MAROON : DARKBLUE;

                for (uint64_t i = 0; i < commands.len; i++) {
                    Command command = commands.e[i];
                    if (command.piece_id == hovered_piece.id) {
                        DPos target_dpos = dpos_from_cpos(command.target);
                        Vector2 target_pos =
                                (Vector2) {game_center.x + (horizontal_offset * target_dpos.x),
                                           game_center.y + vertical_offset * target_dpos.y};
                        if (command.kind == COMMAND_MOVE) {
                            DrawPolyLinesEx(target_pos, 6, hex_radius - 2, 90, 2, color);
                        } else if (command.kind == COMMAND_VOLLEY) {
                            DrawCircle(target_pos.x, target_pos.y, hex_radius / 4, color);
                        }
                    }
                }
            }
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
