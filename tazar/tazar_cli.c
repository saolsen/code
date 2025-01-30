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

typedef struct {
    int q;
    int r;
    int s;
} CPos;
typedef struct {
    int q;
    int r;
} APos;
typedef struct {
    int x;
    int y;
} Pos;

// ok, so like this
// the CENTER hex (0,0,0) should be at 18, 4

int cli_main(void) {
    setlocale(LC_ALL, "");
    cli_set_wide_stream(stdout);
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


#define MAX(a, b) ((a) > (b) ? (a) : (b)) \
// q is "width" rows to the up/right
#define board_q 2
// r is "height" rows down from 0.
#define board_r 2
// s is "depth" rows to the up/left
#define board_s 2
// since there are 3 directions, to take a step you have to move in 2 directions.



// that's not a great way to index stuff since it's sparse. There are lots of invalid (q,r,s) "positions"
// so a better coordinate system is axial with q and r only.
// s is always = to -q-r
#define board_size (board_q * board_r)
// index the board with r*board_r + q (similar to y*width + x)
    //int board[board_size] = {0};

    // I guess try drawing it centered in an 80 col terminal?
    //int char_width = board_r * 2 + 1;
    int char_height = MAX(board_q * 2 + 1, board_s * 2 + 1);

    // lets call the plane position, x and y.
    // y from -soemthing to something
    // x from -something to something
    // the center thing will be 0,0,0

    // we don't check every position though, it's like an offset grid thing.
    // lets use what's called "doubled coordinates".

    // 0,0 | 2,0 | 4,0 | 6,0 | 8,0
    //    1,1 | 3,1 | 5,1 | 7,1 | 9,1
    // 0,2 | 2,2 | 4,2 | 6,2 | 8,2
    //    1,3 | 3,3 | 5,3 | 7,3 | 9,3

    // 0,0,0 maps to 0,0 and we want that to be the center of the screen.

    // so we start from the upper left most tile and proceed to the lower right most tile.
    // that is start with q=0, s=size, r=-size

    // so we want to know farthest tile up from origin and furthest tile left from origin to start
    // the drawing at.

    // up is easy, because it's just r
    // left is I think max(q,s)

    // so we have cubic coordinates.
    // axial coordinates.
    // planar coordinates which is the double coordinate offsets in the plane
    // and then screen coordinates.
    // I suppose you should be able to write matrices that can convert between all of them, then you can
    // directly translate everything and index it directly. I don't really feel like figuring that out rn.



//    function doublewidth_to_axial(hex):
//    var q = (hex.col - hex.row) / 2
//    var r = hex.row
//    return Hex(q, r)

//    function axial_to_doublewidth(hex):
//    var col = 2 * hex.q + hex.r
//    var row = hex.r
//    return DoubledCoord(col, row)
    APos apos = {0, 1};
    CPos cpos = {apos.q, apos.r, -apos.q - apos.r};
    Pos double_width_pos = {cpos.r, 2 * cpos.q + cpos.r};
    printf("double width pos: %d, %d\n", double_width_pos.x, double_width_pos.y);

    // most left is max of q and s
    // most up is r
    // I wanna start at where it'd be centered in 80cols

    wchar_t buf[(80 * 19) + 1];
    wmemset(buf, L' ', 80 * 19);
    buf[80 * 19] = L'\0';
    for (int row = 0; row < 19; row++) {
        buf[row * 80 + 79] = L'\n';
    }
    wchar_t *symbols = L"■□▲△●○★☆.";

    // the CENTER hex (0,0,0) should be at 18, 4
    Pos screen_center = {18, 4};

    // Iterate in double coordinates.
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 38; col++) {
            if ((row % 2 == 0 && col % 2 != 0) ||
                (row % 2 == 1 && col % 2 != 1)) {
                continue;
            }

            Pos screen_pos = {col - screen_center.x, row - screen_center.y};

            // Draw a C at the center.
            // @opt(steve): This does "overdraw" in that it sets the border characters multiple times.
            // I don't think I care tho.
            int char_x = col * 2 + 2;
            int char_y = row * 2 + 1;
            if (screen_pos.x == 0 && screen_pos.y == 0) {
                buf[(char_y - 1) * 80 + (char_x - 1)] = L'╱';
                buf[(char_y - 1) * 80 + (char_x + 1)] = L'╲';
                buf[char_y * 80 + (char_x - 2)] = L'│';
                buf[char_y * 80 + (char_x + 2)] = L'│';
                buf[(char_y + 1) * 80 + (char_x - 1)] = L'╲';
                buf[(char_y + 1) * 80 + (char_x + 1)] = L'╱';
                buf[char_y * 80 + char_x] = L'♛';
            } else {
                buf[char_y * 80 + char_x] = symbols[row];
            }

        }
    }
    buf[80 * 19] = L'\0';
    wprintf(L"%ls", buf);
    wprintf(L"\n");
    wprintf(L"■□▲△●○★☆ \n");

    for (int char_y = 0; char_y < 19; char_y++) {
        for (int char_x = 0; char_x < 80; char_x++) {
            // Is this a double coordinate?
            // column starts at 2 on even rows, and next one is + 4
            // or, for every row, 2, 4, 6, 8, ... just count by 2. so check mod 2.
            // 1 is the first row, then every 2 after that is a row.
        }
    }


    for (int y = 0; y < char_height; y++) {

        // for the first row.
        // s starts at size and goes down to min(-row, -size)
        // q starts at min(-r, -size) and goes up to
        //

        // width at this height?
        // q = board_q - i,
    }

    fflush(stdout);
    return 0;
}