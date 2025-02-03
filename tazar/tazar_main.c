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


int main(void) {
    Game game;
    game_init_attrition_hex_field_small(&game);

    const int screen_width = 800;
    const int screen_height = 450;

    Rectangle game_area = {220, 10, (float) screen_width - 230.0f, (float) screen_height - 20.0f};
    Vector2 game_center = {game_area.x + game_area.width / 2, game_area.y + game_area.height / 2};

    InitWindow(screen_width, screen_height, "Tazar Bot");

    SetTargetFPS(60);
    SetExitKey(0);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw grid of tiles.
        // @note: Hardcoded to hex field small.
        float hexes_across = 12.0f;
        float hex_radius = floorf(game_area.width * sqrtf(3.0f) / (3 * hexes_across));
        float horizontal_offset = sqrtf(hex_radius * hex_radius - (hex_radius / 2) * (hex_radius / 2));
        float vertical_offset = hex_radius * 1.5f;

//        Vector2 pos = (Vector2) {game_area.x + horizontal_offset, game_area.y + hex_radius};
//        while (pos.x < game_area.x + game_area.width) {
//            DrawPolyLines(pos, 6, hex_radius, 90, BLACK);
//            pos.x += horizontal_offset * 2;
//        }
        DrawRectangleRec(game_area, LIGHTGRAY);

        DPos screen_center = {10, 4};

        DPos hilighted_hex = {100, 100};
        // iterate in double coordinates
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


//                DrawText(TextFormat("%d,%d,%d", cpos.q, cpos.r, cpos.s),
//                         game_center.x + (horizontal_offset * dpos.x) - 10,
//                         game_center.y + (vertical_offset * dpos.y), 10, BLACK);
                if (CheckCollisionPointCircle(GetMousePosition(),
                                              (Vector2) {game_center.x + horizontal_offset * dpos.x,
                                                         game_center.y + vertical_offset * dpos.y},
                                              horizontal_offset)) {
                    hilighted_hex = dpos;
                }
            }
        }
        DrawPolyLines((Vector2) {game_center.x + horizontal_offset * hilighted_hex.x,
                                 game_center.y + vertical_offset * hilighted_hex.y}, 6, hex_radius,
                      90, RED);




//        for (int i = 0; i < gesturesCount; i++) {
//            if (i % 2 == 0) DrawRectangle(10, 30 + 20 * i, 200, 20, Fade(LIGHTGRAY, 0.5f));
//            else DrawRectangle(10, 30 + 20 * i, 200, 20, Fade(LIGHTGRAY, 0.3f));
//
//            if (i < gesturesCount - 1) DrawText(gestureStrings[i], 35, 36 + 20 * i, 10, DARKGRAY);
//            else DrawText(gestureStrings[i], 35, 36 + 20 * i, 10, MAROON);
//        }

        DrawRectangleLines(10, 29, 200, screen_height - 39, GRAY);
        DrawText("ACTIONS", 12, 15, 10, GRAY);

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