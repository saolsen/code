// Since on 64-bit systems you always have sse2 or neon, can you just always use those for 128 bit ints?
#include <stdio.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(_M_IX86)
#include <emmintrin.h>  // SSE2 headers

#elif defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>   // NEON headers

#else
#error "Unsupported Platform"
#endif

int main(void) {
    printf("Big Ints\n");
    return 0;
}
