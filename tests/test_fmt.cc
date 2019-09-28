#include <gtest/gtest.h>
#include <fmt.hpp>

using namespace vm;

TEST( test_fmt, test_fmt )
{
	auto a = fmt( "{}", 128 );
	EXPECT_EQ( a, "128" );
	auto c = fmt( "{} + {} = {}", 2, a, 130 );
	EXPECT_EQ( c, "2 + 128 = 130" );

	// escape
	{
		auto b = fmt( "{{" );
		EXPECT_EQ( b, "{" );
		auto c = fmt( "}}" );
		EXPECT_EQ( c, "}" );
	}
}
