#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

// 128 big unsigned integer just works on 64-bit systems with clang/gcc.
// still need to figure it out for msvc.

typedef uint8_t     u8;
typedef int32_t     i32;
typedef uint64_t    u64;
typedef __uint128_t u128;

int main(void) {
    u128 a = (((u128)100000004294967294 << 64) | 4294967294);
    //a += 1;

    // Print the hex integer
    u64 high = (u64)(a >> 64);
    u64 low = (u64)a & 0xffffffffffffffff;
    // 32 digits.
    printf("Hex:       0x%016" PRIx64 "%016" PRIx64 "\n", high, low);
    // 40 digits.
    printf("Dec: %20.0" PRIu64 "%020" PRIu64 "\n", high, low);

    return 0;
}
