#include <gtest/gtest.h>
#include <VMUtils/fmt.hpp>

using namespace vm;
using namespace std;

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

	{
		ostringstream os;
		fprint( os, "{} a, b", 1, 2 );
		EXPECT_EQ( os.str(), "1 a, b" );
		fprintln( os, "{}{}", "qw", "1" );
		EXPECT_EQ( os.str(), "1 a, bqw1\n" );
	}
}
