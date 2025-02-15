#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
int foo(int x) {
    return x + 3;
}

#include "tazar.h"

#include <assert.h>