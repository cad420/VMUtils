#include <gtest/gtest.h>
#include <VMUtils/ref.hpp>
#include <VMUtils/ieverything.hpp>
#include <VMUtils/vmnew.hpp>
#include <VMUtils/threadpool.hpp>
#include <VMUtils/timer.hpp>
#include <random>

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
class BarImpl : public EverythingBase<Bar>
{
	int a = 0;
	const char *info = nullptr;

public:
	BarImpl( IRefCnt *cnt, int a, const char *str = nullptr ) :
	  EverythingBase<Bar>( cnt ), a( a ), info( str )  // The first parameter of the ctor must be IRefCnt*
	{
		std::cout << "number: " << a << " ,string: " << ( !str ? "" : str ) << std::endl;
	}
	void Foo() override
	{
		std::cout << "AImpl::Foo()" << std::endl;
	}
	~BarImpl() noexcept
	{
		cout << ( !info ? "" : info ) << " has been destroyed\n";
	}
};

TEST( test_ref, test_ref )
{
	{
		std::cout << "sizeof(AImpl) is: " << sizeof( BarImpl ) << std::endl;
		auto p = VM_NEW<BarImpl>( 1, "p" );
		Ref<Bar> a = p;
		Ref<Bar> b( p );
		Ref<Bar> c( a );
		Ref<Bar> d( c.Get() );

		MyAllocator myAlloc;
		Ref<Bar> e = VM_NEW<MyAllocator, BarImpl>( &myAlloc, 1, "e" );
	}
	EXPECT_EQ( x, ( vector<int>{ 1, 2 } ) );
}

TEST( test_ref, WeakRef )
{
	WeakRef<BarImpl> wp1, wp2;
	{
		auto p = VM_NEW<BarImpl>( 1 );
		Ref<BarImpl> sp = p;
		wp1 = sp;
		wp2 = p;
		EXPECT_FALSE( wp1.Expired() );
		EXPECT_FALSE( wp2.Expired() );

		auto sp1 = wp1.Lock();
		EXPECT_TRUE( sp1.Get() != nullptr );
	}

	EXPECT_TRUE( wp1.Expired() );
	EXPECT_TRUE( wp2.Expired() );
}

TEST( test_ref, multi_thread_scenerio )
{
	using namespace vm;
	using std::default_random_engine;
	using std::uniform_int_distribution;

	auto threadNum = 10;
	ThreadPool pool( threadNum );

	Ref<Bar> ptr = VM_NEW<BarImpl>( 5, "BarImpl Pointer" );

	for ( int i = 0; i < threadNum; i++ ) {
		pool.AppendTask( [ = ]() {
			default_random_engine e;
			e.seed( Timer::current().cnt() );
			uniform_int_distribution<int> u;
			std::vector<Ref<Bar>> vec;
			int count = 10;
			vm::Timer t;
			while ( count-- ) {
				if ( u( e ) > 0 ) {
					vec.push_back( Ref<Bar>( ptr ) );
				} else {
					if ( vec.size() != 0 ) {
						vec.back() = nullptr;
						vec.pop_back();
					} else {
						vec.push_back( Ref<Bar>( ptr ) );
					}
				}
				std::this_thread::sleep_for( 0.5s );
			}
			std::cout << "Thread: " << i << " finished\n";
		} );
	}
	pool.Wait();
	const auto refcnt = ptr->GetRefCounter()->GetStrongRefCount();
	EXPECT_EQ( refcnt, 1 );
}

