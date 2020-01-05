
#pragma once
#include "modules.hpp"
#include <list>
#include <mutex>
#include <condition_variable>
VM_BEGIN_MODULE(vm)
using namespace std;


VM_EXPORT
{
	template <typename E>
	class BlockingQueue
	{
		list<E> queue{};
		const size_t maxSize = 0;
		mutex mut;
		condition_variable notFull;
		condition_variable notEmpty;

	public:
		BlockingQueue( size_t maxSize ) :
		  maxSize( maxSize ),
		  queue() {}

		template <typename U = E>
		void Put( U &&e )
		{
			unique_lock<mutex> lk( mut );
			notFull.wait( lk, [this]() { return queue.size() < maxSize; } );
			queue.push_back( std::forward<U>( e ) );
			notEmpty.notify_one();
		}

		E Take()
		{
			unique_lock<mutex> lk( mut );
			notEmpty.wait( lk, [this]() { return !queue.empty(); } );
			E e = std::move(queue.front());
			queue.pop_front();

			notFull.notify_one();
			return e;
		}
	};

}





VM_END_MODULE()