// Since on 64-bit systems you always have sse2 or neon, can you just always use those for 128 bit
// ints?
#include <stdint.h>
#include <stdio.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(_M_IX86)
#include <emmintrin.h> // SSE2 headers

#elif defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h> // NEON headers

#else
#error "Unsupported Platform"
#endif

int main(void) {
    // Do 128 bit ints just work?
    // instead of uint_8, this should be uint2_t, 00, 01, 10, 11
    // uint8_t seed[64] = {0,1,2,3,0,1,2,3,
    //                     0,1,2,3,0,1,2,3,
    //                     0,1,2,3,0,1,2,3,
    //                     0,1,2,3,0,1,2,3,
    //                     0,1,2,3,0,1,2,3,
    //                     0,1,2,3,0,1,2,3,
    //                     0,1,2,3,0,1,2,3,
    //                     0,1,2,3,0,1,2,3,
    //                     };
    //
    // __uint128_t a = 0x123456789abcdef0;
    //
    // uint16_t seed_rows[8] = {0,1,2,3,4,5,6,7};
    //
    // printf("Big Int: %lld\n", a);
    return 0;
}
