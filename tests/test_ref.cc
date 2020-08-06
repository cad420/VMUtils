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
struct Bar : IEverything
{
	virtual void Foo() = 0;
};


 //Actual implementation
class AImpl : public EverythingBase<Bar>
{
	int a = 0;
	const char *info = nullptr;

public:
	AImpl( IRefCnt *cnt, int a ,const char * str = nullptr) :
	  EverythingBase<Bar>( cnt ), a( a ),info(str)	// The first parameter of the ctor must be IRefCnt*
	{
		std::cout << "number: " << a << " ,string: " << ( !str ? "" : str ) << std::endl;
	}
	void Foo() override
	{
		std::cout << "AImpl::Foo()"<<std::endl;
	}
	~AImpl()noexcept
	{
		cout << (!info?"":info) << " has been destroyed\n";
	}
};

TEST( test_ref, test_ref )
{
	{
		std::cout << "sizeof(AImpl) is: " << sizeof( AImpl ) << std::endl;
		auto p = VM_NEW<AImpl>( 1,"p" );
		Ref<Bar> a = p;
		Ref<Bar> b( p );
		Ref<Bar> c( a );
		Ref<Bar> d( c.Get() );

		MyAllocator myAlloc;
		Ref<Bar> e = VM_NEW<MyAllocator, AImpl>( &myAlloc, 1,"e" );
	}
	EXPECT_EQ( x, ( vector<int>{ 1, 2 } ) );
}

TEST( test_ref, WeakRef )
{
	WeakRef<AImpl> wp1, wp2;
	{
		auto p = VM_NEW<AImpl>( 1 );
		Ref<AImpl> sp = p;
		wp1 = sp;
		wp2 = p;
		EXPECT_FALSE( wp1.Expired());
		EXPECT_FALSE( wp2.Expired());

		auto sp1 = wp1.Lock();
		EXPECT_TRUE( sp1.Get() != nullptr );
	}


	EXPECT_TRUE( wp1.Expired() );
	EXPECT_TRUE( wp2.Expired() );

}
