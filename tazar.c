// A really basic implementation of Tazar and an AI player, so I have someone to play with.
// https://kelleherbros.itch.io/tazar
// Been running this alongside and inputting all the moves into the real game, that's why it
// asks for the results of Volley rolls. Not really meant to be "played" standalone.

#include <stdio.h>

#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <wchar.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>

static int set_wide_stream(FILE *stream)
{
    return _setmode(_fileno(stream), _O_U16TEXT);
}

#else

static int set_wide_stream(FILE *stream) {
    return fwide(stream, 1);
}

#endif

int main(void) {
    setlocale(LC_ALL, "");

    /* After this call, you must use wprintf(),
       fwprintf(), fputws(), putwc(), fputwc()
       -- i.e. only wide print/scan functions
       with this stream.
       You can print a narrow string using e.g.
       wprintf(L"%s\n", "Hello, world!");
    */
    set_wide_stream(stdout);

    /* These may not work in Windows, because
       the code points are 0x1F785 .. 0x1F7AE
       and Windows is probably limited to
       Unicode 0x0000 .. 0xFFFF */
    wprintf(L"🞨🞩🞪🞫🞬🞭🞮 🞉🞈🞇🞆🞅\n");

    /* These are from the Box Drawing Unicode block,
       U+2500 ─, U+2502 │, and U+253C ┼,
       and should work everywhere. */
    wprintf(L"   │   │   \n");
    wprintf(L"───┼───┼───\n");
    wprintf(L"   │   │   \n");
    wprintf(L"───┼───┼───\n");
    wprintf(L"   │   │   \n");
    fflush(stdout);
    return 1;
}