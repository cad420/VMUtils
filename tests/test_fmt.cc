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

TEST( test_fmt, test_fmt_with_syntax )
{
	// align
	EXPECT_EQ( fmt( "{x<8}", 15 ), "15xxxxxx" );
	EXPECT_EQ( fmt( "{x>8}", 15 ), "xxxxxx15" );
	EXPECT_EQ( fmt( "{x<4}", 15000000 ), "15000000" );
	EXPECT_EQ( fmt( "{x<8}", -1.56 ), "-1.56xxx" );
	EXPECT_EQ( fmt( "{x>8}", -1.56 ), "xxx-1.56" );
	EXPECT_EQ( fmt( "{x<4}", -1.56456 ), "-1.56456" );
	// sign
	EXPECT_EQ( fmt( "{+} {+} {+} {+}", 1.88, -1.88, 1, -1 ), "+1.88 -1.88 +1 -1" );	
	// int
	EXPECT_EQ( fmt( "{#X}", 15 ), "0xF" );
	EXPECT_EQ( fmt( "{#x}", 15 ), "0xf" );
	EXPECT_EQ( fmt( "{#o}", 15 ), "0o17" );
	EXPECT_EQ( fmt( "{#b}", 15 ), "0b0000000000000000000000000000000000000000000000000000000000001111" );
	// fp
	EXPECT_EQ( fmt( "{.2}", 2.34567 ), "2.35" );
	EXPECT_EQ( fmt( "{.12}", 2.34567 ), "2.345670000000" );
}
