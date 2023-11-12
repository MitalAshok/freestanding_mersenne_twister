#ifndef MT_TEST_CONFIG
#define MT_TEST_CONFIG

#include <cstdint>
#include <random>
#include <limits>

#include <catch2/catch_test_macros.hpp>

#include "freestanding_mersenne_twister.h"

/**
 * Two sets of definitions, one for "MT_BASE" for the correctly implemented mersenne twister engine
 * and "MT_TEST" which will be tested against "MT_BASE".
 */

// Minimum value for parameter w
#define MT_BASE_MIN_WIDTH 3
// Maximum value for parameter w (Not defined if there is no maximum)
// Set to std::numeric_limits<T>::digits if the base engine has a fixed underlying type T
#undef MT_BASE_MAX_WIDTH
// `mt_base<params>` should be a class that can be constructed with a seed value (of at most w bits) and
// has an operator() that returns the next number. It should also be copy constructible.
// Minimal example if these are passed at runtime:
/*
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
struct mt_base {
    // This library uses unsigned long long as a fixed underlying type for this example
    // So MT_BASE_MAX_WIDTH is 64
    my_library_mt_state_t* state;
    static_assert(MT_BASE_MIN_WIDTH <= w);
    // All the other parameters will be in bounds
    mt_base(unsigned long long seed) {
      my_library_create_state(&state, w, n, m, r, a, u, d, s, b, t, c, l, f);
    }
    mt_base(const mt_base& other) {
      my_library_copy_state(&state, other.state);
    }
    ~mt_base() {
      my_library_dispose_state(state);
    }
    unsigned long long operator()() {
      unsigned long long result = my_library_get(state);
      my_library_advance(state);
      return result;
    }
    // Optionally
    void discard(unsigned long long amount) {
        my_library_discard(state, amount);
    }
};
 */
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
using mt_base = std::mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>;
// Define if there is a member discard()
#define MT_BASE_HAS_DISCARD
#define MT_BASE_HAS_SEEDSEQ

#define MT_TEST_MIN_WIDTH 3
#undef MT_TEST_MAX_WIDTH
// mt_test is like mt_base, except extra functionality is expected:
//  * Equality comparable (operator==)
//  * Copy assignable
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
using mt_test = freestanding_mersenne_twister::mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>;
#define MT_TEST_HAS_DISCARD
#define MT_TEST_HAS_SEEDSEQ
#define MT_TEST_HAS_CALL_ITERATOR
#define MT_TEST_HAS_PEEK
#define MT_TEST_HAS_PEEK_ITERATOR
#define MT_TEST_HAS_STREAM_INSERTION


// Config end

using std::size_t;


#ifdef MT_BASE_HAS_DISCARD
template<class Engine>
void mt_base_discard(Engine& e, unsigned long long amount) {
    e.discard(amount);
}
#else  // MT_BASE_HAS_DISCARD
template<class Engine>
void mt_base_discard(Engine& e, unsigned long long amount) {
    while (amount-- > 0u) {
        e();
    }
}
#endif  // !MT_BASE_HAS_DISCARD

#ifndef MT_TEST_NO_BITINT_NUMERIC_LIMITS
#ifdef __BITINT_MAXWIDTH__
template<int N>
constexpr int bitint_digits10(unsigned _BitInt(N) x) {
    return x ? 1 + bitint_digits10<N>(x / 10u) : 0;
}

namespace std {

template<int N>
struct numeric_limits<unsigned _BitInt(N)> {  // If there is a redefinition error on this line define MT_TEST_NO_BITINT_NUMERIC_LIMITS
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true;
    static constexpr int digits = N;
    static constexpr int digits10 = bitint_digits10<N>(~static_cast<unsigned _BitInt(N)>(0u)) - 1;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps = numeric_limits<int>::traps;
    static constexpr bool tinyness_before = false;
    static constexpr unsigned _BitInt(N) min() noexcept { return 0; }
    static constexpr unsigned _BitInt(N) lowest() noexcept { return 0; }
    static constexpr unsigned _BitInt(N) max() noexcept { return ~static_cast<unsigned _BitInt(N)>(0); }
    static constexpr unsigned _BitInt(N) epsilon() noexcept { return 0; }
    static constexpr unsigned _BitInt(N) round_error() noexcept { return 0; }
    static constexpr unsigned _BitInt(N) infinity() noexcept { return 0; }
    static constexpr unsigned _BitInt(N) quiet_NaN() noexcept { return 0; }
    static constexpr unsigned _BitInt(N) signaling_NaN() noexcept { return 0; }
    static constexpr unsigned _BitInt(N) denorm_min() noexcept { return 0; }
};

#endif
}
#endif


#if MT_BASE_MIN_WIDTH < MT_TEST_MIN_WIDTH
#define MT_MIN_WIDTH MT_TEST_MIN_WIDTH
#else
#define MT_MIN_WIDTH MT_BASE_MIN_WIDTH
#endif

#ifdef MT_BASE_MAX_WIDTH
#ifdef MT_TEST_MAX_WIDTH  // defined(MT_BASE_MAX_WIDTH) && defined(MT_TEST_MAX_WIDTH)

#if MT_BASE_MAX_WIDTH > MT_TEST_MAX_WIDTH
#define MT_MAX_WIDTH MT_TEST_MAX_WIDTH
#else
#define MT_MAX_WIDTH MT_BASE_MAX_WIDTH
#endif

#else  // defined(MT_BASE_MAX_WIDTH) && !defined(MT_TEST_MAX_WIDTH)
#define MT_MAX_WIDTH MT_BASE_MAX_WIDTH
#endif
#else
#ifdef MT_TEST_MAX_WIDTH  // !defined(MT_BASE_MAX_WIDTH) && defined(MT_TEST_MAX_WIDTH)
#define MT_MAX_WIDTH MT_BASE_MAX_WIDTH
#else  // !defined(MT_BASE_MAX_WIDTH) && !defined(MT_TEST_MAX_WIDTH)
#undef MT_MAX_WIDTH
#endif
#endif


#ifdef MT_BASE_MAX_WIDTH
#define MT_BASE_SUPPORTS_WIDTH(W) (((MT_BASE_MIN_WIDTH) <= (W)) && ((W) <= (MT_BASE_MAX_WIDTH)))
#else
#define MT_BASE_SUPPORTS_WIDTH(W) ((MT_BASE_MIN_WIDTH) <= (W))
#endif

#ifdef MT_TEST_MAX_WIDTH
#define MT_TEST_SUPPORTS_WIDTH(W) (((MT_TEST_MIN_WIDTH) <= (W)) && ((W) <= (MT_TEST_MAX_WIDTH)))
#else
#define MT_TEST_SUPPORTS_WIDTH(W) ((MT_TEST_MIN_WIDTH) <= (W))
#endif

#ifdef MT_MAX_WIDTH
#if (MT_MAX_WIDTH) < (MT_MIN_WIDTH)
#error No overlap in width between base and test mersenne twister engines
#endif
#define MT_SUPPORTS_WIDTH(W)  (((MT_MIN_WIDTH) <= (W)) && ((W) <= (MT_MAX_WIDTH)))
#else
#define MT_SUPPORTS_WIDTH(W) ((MT_MIN_WIDTH) <= (W))
#endif

#if MT_MIN_WIDTH > 64
#error MT_BASE_MIN_WIDTH MT_TEST_MIN_WIDTH should both be smaller than 64 bits
#endif


template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
struct mt_both {
    using base = mt_base<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>;
    using test = mt_test<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>;
};

#if MT_BASE_SUPPORTS_WIDTH(32)
using base32 = mt_base<std::uint32_t, 32, 624, 397, 31, 0x9908'b0dfu, 11, 0xffff'ffffu, 7, 0x9d2c'5680u, 15, 0xefc6'0000u, 18, 1'812'433'253u>;
#endif
#if MT_BASE_SUPPORTS_WIDTH(64)
using base64 = mt_base<std::uint64_t, 64, 312, 156, 31, 0xb502'6f5a'a966'19e9u, 29, 0x5555'5555'5555'5555u, 17, 0x71d6'7fff'eda6'0000u, 37, 0xfff7'eee0'0000'0000u, 43, 6'364'136'223'846'793'005u>;
#endif

#if MT_TEST_SUPPORTS_WIDTH(32)
using test32 = mt_test<std::uint32_t, 32, 624, 397, 31, 0x9908'b0dfu, 11, 0xffff'ffffu, 7, 0x9d2c'5680u, 15, 0xefc6'0000u, 18, 1'812'433'253u>;
#endif
#if MT_TEST_SUPPORTS_WIDTH(64)
using test64 = mt_test<std::uint64_t, 64, 312, 156, 31, 0xb502'6f5a'a966'19e9u, 29, 0x5555'5555'5555'5555u, 17, 0x71d6'7fff'eda6'0000u, 37, 0xfff7'eee0'0000'0000u, 43, 6'364'136'223'846'793'005u>;
#endif

#endif  // MT_TEST_CONFIG
