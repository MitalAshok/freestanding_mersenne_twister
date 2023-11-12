#include "./config.h"

TEST_CASE( "Initial few elements of sequence" ) {
    constexpr size_t w =
#if !defined(MT_MAX_WIDTH) || MT_MAX_WIDTH > 64
            MT_TEST_SUPPORTS_WIDTH(47) ? 47 : MT_MIN_WIDTH
#else
            MT_TEST_SUPPORTS_WIDTH(47) ? 47 : MT_MAX_WIDTH
#endif
            ;
    constexpr unsigned long long mask = w >= 64 ? 0xffff'ffff'ffff'ffffull : (1ull << (MT_MIN_WIDTH)) - 1ull;
    //
    constexpr size_t m = 13;
    constexpr size_t r = w / 2u;
    constexpr unsigned long long a = 0xafce8adbaee3fae0ull & mask;
    constexpr size_t u = 13;
    constexpr unsigned long long d = 0x86ec67497b583863ull & mask;
    constexpr size_t s = 2;
    constexpr unsigned long long b = 0x8940d4de06d0fef6ull & mask;
    constexpr size_t t = 0;
    constexpr unsigned long long c = 0x69526c3559c3a13bull & mask;
    constexpr size_t l = 31;
    constexpr unsigned long long f = 0x2ff6fad8e693caf6ull & mask;
    using test = mt_test<unsigned long long, w, 17, m, r, a, u, d, s, b, t, c, l, f>;
    using base = mt_base<unsigned long long, w, 17, m, r, a, u, d, s, b, t, c, l, f>;

    SECTION( "First 1000 (Integer seed)" ) {
        test t(1);
        base b(1);
        for (int i = 0; i < 1000; ++i) {
            REQUIRE(t() == b());
        }
    }
}
