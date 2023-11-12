#include "./config.h"

namespace {
template<class Engine>
void check_equality(bool should_be_equal, Engine& l, Engine& r) {
    if (should_be_equal) {
        REQUIRE(l == r);
        for (int i = 0; i < 65; ++i) {
            REQUIRE(l() == r());
        }
    } else {
        REQUIRE(l != r);
    }
}

template<class Engine>
void check_equality_unknown_equality(Engine& l, Engine& r) {
    if (l == r) {
        for (int i = 0; i < 65; ++i) {
            REQUIRE(l() == r());
        }
    }
}
}

TEST_CASE( "Test equality operator" ) {
    using mt = mt_test<unsigned long long, MT_TEST_MIN_WIDTH, 32, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0>;
    const mt zero(0);
    SECTION( "Self equality" ) {
        REQUIRE(zero == zero);

        mt a(zero);
        REQUIRE(a == a);
        for (int i = 0; i < 65; ++i) {
            a();
            REQUIRE(a == a);
        }
    }

    SECTION( "After construction" ) {
        {
            mt a(zero);
            mt b(zero);
            check_equality(true, a, b);
        }
        {
            mt a(0b111);
            mt b(0b111);
            check_equality(true, a, b);
        }
        {
            mt a(zero);
            mt b(0b111);
            check_equality_unknown_equality(a, b);
        }
    }

    SECTION( "After copying" ) {
        {
            mt a(zero);
            mt b(a);
            check_equality(true, a, b);
        }
        {
            mt a(zero);
            mt b(a);
            b();
            check_equality(false, a, b);
        }
        {
            mt a(zero);
            a();
            mt b(a);
            a();
            b();
            check_equality(true, a, b);
        }
        {
            mt a(zero);
            a();
            mt b(a);
            mt c(zero);
            c();
            check_equality(true, b, c);
        }
    }

    SECTION( "After advancing" ) {
        {
            mt a(zero);
            mt b(zero);
            a();
            check_equality(false, a, b);
        }
        {
            mt a(zero);
            mt b(zero);
            b();
            check_equality(false, a, b);
        }
        {
            mt a(zero);
            mt b(zero);
            a();
            b();
            b();
            check_equality(false, a, b);
        }
        {
            mt a(zero);
            mt b(zero);
            a();
            b();
            check_equality(true, a, b);
        }
        {
            for (int i = 0; i < 65; ++i) {
                for (int j = 0; j < 65; ++j) {
                    mt a(zero);
                    mt b(zero);

                    for (int k = 0; k < i; ++k) a();
                    for (int k = 0; k < j; ++k) b();

                    check_equality(i == j, a, b);
                }
            }
        }
    }
}
