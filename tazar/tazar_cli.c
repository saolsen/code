#include "tazar.h"

#include <locale.h>
#include <stdio.h>
#include <wchar.h>

// Enable wide character printing.
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>

int cli_set_wide_stream(FILE *stream) {
    return _setmode(_fileno(stream), _O_U16TEXT);
}

#else // posix

int cli_set_wide_stream(FILE *stream) {
    return fwide(stream, 1);
}

#endif


int cli_main(void) {
    setlocale(LC_ALL, "");
    cli_set_wide_stream(stdout);

    wprintf(L"Simple Tazar Bot, Playing 'ATTRITION' on 'Hex Field Small'\n");
    wprintf(L"* You are Red, (the pieces in white). Opponent is Blue (the pieces in black).\n");
    wprintf(L"Units: ☆ = (c)rown, □ = (p)ike, △ = (h)orse, ○ = (b)ow.\n  Refer to units by id, eg: 'p2'\n");
    wprintf(L"Actions: (move),(volley),(charge),(end)'\n");
    wprintf(L"Directions: (r)ight-(u)p, (r)ight, (r)ight-(d)own, (l)eft-(d)own, (l)eft, (l)eft-(u)p.'\n");
    wprintf(L"Commands: '[UNIT] [ACTION] [TARGET PATH]'.\n");
    wprintf(L" examples:\n");
    wprintf(L"   'p3 move ru r'      Move pike(3) 2 tiles. Right-up, then right\n");
    wprintf(L"   'b1 volley r r r'   Shoots a volley from bow(1) at the tile two to the right of it.\n");
    wprintf(L"Red (you) goes first.\n");

    Game game;
    game_init(&game);

    wchar_t buf[(80 * 19) + 1];
    wmemset(buf, L' ', 80 * 19);
    buf[80 * 19] = L'\0';
    for (int row = 0; row < 19; row++) {
        buf[row * 80 + 79] = L'\n';
    }

    // The center hex in the view.
    DPos screen_center = {18, 4};

    // Iterate in double coordinates.
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 38; col++) {
            if ((row % 2 == 0 && col % 2 != 0) ||
                (row % 2 == 1 && col % 2 != 1)) {
                // Not a valid double coordinate.
                continue;
            }

            DPos screen_pos = {col - screen_center.x, row - screen_center.y};
            CPos cpos = cpos_from_dpos(screen_pos);
            Piece piece = *board_at(&game, cpos);
            if (piece.kind == PIECE_NONE) {
                continue;
            }

            // Draw hex outline.
            // @opt(steve): This does "overdraw" in that it sets the border characters multiple times.
            // I don't think I care tho.
            int char_x = col * 2 + 2;
            int char_y = row * 2 + 1;
            buf[(char_y - 1) * 80 + (char_x - 1)] = L'╱';
            buf[(char_y - 1) * 80 + (char_x + 1)] = L'╲';
            buf[char_y * 80 + (char_x - 2)] = L'│';
            buf[char_y * 80 + (char_x + 2)] = L'│';
            buf[(char_y + 1) * 80 + (char_x - 1)] = L'╲';
            buf[(char_y + 1) * 80 + (char_x + 1)] = L'╱';

            // Draw piece.
            wchar_t symbol;
            wchar_t id;
            switch (piece.kind) {
                case PIECE_CROWN:
                    symbol = piece.player == PLAYER_RED ? L'☆' : L'★';
                    id = L'0' + piece.id;
                    break;
                case PIECE_PIKE:
                    symbol = piece.player == PLAYER_RED ? L'□' : L'■';
                    id = L'0' + piece.id;
                    break;
                case PIECE_HORSE:
                    symbol = piece.player == PLAYER_RED ? L'△' : L'▲';
                    id = L'0' + piece.id;
                    break;
                case PIECE_BOW:
                    symbol = piece.player == PLAYER_RED ? L'○' : L'●';
                    id = L'0' + piece.id;
                    break;
                default:
                    symbol = L' ';
                    id = L' ';
                    break;
            }
            buf[char_y * 80 + char_x] = symbol;
            buf[(char_y + 1) * 80 + char_x] = id;
        }
    }
    buf[80 * 19] = L'\0';
    wprintf(L"%ls", buf);
    wprintf(L"\n");
    wprintf(L"> ");


    fflush(stdout);
    return 0;
}

#if 0
wprintf(L"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    wprintf(L"         .         .         .        C          .         .         .         \n");
    wprintf(L" ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲   \n");
    wprintf(L"│ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |  \n");
    wprintf(L" ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ \n");
    wprintf(L"  │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |\n");
    wprintf(L" ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ \n");
    wprintf(L"│ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |  \n");
    wprintf(L" ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ \n");
    wprintf(L"  │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |\n");
    wprintf(L" ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ \n");
    wprintf(L"│ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ C │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |  \n");
    wprintf(L" ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ \n");
    wprintf(L"  │ X │ Y │ Y │ Y │ Y │ Y │ Y │ P │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |\n");
    wprintf(L" ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲1╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ \n");
    wprintf(L"│ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |  \n");
    wprintf(L" ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ \n");
    wprintf(L"  │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |\n");
    wprintf(L" ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ \n");
    wprintf(L"│ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |  \n");
    wprintf(L" ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱   \n");
    wprintf(L"|                                                                              \n");
    wprintf(L"           ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲           \n");
    wprintf(L"          │ Y │ Y │ Y │ Y │ Y │          \n");
    wprintf(L"         ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲         \n");
    wprintf(L"        │ Y │ Y │ Y │ Y │ Y │ Y │        \n");
    wprintf(L"       ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲       \n");
    wprintf(L"      │ Y │ Y │ Y │ Y │ Y │ Y │ Y │      \n");
    wprintf(L"     ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲     \n");
    wprintf(L"    │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │    \n");
    wprintf(L"   ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲   \n");
    wprintf(L"  │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │  \n");
    wprintf(L"   ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱   \n");
    wprintf(L"    │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │    \n");
    wprintf(L"     ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱     \n");
    wprintf(L"      │ Y │ Y │ Y │ Y │ Y │ Y │ Y │      \n");
    wprintf(L"       ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱       \n");
    wprintf(L"        │ Y │ Y │ Y │ Y │ Y │RB4│        \n");
    wprintf(L"         ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱         \n");
    wprintf(L"          │ ■ │ Y │RC1│ ■ │BP3│          \n");
    wprintf(L"           ╲1╱ ╲ ╱ ╲ ╱ ╲5╱ ╲ ╱           \n");
    wprintf(L"                                         \n");
#endif

// P1 MOVE R R UR
// H1 CHARGE L L L
//
//P1 R R UR

// P1 MOVE R R UR DL L UR DR R