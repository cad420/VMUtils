#include <vector>
#include <gtest/gtest.h>
#include <VMUtils/timer.hpp>

using namespace vm;

TEST( test_timer, test_timer )
{
	Timer t;
	t.start();
	t.stop();
	
	auto s = t.duration().s();
	auto ms = t.duration().ms();
	auto us = t.duration().us();
	auto ns = t.duration().ns();

	std::cout << s << std::endl;
	std::cout << ms << std::endl;
	std::cout << us << std::endl;
	std::cout << ns << std::endl;

	EXPECT_DOUBLE_EQ( ms, 1000 * s );
	EXPECT_DOUBLE_EQ( us, 1000 * ms );
	EXPECT_DOUBLE_EQ( ns, 1000 * us );


  auto tp = Timer::current();
  std::cout << tp <<std::endl;
  std::ostringstream ss;
  ss << tp;

  std::ostringstream ss1;
  ss1 << tp.to("%c");
  
  EXPECT_STREQ(ss.str().c_str(), ss1.str().c_str());

}

TEST( test_scoped, test_scoped )
{
	using namespace std;
	vector<int> v;
	{
		Timer::Scoped _( [&]( auto dt ) {
			v.emplace_back( 0 );
		} );
		for ( int i = 1; i < 5; ++i ) {
			Timer::Scoped _( [&]( auto dt ) {
				v.emplace_back( i );
			} );
		}
	}
	EXPECT_EQ( v, ( vector<int>{ 1, 2, 3, 4, 0 } ) );
}
