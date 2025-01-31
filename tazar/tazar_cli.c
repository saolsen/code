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
            switch (piece.kind) {
                case PIECE_CROWN:
                    symbol = piece.player == PLAYER_RED ? L'☆' : L'★';
                    break;
                case PIECE_PIKE:
                    symbol = piece.player == PLAYER_RED ? L'□' : L'■';
                    break;
                case PIECE_HORSE:
                    symbol = piece.player == PLAYER_RED ? L'△' : L'▲';
                    break;
                case PIECE_BOW:
                    symbol = piece.player == PLAYER_RED ? L'○' : L'●';
                    break;
                default:
                    symbol = L' ';
                    break;
            }
            buf[char_y * 80 + char_x] = symbol;
        }
    }
    buf[80 * 19] = L'\0';
    wprintf(L"%ls", buf);
    wprintf(L"\n");

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
    wprintf(L"  │ X │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y │ Y |\n");
    wprintf(L" ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ \n");
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
    wprintf(L"        │ Y │ Y │ Y │ Y │ Y │ Y │        \n");
    wprintf(L"         ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱         \n");
    wprintf(L"          │ Y │ Y │ Y │ Y │ Y │          \n");
    wprintf(L"           ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱ ╲ ╱           \n");
    wprintf(L"                                         \n");
#endif