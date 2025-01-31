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






// ok, so like this
// the CENTER hex (0,0,0) should be at 18, 4


// this is the hex field small
#define board_q 4
#define board_r 4
#define board_s 4

// Board is indexed by cubic coordinates.
wchar_t board[((board_q * 2) + 1) * ((board_r * 2) + 1)];

wchar_t wnull = 0;

wchar_t *board_at(CPos pos) {
    if (pos.q < -board_q || pos.q > board_q || pos.r < -board_r || pos.r > board_r || pos.s < -board_s ||
        pos.s > board_s) {
        return &wnull;
    }
    int index = (pos.r + 4) * (board_q * 2 + 1) + (pos.q + 4);
    return &board[index];
}


int cli_main(void) {
    // Set up board.
    wmemset(board, 0, sizeof(board) / sizeof(wchar_t));

    for (int q = -board_q; q <= board_q; q++) {
        for (int r = -board_r; r <= board_r; r++) {
            for (int s = -board_s; s <= board_s; s++) {
                CPos cpos = {q, r, s};
                if (cpos.q + cpos.r + cpos.s != 0) {
                    continue;
                }
                *board_at(cpos) = L' ';
            }
        }
    }

    // Set up attrition.
    CPos p = (CPos) {-4, 0, 4};
    *board_at(p) = L'☆';
    p = cpos_add(p, CPOS_RIGHT_UP);
    *board_at(p) = L'○';
    p = cpos_add(p, CPOS_RIGHT_UP);
    *board_at(p) = L'△';
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(p) = L'□';
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(p) = L'□';
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(p) = L'○';
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(p) = L'□';
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(p) = L'□';
    p = cpos_add(p, CPOS_LEFT);
    *board_at(p) = L'○';
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(p) = L'△';
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(p) = L'□';

    p = (CPos) {4, 0, -4};
    *board_at(p) = L'★';
    p = cpos_add(p, CPOS_LEFT_UP);
    *board_at(p) = L'●';
    p = cpos_add(p, CPOS_LEFT_UP);
    *board_at(p) = L'▲';
    p = cpos_add(p, CPOS_LEFT);
    *board_at(p) = L'■';
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(p) = L'■';
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(p) = L'●';
    p = cpos_add(p, CPOS_LEFT);
    *board_at(p) = L'■';
    p = cpos_add(p, CPOS_RIGHT_DOWN);
    *board_at(p) = L'■';
    p = cpos_add(p, CPOS_RIGHT);
    *board_at(p) = L'●';
    p = cpos_add(p, CPOS_LEFT_DOWN);
    *board_at(p) = L'▲';
    p = cpos_add(p, CPOS_LEFT);
    *board_at(p) = L'■';


    setlocale(LC_ALL, "");
    cli_set_wide_stream(stdout);
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

    wchar_t buf[(80 * 19) + 1];
    wmemset(buf, L' ', 80 * 19);
    buf[80 * 19] = L'\0';
    for (int row = 0; row < 19; row++) {
        buf[row * 80 + 79] = L'\n';
    }
    //wchar_t *symbols = L"■□▲△●○★☆.";

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
            wchar_t piece = *board_at(cpos);
            if (piece == (wchar_t) 0) {
                // Not a tile on the map.
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
            buf[char_y * 80 + char_x] = piece;
        }
    }
    buf[80 * 19] = L'\0';
    wprintf(L"%ls", buf);
    wprintf(L"\n");

    fflush(stdout);
    return 0;
}