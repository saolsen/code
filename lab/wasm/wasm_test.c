// asdf exec emcc ./wasm_test.c -s WASM=1 -s EXPORTED_FUNCTIONS="['_foo']" -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -o module.js
// I think it's probably best if the emscripten layer is on top of a normal c layer.
// That way I can run the normal c as c locally and debug it.

// I think I need to use embind to make the passing of data back and forth easier.
// I suppose just having one single module makes this easier too.
// Should I port the existing stuff first or should I design the new game engine from scratch
// to support all the stuff I want to support?

#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
int foo(int x) {
    return x+3;
}

// I need to figure out how to set up clion for emscripten. I don't think I'll be able to debug
// though.