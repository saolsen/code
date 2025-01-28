// This works with gcc and clang on linux and macos.
// It also works with mingw64 on windows.
// It however does not work with windows clang and msvc.
// With the windows c runtime library, the intrinsice headers check for stuff like __AVX2__
// so you can't compile this without passing the flag `-mavx2` or `/arch:AVX2`.
// If you pass those flags, the whole thing will get compiled in that mode so any auto-vectorization
// that happens could use those instructions and if called from a different platform would crash.
// This means the only way to get this to work on windows is to put the implementations in separate
// compilation units and compile those each with the correct flags, they could then be linked into
// the main program.
// That's annoying, because it means you need separate files for each instruction set and a more complicated
// build to compile them each correctly and link them together.
// Learned this from o1 btw which is pretty cool. https://chatgpt.com/share/679938fe-7520-800d-813f-9db72462b344

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// Base version (no SIMD)
void process_data_base(float* data, size_t n) {
    for (size_t i = 0; i < n; i++) data[i] *= 2.0f;
}

// uh, p sure for this sse2 and ss34.1 are the same.
// dunno if I ever need both unless I'm doing a sse4.1 only thing.

#if defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(_M_IX86)
// SSE2 version
#include <emmintrin.h>  // SSE2 headers

__attribute__((target("sse2")))
void process_data_sse2(float* data, size_t n) {
    printf("using sse2\n");
    for (size_t i = 0; i < n; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        vec = _mm_mul_ps(vec, _mm_set1_ps(2.0f));
        _mm_storeu_ps(&data[i], vec);
    }
}

// SSE4.1 version
#include <smmintrin.h>  // SSE4.1 headers

__attribute__((target("sse4.1")))
void process_data_sse41(float* data, size_t n) {
    printf("using sse4.1\n");
    for (size_t i = 0; i < n; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        vec = _mm_mul_ps(vec, _mm_set1_ps(2.0f));
        _mm_storeu_ps(&data[i], vec);
    }
}

// AVX2 version
#include <immintrin.h>  // AVX headers

__attribute__((target("avx2")))
void process_data_avx2(float* data, size_t n) {
    printf("using avx2\n");


    for (size_t i = 0; i < n; i += 8) {
        __m256 vec = _mm256_loadu_ps(&data[i]);
        vec = _mm256_mul_ps(vec, _mm256_set1_ps(2.0f));
        _mm256_storeu_ps(&data[i], vec);
    }
}
#endif

// AI wrote this I have no idea if it's right.
#ifdef __ARM_NEON
#include <arm_neon.h>
void process_data_neon(float* data, size_t n) {
    printf("using neon\n");
    for (size_t i = 0; i < n; i += 4) {
        float32x4_t vec = vld1q_f32(&data[i]);
        vec = vmulq_n_f32(vec, 2.0f);
        vst1q_f32(&data[i], vec);
    }
}
#endif

// CPUID check for x86 features
#if defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(_M_IX86)
static bool has_sse2(void) {
    return __builtin_cpu_supports("sse2");
}

static bool has_sse41(void) {
    return __builtin_cpu_supports("sse4.1");
}

static bool has_avx2(void) {
    return __builtin_cpu_supports("avx2");
}
#endif

#if defined(__ARM_NEON) || defined(__aarch64__)
static bool has_neon(void) {
    return true;  // NEON is always available on ARM (for the 64 bit platforms we care about).
}
#endif

// Function pointer type
typedef void (*process_data_func)(float*, size_t);

// Select best available implementation
process_data_func get_best_impl(void) {
#if defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(_M_IX86)
    if (has_avx2()) return process_data_avx2;
    if (has_sse41()) return process_data_sse41;
    if (has_sse2()) return process_data_sse2;
#elif defined(__aarch64__) || defined(__ARM_NEON)
    if (has_neon()) return process_data_neon;
#endif
    return process_data_base;  // Fallback
}

int main() {
    float data[1024];
    process_data_func impl = get_best_impl();

    impl(data, 1024);  // Uses optimal SIMD version
    return 0;
}