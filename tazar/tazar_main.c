// well, now for sharing reasons I think it'd be nice to do this via a web interface.
// Then I could host it on val.town and the bros could play it directly.
// In that case I'd prolly wanna write just the core logic in c and build the ui in javascript.
// The AI turn could be done serverside, which makes it "async" and won't block the UI.
// It could still be stateless, where the game state is passed around each time, that's kind of nice
// since I don't have to deal with state, but you wouldn't be able to refresh the page that way.
// Saving gamestate and turns in a database the way gameplay.computer works would be better.
// That's more work that I don't super feel like doing, but also not a huge deal. I can just store
// a match id and use raw sqlite to get it done faster.
// Another alternative is do the whole thing in javascript. Def easier and faster, just less fun.


// Just have a single global id, and look stuff up by id, way easier.

// A really basic implementation of Tazar and an AI player, so I have someone to play with.
// https://kelleherbros.itch.io/tazar
// Been running this alongside and inputting all the moves into the real game, that's why it
// asks for the results of Volley rolls. Not really meant to be "played" standalone.

#include <math.h>
#include "tazar_game.c"

#define STEVE_IMPLEMENTATION
#include "steve.h"

#include "raylib.h"

#define MAX_GESTURE_STRINGS   20


typedef enum {
    UI_STATE_NONE,
    UI_STATE_WAITING_FOR_SELECTION,
    UI_STATE_WAITING_FOR_COMMAND,
    UI_STATE_ANIMATING_ACTION,
    // UI_STATE_WAITING_FOR_SHIELD_WALL_RESULT,
    UI_STATE_WAITING_FOR_OPPONENT_ACTION,
    UI_STATE_GAME_OVER
} UIState;

// During waiting for selection you can hover units to show actions
// During waiting for action you can still hover units to show their actions, but will need to
// show that in a different way.
// Animating will show the pieces moving.
// During waiting for opponent you can still hover to show I think. So I guess you should be able to hover during animations too.
// Need the visualization for hover to be different from the one for picking after selection.

int main(void) {
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

        // Process Input
        for (int row = 0; row <= 8; row++) {
            for (int col = 0; col <= 20; col++) {
                if ((row % 2 == 0 && col % 2 != 0) ||
                    (row % 2 == 1 && col % 2 != 1)) {
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
        bool keep_selected = false;
        if (mouse_clicked && mouse_in_end_turn_button) {
            game_apply_command(frame_arena, &game, game.turn.player, ((Command) {
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
                    if (command.piece_id == selected_piece_id && cpos_eq(command.target, mouse_cpos)) {

                        // hack, don't unselect a bow after shooting.
                        keep_selected = command.kind == COMMAND_VOLLEY &&
                                        (game.turn.activations[game.turn.activation_i].piece_id !=
                                         selected_piece_id ||
                                         game.turn.activations[game.turn.activation_i].order_i == 0);

                        game_apply_command(frame_arena, &game, game.turn.player, command);
                        // update commands
                        commands = game_valid_commands(frame_arena, &game);

                        if (!keep_selected) {
                            ui_state = UI_STATE_WAITING_FOR_SELECTION;
                            matched_command = true;
                        }
                        break;
                    }
                }
                if (!matched_command && !keep_selected) {
                    selected_piece_id = 0;
                    ui_state = UI_STATE_WAITING_FOR_SELECTION;
                }
            }
        }
        if (ui_state == UI_STATE_WAITING_FOR_SELECTION) {
            if (!keep_selected && mouse_clicked && mouse_in_board) {
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

        // ok, so I need just a couple more things.
        // a selected outline. and an end turn button.
        // and if there are no actions I can take except for end turn, I should auto end it.

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Actions List (todo)
        DrawRectangleLines(10, 29, 200, screen_height - 39, GRAY);
        DrawText("ACTIONS", 12, 15, 10, GRAY);

        // Game View
        DrawRectangleRec(game_area, DARKGRAY);

        // Left Panel
        DrawRectangleRec(left_panel, RAYWHITE);
        if (game.turn.player == PLAYER_RED) {
            DrawText("RED's TURN", game_area.x + 10, 18, 10, GRAY);
        } else {
            DrawText("BLUE's TURN", game_area.x + 10, 18, 10, GRAY);
        }

        // end turn button

        DrawRectangleRec(end_turn_button, LIGHTGRAY);
        DrawText("END TURN", game_area.x + 13, 41, 10, RAYWHITE);
        if (mouse_in_end_turn_button) {
            DrawRectangleLines(end_turn_button.x, end_turn_button.y, end_turn_button.width, end_turn_button.height,
                               PINK);
            DrawText("END TURN", game_area.x + 13, 41, 10, PINK);
        }


        // Right Panel
        DrawRectangleRec(right_panel, RAYWHITE);
        DrawText("OPTIONS", game_area.x + game_area.width - 100, 18, 10, GRAY);

        // Game Board
        // @note: Hardcoded to hex field small.
        for (int row = 0; row <= 8; row++) {
            for (int col = 0; col <= 20; col++) {
                if ((row % 2 == 0 && col % 2 != 0) ||
                    (row % 2 == 1 && col % 2 != 1)) {
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

                DrawPoly(screen_pos, 6, hex_radius,
                         90, LIGHTGRAY);
                DrawPolyLines(screen_pos, 6, hex_radius,
                              90, BLACK);
                Color color;
                if (piece.player == PLAYER_RED) {
                    color = RED;
                } else if (piece.player == PLAYER_BLUE) {
                    color = BLUE;
                } else {
                    color = PINK; // draw pink for invalid stuff, helps with debugging.
                }

                if (ui_state == UI_STATE_WAITING_FOR_COMMAND && piece.id == selected_piece_id) {
                    color = RAYWHITE;
                }

                switch (piece.kind) {
                    case PIECE_CROWN: {
                        DrawTriangle((Vector2) {screen_pos.x, screen_pos.y - hex_radius / 2},
                                     (Vector2) {screen_pos.x - hex_radius / 2, screen_pos.y + hex_radius / 4},
                                     (Vector2) {screen_pos.x + hex_radius / 2, screen_pos.y + hex_radius / 4}, color);
                        DrawTriangle((Vector2) {screen_pos.x - hex_radius / 2, screen_pos.y - hex_radius / 4},
                                     (Vector2) {screen_pos.x, screen_pos.y + hex_radius / 2},
                                     (Vector2) {screen_pos.x + hex_radius / 2, screen_pos.y - hex_radius / 4}, color);
                        break;
                    }
                    case PIECE_PIKE:
                        DrawRectangleV((Vector2) {screen_pos.x - hex_radius / 2, screen_pos.y - hex_radius / 2},
                                       (Vector2) {hex_radius, hex_radius},
                                       color);
                        break;
                    case PIECE_HORSE:
                        DrawTriangle((Vector2) {screen_pos.x, screen_pos.y - hex_radius / 2},
                                     (Vector2) {screen_pos.x - hex_radius / 2, screen_pos.y + hex_radius / 4},
                                     (Vector2) {screen_pos.x + hex_radius / 2, screen_pos.y + hex_radius / 4}, color);
                        break;
                    case PIECE_BOW:
                        DrawCircleV(screen_pos, hex_radius / 2, color);
                        break;
                    default:
                        break;
                }
            }
        }
        if (ui_state == UI_STATE_WAITING_FOR_COMMAND) {
            for (uint64_t i = 0; i < commands.len; i++) {
                Command command = commands.e[i];
                if (command.piece_id == selected_piece_id) {
                    DPos target_dpos = dpos_from_cpos(command.target);
                    Vector2 target_pos = (Vector2) {game_center.x + (horizontal_offset * target_dpos.x),
                                                    game_center.y + vertical_offset * target_dpos.y};
                    if (command.kind == COMMAND_MOVE) {
                        DrawPolyLinesEx(target_pos, 6, hex_radius - 1,
                                        90, 4, RAYWHITE);
                    } else if (command.kind == COMMAND_VOLLEY) {
                        DrawCircle(target_pos.x, target_pos.y, hex_radius / 4, RAYWHITE);
                    }
                }
            }
        }
        if (mouse_in_board) {
            Piece hovered_piece = *board_at(&game, mouse_cpos);
            if ((ui_state != UI_STATE_WAITING_FOR_COMMAND) &&
                !(hovered_piece.kind == PIECE_NONE || hovered_piece.kind == PIECE_EMPTY) &&
                hovered_piece.player == game.turn.player) {
                Color color = hovered_piece.player == PLAYER_RED ? MAROON : DARKBLUE;

                for (uint64_t i = 0; i < commands.len; i++) {
                    Command command = commands.e[i];
                    if (command.piece_id == hovered_piece.id) {
                        DPos target_dpos = dpos_from_cpos(command.target);
                        Vector2 target_pos = (Vector2) {game_center.x + (horizontal_offset * target_dpos.x),
                                                        game_center.y + vertical_offset * target_dpos.y};
                        if (command.kind == COMMAND_MOVE) {
                            DrawPolyLinesEx(target_pos, 6, hex_radius - 2,
                                            90, 2, color);
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
}

//int main(void) {
//    //return cli_main();
//    printf("Hello, World!\n");
//    return 0;
//}