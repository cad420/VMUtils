

#include <VMUtils/concurrency.hpp>
#include <VMUtils/threadpool.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace vm;

mutex mut;

TEST( test_concurrent, SyncQueue )
{
	ThreadPool pubThread( 1 );
	ThreadPool subThread( 1 );

	auto syncQueue = std::make_shared<BlockingQueue<int>>( 1 );

	const auto cases = 10000;

	auto result = std::make_shared<std::vector<pair<int,int>>>(cases);
	for ( auto &r : *result ) {
		r.first = r.second = 0;
	}

	for ( int i = 0; i < cases; i++ ) {
		pubThread.AppendTask( [syncQueue, result]( int idx ) {
			syncQueue->Put( idx );
			( *result )[ idx ].first += 1;
		},
							  i );

		subThread.AppendTask( [syncQueue, result]() {
			const auto t = syncQueue->Take();
			( *result )[ t ].second += 1;
		} );
	}

	pubThread.Wait();
	subThread.Wait();

	bool ok = true;
	
	for ( const auto &p : (*result) ) {
		if ( !( p.first == p.second && p.first == 1 ) ) {
			ok = false;
			break;
		}
	}

	EXPECT_TRUE( ok );
	
	//EXPECT_EQ( producer, consumer );
	
}