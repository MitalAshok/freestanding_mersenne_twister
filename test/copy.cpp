#include "./config.h"

TEST_CASE( "Copying" ) {
    using mt = mt_test<unsigned long long, MT_TEST_MIN_WIDTH, 32, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0>;
    const mt zero(0);
    const mt one(0);

    SECTION( "copy constructor" ) {
        {
            mt a(zero);
            REQUIRE(a == zero);
        }
        {
            mt a(zero);
            a();
            mt b(a);
            REQUIRE(a == b);
            REQUIRE(a() == b());
        }

        {
            for (int i = 29; i < 34; ++i) {
                mt a(zero);
                for (int j = 0; j < i; ++j) a();
                for (int j = 0; j < 3; ++j) {
                    mt b(a);
                    REQUIRE(a == b);
                    mt c(b);
                    for (int k = 0; k < j; ++k) {
                        c();
                        b();
                    }
                    REQUIRE(b == c);
                    REQUIRE(b() == c());
                }
            }
        }
    }

    SECTION( "copy assign" ) {
        {
            mt a(1);
            a = zero;
            REQUIRE(a == zero);
        }
        {
            mt a(zero);
            unsigned long long x = a();
            a = zero;
            REQUIRE(a == zero);
            REQUIRE(x == static_cast<unsigned long long>(a()));
        }
        {
            for (int i = 29; i < 34; ++i) {
                mt a(zero);
                for (int j = 0; j < i; ++j) a();
                for (int j = 0; j < 3; ++j) {
                    mt b(zero);
                    b = a;
                    REQUIRE(a == b);
                    mt c(one);
                    c = b;
                    for (int k = 0; k < j; ++k) {
                        c();
                        b();
                    }
                    REQUIRE(b == c);
                    REQUIRE(b() == c());
                }
            }
        }
    }
}
