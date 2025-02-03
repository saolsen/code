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

// A really basic implementation of Tazar and an AI player, so I have someone to play with.
// https://kelleherbros.itch.io/tazar
// Been running this alongside and inputting all the moves into the real game, that's why it
// asks for the results of Volley rolls. Not really meant to be "played" standalone.

#include <math.h>
#include "tazar_game.c"
#include "tazar_parse.c"
//#include "tazar_cli.c"

#define STEVE_IMPLEMENTATION
#include "steve.h"

#include "raylib.h"

#define MAX_GESTURE_STRINGS   20


// The UI interactions are making this hard. Obviously need to separate the update and input stuff from the drawing.
typedef enum {
    UI_STATE_NONE,
    UI_STATE_WAITING_FOR_SELECTION,
    UI_STATE_WAITING_FOR_ACTION,
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

    while (!WindowShouldClose()) {
        arena_reset(frame_arena);

        ActionSlice red_actions = game_valid_actions(frame_arena, &game, PLAYER_RED);
        ActionSlice blue_actions = game_valid_actions(frame_arena, &game, PLAYER_BLUE);

        bool mouse_in_game_area = CheckCollisionPointRec(GetMousePosition(), game_area) &&
                                  !(CheckCollisionPointRec(GetMousePosition(), left_panel) ||
                                    CheckCollisionPointRec(GetMousePosition(), right_panel));
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
                Vector2 screen_pos = (Vector2) {game_center.x + (horizontal_offset * dpos.x),
                                                game_center.y + vertical_offset * dpos.y};
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
        if (ui_state == UI_STATE_WAITING_FOR_SELECTION) {
            if (mouse_clicked && mouse_in_board) {
                Piece selected_piece = *board_at(&game, mouse_cpos);
                if (selected_piece.player == game.active_player) {
                    game.active_piece = selected_piece;
                    ui_state = UI_STATE_WAITING_FOR_ACTION;
                }
            }
        } else if (ui_state == UI_STATE_WAITING_FOR_ACTION) {
            if (mouse_clicked && mouse_in_game_area && !mouse_in_board) {
                game.active_piece = piece_null;
                ui_state = UI_STATE_WAITING_FOR_SELECTION;
            }
        }

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
        DrawText("BLUE TURN", game_area.x + 10, 18, 10, GRAY);

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
        if (ui_state == UI_STATE_WAITING_FOR_ACTION) {
            Piece selected_piece = game.active_piece;
            ActionSlice actions = selected_piece.player == PLAYER_RED ? red_actions : blue_actions;
            for (uint64_t i = 0; i < actions.len; i++) {
                Action action = actions.e[i];
                if (action.piece == selected_piece.kind && action.piece_id == selected_piece.id) {
                    DPos target_dpos = dpos_from_cpos(action.target);
                    Vector2 target_pos = (Vector2) {game_center.x + (horizontal_offset * target_dpos.x),
                                                    game_center.y + vertical_offset * target_dpos.y};
                    DrawPolyLinesEx(target_pos, 6, hex_radius - 1,
                                    90, 4, RAYWHITE);
                }
            }
        }
        if (mouse_in_board) {
            Piece hovered_piece = *board_at(&game, mouse_cpos);

            if (ui_state == UI_STATE_WAITING_FOR_ACTION &&
                hovered_piece.kind == game.active_piece.kind &&
                hovered_piece.player == game.active_piece.player &&
                hovered_piece.id == game.active_piece.id) {
                // Don't show hover for the selected one.
            } else if (hovered_piece.kind != PIECE_NONE && hovered_piece.kind != PIECE_EMPTY) {
                ActionSlice actions = hovered_piece.player == PLAYER_RED ? red_actions : blue_actions;
                Color color = hovered_piece.player == PLAYER_RED ? MAROON : DARKBLUE;
                for (uint64_t i = 0; i < actions.len; i++) {
                    Action action = actions.e[i];
                    if (action.piece == hovered_piece.kind && action.piece_id == hovered_piece.id) {
                        DPos target_dpos = dpos_from_cpos(action.target);
                        Vector2 target_pos = (Vector2) {game_center.x + (horizontal_offset * target_dpos.x),
                                                        game_center.y + vertical_offset * target_dpos.y};
                        DrawPolyLinesEx(target_pos, 6, hex_radius - 2,
                                        90, 2, color);
                    }
                }
            }
        }



        // If hovering, show preview of actions for any piece.
        // If one is selected, show possible actions.


//        if (selected_piece) {
//            Piece selected_piece = *board_at(&game, selected_cpos);
//            for (uint64_t i = 0; i < actions.len; i++) {
//                Action action = actions.e[i];
//                if (selected_piece.player == game.active_player && action.piece == selected_piece.kind &&
//                    action.piece_id == selected_piece.id) {
//                    DPos target_dpos = dpos_from_cpos(action.target);
//                    Vector2 target_pos = (Vector2) {game_center.x + (horizontal_offset * target_dpos.x),
//                                                    game_center.y + vertical_offset * target_dpos.y};
//                    //DrawCircleV(target_pos, hex_radius / 4, GREEN);
//                    DrawPolyLinesEx(target_pos, 6, hex_radius - 1,
//                                    90, 4, RAYWHITE);
//                }
//            }
//        } else {
//            CPos highlighted_cpos = cpos_from_dpos(hilighted_hex);
//            Piece highlighted_piece = *board_at(&game, highlighted_cpos);
//            for (uint64_t i = 0; i < actions.len; i++) {
//                Action action = actions.e[i];
//                if (highlighted_piece.player == game.active_player && action.piece == highlighted_piece.kind &&
//                    action.piece_id == highlighted_piece.id) {
//                    DPos target_dpos = dpos_from_cpos(action.target);
//                    Vector2 target_pos = (Vector2) {game_center.x + (horizontal_offset * target_dpos.x),
//                                                    game_center.y + vertical_offset * target_dpos.y};
//                    //DrawCircleV(target_pos, hex_radius / 4, GREEN);
////                    DrawPolyLinesEx(target_pos, 6, hex_radius - 1,
////                                    90, 4, RAYWHITE);
//                    DrawPolyLinesEx(target_pos, 6, hex_radius - 2,
//                                    90, 2, MAROON);
//                }
//            }
//        }






//        for (int i = 0; i < gesturesCount; i++) {
//            if (i % 2 == 0) DrawRectangle(10, 30 + 20 * i, 200, 20, Fade(LIGHTGRAY, 0.5f));
//            else DrawRectangle(10, 30 + 20 * i, 200, 20, Fade(LIGHTGRAY, 0.3f));
//
//            if (i < gesturesCount - 1) DrawText(gestureStrings[i], 35, 36 + 20 * i, 10, DARKGRAY);
//            else DrawText(gestureStrings[i], 35, 36 + 20 * i, 10, MAROON);
//        }



        //if (currentGesture != GESTURE_NONE) DrawCircleV(touchPosition, 30, MAROON);

        EndDrawing();

        //Vector2 touchPosition = {0, 0};
        //Rectangle touchArea = {220, 10, screenWidth - 230.0f, screenHeight - 20.0f};

        //int gesturesCount = 0;
        //char gestureStrings[MAX_GESTURE_STRINGS][32];

        //int currentGesture = GESTURE_NONE;
        //int lastGesture = GESTURE_NONE;

        //SetGesturesEnabled(0b0000000000001001);   // Enable only some gestures to be detected

//        lastGesture = currentGesture;
//        currentGesture = GetGestureDetected();
//        touchPosition = GetTouchPosition(0);
//
//        if (CheckCollisionPointRec(touchPosition, touchArea) && (currentGesture != GESTURE_NONE)) {
//            if (currentGesture != lastGesture) {
//                // Store gesture string
//                switch (currentGesture) {
//                    case GESTURE_TAP:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE TAP");
//                        break;
//                    case GESTURE_DOUBLETAP:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE DOUBLETAP");
//                        break;
//                    case GESTURE_HOLD:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE HOLD");
//                        break;
//                    case GESTURE_DRAG:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE DRAG");
//                        break;
//                    case GESTURE_SWIPE_RIGHT:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE SWIPE RIGHT");
//                        break;
//                    case GESTURE_SWIPE_LEFT:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE SWIPE LEFT");
//                        break;
//                    case GESTURE_SWIPE_UP:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE SWIPE UP");
//                        break;
//                    case GESTURE_SWIPE_DOWN:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE SWIPE DOWN");
//                        break;
//                    case GESTURE_PINCH_IN:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE PINCH IN");
//                        break;
//                    case GESTURE_PINCH_OUT:
//                        TextCopy(gestureStrings[gesturesCount], "GESTURE PINCH OUT");
//                        break;
//                    default:
//                        break;
//                }
//
//                gesturesCount++;
//
//                // Reset gestures strings
//                if (gesturesCount >= MAX_GESTURE_STRINGS) {
//                    for (int i = 0; i < MAX_GESTURE_STRINGS; i++) TextCopy(gestureStrings[i], "\0");
//
//                    gesturesCount = 0;
//                }
//            }
//        }


    }

    CloseWindow();
}

//int main(void) {
//    //return cli_main();
//    printf("Hello, World!\n");
//    return 0;
//}