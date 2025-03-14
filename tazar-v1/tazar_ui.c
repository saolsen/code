// A really basic implementation of Tazar and an AI player, so I have someone to play with.
// https://kelleherbros.itch.io/tazar
#include "tazar.h"

#include "raylib.h"
#include <math.h>
#include <assert.h>

typedef enum {
    UI_STATE_WAITING_FOR_SELECTION,
    UI_STATE_WAITING_FOR_COMMAND,
    UI_STATE_AI_TURN,
    UI_STATE_GAME_OVER,
} UIState;

typedef enum {
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD,
} Difficulty;

int ui_main(void) {
    Game game;
    game_init_attrition_hex_field_small(&game);

    const int screen_width = 1024;
    const int screen_height = 768;

    Rectangle game_area = {280, 20, (float) screen_width - 300.0f, (float) screen_height - 40.0f};
    Vector2 game_center = {game_area.x + game_area.width / 2, game_area.y + game_area.height / 2};

    Rectangle left_panel = {game_area.x + 10, game_area.y + 10, 150, 200};
    Rectangle right_panel = {game_area.x + game_area.width - 160, game_area.y + 10, 150, 200};
    Rectangle end_turn_button = {game_area.x + 15, 72, 135, 30};

    Rectangle diff_easy_button = {right_panel.x + 5, right_panel.y + 40, 135, 25};
    Rectangle diff_medium_button = {right_panel.x + 5, right_panel.y + 70, 135, 25};
    Rectangle diff_hard_button = {right_panel.x + 5, right_panel.y + 100, 135, 25};

    float hexes_across = 12.0f;
    float hex_radius = floorf(game_area.width * sqrtf(3.0f) / (3 * hexes_across));
    float horizontal_offset = sqrtf(hex_radius * hex_radius - (hex_radius / 2) * (hex_radius / 2));
    float vertical_offset = hex_radius * 1.5f;
    DPos screen_center = {10, 4};

    InitWindow(screen_width, screen_height, "Tazar Bot");

    SetTargetFPS(60);
    SetExitKey(0);

    UIState ui_state = UI_STATE_WAITING_FOR_SELECTION;
    int selected_piece_id = 0;

    int num_ai_turn_lag_frames = 45;
    int ai_turn_lag_frames_left = 0;
    int num_ai_thinking_frames = 60 * 5;
    int num_ai_thinking_frames_left = 0;
    bool ai_thinking = false;
    Command chosen_ai_command = {0};

    MCState mc_state = {0};
    MCTSState mcts_state = {0};

    Difficulty current_difficulty = DIFFICULTY_EASY;

    Command commands[1024];
    int num_commands = 0;

    while (!WindowShouldClose()) {

        num_commands = game_valid_commands(&commands[0], 1024, &game);
        assert(num_commands <= 1024);

        bool mouse_in_game_area = CheckCollisionPointRec(GetMousePosition(), game_area) &&
                                  !(CheckCollisionPointRec(GetMousePosition(), left_panel) ||
                                    CheckCollisionPointRec(GetMousePosition(), right_panel));
        bool mouse_in_end_turn_button = CheckCollisionPointRec(GetMousePosition(), end_turn_button);
        bool mouse_in_board = false;
        CPos mouse_cpos = {0, 0, 0};
        bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        bool mouse_in_easy = CheckCollisionPointRec(GetMousePosition(), diff_easy_button);
        bool mouse_in_medium = CheckCollisionPointRec(GetMousePosition(), diff_medium_button);
        bool mouse_in_hard = CheckCollisionPointRec(GetMousePosition(), diff_hard_button);

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
                                              (Vector2) {game_center.x + horizontal_offset * (float) dpos.x,
                                                         game_center.y + vertical_offset * (float) dpos.y},
                                              horizontal_offset)) {
                    mouse_in_board = true;
                    mouse_cpos = cpos;
                }
            }
        }

        if (mouse_clicked && ui_state != UI_STATE_AI_TURN) {
            if (mouse_in_easy)
                current_difficulty = DIFFICULTY_EASY;
            if (mouse_in_medium)
                current_difficulty = DIFFICULTY_MEDIUM;
            if (mouse_in_hard)
                current_difficulty = DIFFICULTY_HARD;
        }

        // Update
        if (mouse_clicked && mouse_in_end_turn_button) {
            game_apply_command(&game, game.turn.player,
                               ((Command) {
                                       .kind = COMMAND_END_TURN,
                                       .piece_id = 0,
                                       .target = (CPos) {0, 0, 0},
                               }), VOLLEY_ROLL);
            num_commands = game_valid_commands(&commands[0], 1024, &game);
            assert(num_commands <= 1024);
            ui_state = UI_STATE_WAITING_FOR_SELECTION;
        }
        if (ui_state == UI_STATE_WAITING_FOR_COMMAND) {
            if (mouse_clicked && mouse_in_game_area && !mouse_in_board) {
                selected_piece_id = 0;
                ui_state = UI_STATE_WAITING_FOR_SELECTION;
            } else if (mouse_clicked && mouse_in_board) {
                bool matched_command = false;
                for (int i = 0; i < num_commands; i++) {
                    Command command = commands[i];
                    if (command.piece_id == selected_piece_id &&
                        cpos_eq(command.target, mouse_cpos)) {
                        matched_command = true;
                        game_apply_command(&game, game.turn.player, command, VOLLEY_ROLL);
                        num_commands = game_valid_commands(&commands[0], 1024, &game);
                        assert(num_commands <= 1024);
                        break;
                    }
                }
                if (matched_command) {
                    bool has_another_command = false;
                    for (int i = 0; i < num_commands; i++) {
                        Command command = commands[i];
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
                    if (num_commands == 1) {
                        assert(commands[0].kind == COMMAND_END_TURN);
                        game_apply_command(&game, game.turn.player,
                                           ((Command) {
                                                   .kind = COMMAND_END_TURN,
                                                   .piece_id = 0,
                                                   .target = (CPos) {0, 0, 0},
                                           }), VOLLEY_ROLL);
                        num_commands = game_valid_commands(&commands[0], 1024, &game);
                        assert(num_commands <= 1024);
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
                    for (int i = 0; i < num_commands; i++) {
                        Command command = commands[i];
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
        if (ui_state == UI_STATE_AI_TURN) {
            if (ai_thinking && num_ai_thinking_frames_left-- > 0) {
                switch (current_difficulty) {
                    case DIFFICULTY_MEDIUM:
                        ai_mc_think(&mc_state, &game, commands, num_commands, 10);
                        break;
                    case DIFFICULTY_HARD:
                        ai_mcts_think(&mcts_state, &game, commands, num_commands, 25);
                        break;
                    default:
                        break;
                }
            }

            if (ai_thinking && num_ai_thinking_frames_left <= 0) {
                switch (current_difficulty) {
                    case DIFFICULTY_EASY:
                        chosen_ai_command = ai_select_command_heuristic(&game, commands, num_commands);
                        break;
                    case DIFFICULTY_MEDIUM:
                        chosen_ai_command = ai_mc_select_command(&mc_state, &game, commands, num_commands);
                        ai_mc_state_cleanup(&mc_state);
                        break;
                    case DIFFICULTY_HARD:
                        chosen_ai_command = ai_mcts_select_command(&mcts_state, &game, commands, num_commands);
                        ai_mcts_state_cleanup(&mcts_state);
                        break;
                }
                ai_thinking = false;
                selected_piece_id = chosen_ai_command.piece_id;
                ai_turn_lag_frames_left = num_ai_turn_lag_frames;
            }

            if (!ai_thinking && ai_turn_lag_frames_left-- <= 0) {
                // Apply the command.
                game_apply_command(&game, game.turn.player, chosen_ai_command, VOLLEY_ROLL);
                num_commands = game_valid_commands(&commands[0], 1024, &game);
                assert(num_commands <= 1024);

                if (game.status == STATUS_OVER) {
                    ui_state = UI_STATE_GAME_OVER;
                } else {
                    selected_piece_id = 0;
                    ui_state = UI_STATE_WAITING_FOR_SELECTION;
                }
            }
        }

        if (ui_state != UI_STATE_AI_TURN && game.status != STATUS_OVER &&
            game.turn.player == PLAYER_BLUE) {
            chosen_ai_command = (Command) {0};

            // Initialize the AI state and start thinking.
            switch (current_difficulty) {
                case DIFFICULTY_EASY:
                    // Easy doesn't need any time to think.
                    num_ai_thinking_frames_left = 0;
                    break;
                case DIFFICULTY_MEDIUM:
                    mc_state = ai_mc_state_init(&game, commands, num_commands);
                    num_ai_thinking_frames_left = num_ai_thinking_frames;
                    break;
                case DIFFICULTY_HARD:
                    mcts_state = ai_mcts_state_init(&game, commands, num_commands);
                    num_ai_thinking_frames_left = num_ai_thinking_frames;
                    break;
            }
            ui_state = UI_STATE_AI_TURN;
            ai_thinking = true;
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Actions List (todo)
        DrawRectangleLines(20, 58, 240, screen_height - 78, GRAY);

        // AI Progress Bar
        if (ui_state == UI_STATE_AI_TURN && ai_thinking) {
            float progress = 1.0f - ((float) num_ai_thinking_frames_left / (float) num_ai_thinking_frames);
            Rectangle progress_bar_bg = {24, 100, 232, 20};
            Rectangle progress_bar = {24, 100, 232 * progress, 20};

            DrawRectangleRec(progress_bar_bg, LIGHTGRAY);
            DrawRectangleRec(progress_bar, BLUE);
            DrawRectangleLines((int) progress_bar_bg.x, (int) progress_bar_bg.y, (int) progress_bar_bg.width,
                               (int) progress_bar_bg.height,
                               GRAY);
            DrawText("AI THINKING...", 24, 125, 19, GRAY);
        }


        //DrawText("ACTIONS", 24, 30, 19, GRAY);
        if (ui_state == UI_STATE_WAITING_FOR_COMMAND) {
            DrawText("SELECT COMMAND", 24, 70, 19, GRAY);
        } else if (ui_state == UI_STATE_WAITING_FOR_SELECTION) {
            DrawText("SELECT PIECE", 24, 70, 19, GRAY);
        }

        // Game View
        DrawRectangleRec(game_area, DARKGRAY);

        // Left Panel
        DrawRectangleRec(left_panel, RAYWHITE);

        if (game.winner != PLAYER_NONE) {
            DrawText("GAME OVER", (int) (game_area.x + 20), 36, 19, GRAY);
            if (game.winner == PLAYER_RED) {
                DrawText("RED WINS", (int) (game_area.x + 20), 82, 19, GRAY);
            } else {
                DrawText("BLUE WINS", (int) (game_area.x + 20), 82, 19, GRAY);
            }
        } else if (game.turn.player == PLAYER_RED) {
            DrawText("RED's TURN", (int) (game_area.x + 20), 36, 19, GRAY);
        } else {
            DrawText("BLUE's TURN", (int) (game_area.x + 20), 36, 19, GRAY);
        }

        // end turn button

        if (ui_state == UI_STATE_WAITING_FOR_COMMAND ||
            ui_state == UI_STATE_WAITING_FOR_SELECTION) {
            DrawRectangleRec(end_turn_button, LIGHTGRAY);
            DrawText("END TURN", (int) (game_area.x + 26), 82, 19, RAYWHITE);
            if (mouse_in_end_turn_button) {
                DrawRectangleLines((int) end_turn_button.x, (int) end_turn_button.y, (int) end_turn_button.width,
                                   (int) end_turn_button.height, PINK);
                DrawText("END TURN", (int) (game_area.x + 26), 82, 19, PINK);
            }
        }

        // Right Panel
        DrawRectangleRec(right_panel, RAYWHITE);
        DrawText("DIFFICULTY", (int) (game_area.x + game_area.width - 150), 36, 19, GRAY);

        // Difficulty buttons
        Color easy_color = (current_difficulty == DIFFICULTY_EASY) ? DARKGRAY : LIGHTGRAY;
        Color medium_color = (current_difficulty == DIFFICULTY_MEDIUM) ? DARKGRAY : LIGHTGRAY;
        Color hard_color = (current_difficulty == DIFFICULTY_HARD) ? DARKGRAY : LIGHTGRAY;

        DrawRectangleRec(diff_easy_button, easy_color);
        DrawRectangleRec(diff_medium_button, medium_color);
        DrawRectangleRec(diff_hard_button, hard_color);

        DrawText("Easy", (int) (diff_easy_button.x + 10), (int) (diff_easy_button.y + 5), 19, RAYWHITE);
        DrawText("Medium", (int) (diff_medium_button.x + 10), (int) (diff_medium_button.y + 5), 19, RAYWHITE);
        DrawText("Hard", (int) (diff_hard_button.x + 10), (int) (diff_hard_button.y + 5), 19, RAYWHITE);

        if (mouse_in_easy)
            DrawRectangleLines((int) diff_easy_button.x, (int) diff_easy_button.y, (int) diff_easy_button.width,
                               (int) diff_easy_button.height, PINK);
        if (mouse_in_medium)
            DrawRectangleLines((int) diff_medium_button.x, (int) diff_medium_button.y, (int) diff_medium_button.width,
                               (int) diff_medium_button.height, PINK);
        if (mouse_in_hard)
            DrawRectangleLines((int) diff_hard_button.x, (int) diff_hard_button.y, (int) diff_hard_button.width,
                               (int) diff_hard_button.height, PINK);

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

                Vector2 screen_pos = (Vector2) {game_center.x + (horizontal_offset * (float) dpos.x),
                                                game_center.y + vertical_offset * (float) dpos.y};

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

                if (/*ui_state == UI_STATE_WAITING_FOR_COMMAND && */ piece.id ==
                                                                     selected_piece_id) {
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

        for (int i = 0; i < num_commands; i++) {
            Command command = commands[i];
            if (command.piece_id == selected_piece_id) {
                DPos target_dpos = dpos_from_cpos(command.target);
                Vector2 target_pos = (Vector2) {game_center.x + (horizontal_offset * (float) target_dpos.x),
                                                game_center.y + vertical_offset * (float) target_dpos.y};
                if (command.kind == COMMAND_MOVE) {
                    DrawPolyLinesEx(target_pos, 6, hex_radius - 1, 90, 4, RAYWHITE);
                } else if (command.kind == COMMAND_VOLLEY) {
                    DrawCircle((int) target_pos.x, (int) target_pos.y, hex_radius / 4, RAYWHITE);
                }
            }
        }

        if (mouse_in_board && ui_state != UI_STATE_GAME_OVER && ui_state != UI_STATE_AI_TURN) {
            Piece hovered_piece = *board_at(&game, mouse_cpos);
            if (!(ui_state == UI_STATE_WAITING_FOR_COMMAND &&
                  hovered_piece.id == selected_piece_id) &&
                !(hovered_piece.kind == PIECE_NONE || hovered_piece.kind == PIECE_EMPTY) &&
                hovered_piece.player == game.turn.player) {
                Color color = hovered_piece.player == PLAYER_RED ? MAROON : DARKBLUE;

                for (int i = 0; i < num_commands; i++) {
                    Command command = commands[i];
                    if (command.piece_id == hovered_piece.id) {
                        DPos target_dpos = dpos_from_cpos(command.target);
                        Vector2 target_pos =
                                (Vector2) {game_center.x + (horizontal_offset * (float) target_dpos.x),
                                           game_center.y + vertical_offset * (float) target_dpos.y};
                        if (command.kind == COMMAND_MOVE) {
                            DrawPolyLinesEx(target_pos, 6, hex_radius - 2, 90, 2, color);
                        } else if (command.kind == COMMAND_VOLLEY) {
                            DrawCircle((int) target_pos.x, (int) target_pos.y, hex_radius / 4, color);
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
