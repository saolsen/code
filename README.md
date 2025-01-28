# Code

This is a "mono-repo" for various c code. Created because I got sick of setting up new projects.
It unfortunately uses CMake, because it's the best build system for CLion, my ide of choice.
The code should run under windows/linux/mac under gcc/clang/msvc and c11 or c17. Those platforms are all tested in CI.
The code also only supports 64bit platforms, I depend on that for my Arena implementation in steve.h which uses
large virtual allocations to get contiguous blocks of memory.

## steve.h

This is my general purpose c helper library. It's heavily inspired by the stb libraries, and contains a bunch of
useful data structures and utilities I use accross projects. It's a single header file that includes both the
implementation and the interface. To use it include `steve.h` in any files that use it and define `STEVE_IMPLEMENTATION`
before including it in one file to include the implementation.

It provides the following
* Useful helper macros like MIN/MAX/CLAMP etc.
* Common helper functions like `pow2_next`.
* Arena: A linear allocator that uses a single large virtual allocation.
  * Implemented on posix with `mmap` and windows with `VirtualAlloc`.
  * Includes poisoning/un-poisoning instrumentation so when you run it under address sanitizer it can find invalid reads
    and writes.
  * Arenas can also be serialized/deserialized by copying the block of memory.
  * There are helpers to use arena-relative pointers instead of absolute pointers. If you use these for all the data
    structures in the arena, then you don't need to do any pointer "fixups" when serializing / deserializing.
  * There's a thread local scratch arena that can be used to allocate temporary memory. This makes it easy and cheap to
    allocate things that are just used for a single function call.
* A "dynamic array" that uses an arena for memory.
  * When it needs more room it can reallocate normally, or if it's on the end of an arena it can grow in place. 
* A "standard" slice type to pass around views of arrays.
* Some string utilities that operate on `String`, which is a slice of characters.

### steve.h TODO
* A hash table.
* A pool.
* A string intern table.

## dandd.c

Dungeons and Diagrams solver and puzzle generator.