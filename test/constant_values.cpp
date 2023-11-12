#include "./config.h"

TEST_CASE( "Standard defined constant tests for mt19937 and mt19937_64" ) {
#if MT_TEST_SUPPORTS_WIDTH(32)
    SECTION( "mt19937" ) {
        test32 e(5489u);
        e.discard(9999u);
        REQUIRE( e() == 4123659995u );
    }
#endif
#if MT_TEST_SUPPORTS_WIDTH(64)
    SECTION( "mt19937_64" ) {
        test64 e(5489u);
        e.discard(9999u);
        REQUIRE( e() == 9981545732273789042u );
    }
#endif
}
