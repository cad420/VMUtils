#include <gtest/gtest.h>
#include <VMUtils/ref.hpp>
#include <VMUtils/ieverything.hpp>
#include <VMUtils/vmnew.hpp>

using namespace vm;
using namespace std;

vector<int> x;

struct MyAllocator : IAllocator
{
	void *Alloc( size_t size ) override
	{
		x.emplace_back( 1 );
		return new uint8_t[ size ];
	}
	void Free( void *ptr ) override
	{
		x.emplace_back( 2 );
		delete[] reinterpret_cast<uint8_t *>( ptr );
	}
};

// Define an interface based IEverything
struct A : IEverything
{
	virtual void Foo() = 0;
};

// Actual implementation
class AImpl : public EverythingBase<A>
{
	int a = 0;

public:
	AImpl( IRefCnt *cnt, int a ) :
	  EverythingBase<A>( cnt ), a( a )	// The first parameter of the ctor must be IRefCnt*
	{
	}
	void Foo() override
	{
		cout << a << " "
			 << "Oops\n";
	}
};

TEST( test_ref, test_ref )
{
	{
		auto p = VM_NEW<AImpl>( 1 );
		Ref<A> a = p;
		Ref<A> b( p );
		Ref<A> c( a );
		Ref<A> d( c.Get() );

		MyAllocator myAlloc;
		Ref<A> e = VM_NEW<MyAllocator, AImpl>( &myAlloc, 1 );
	}
	EXPECT_EQ( x, ( vector<int>{ 1, 2 } ) );
}
