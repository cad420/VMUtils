#pragma once

#include <thread>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <vector>
#include <mutex>
#include <future>
#include <functional>
#include "modules.hpp"
#include "traits.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	struct ThreadPool
	{
		ThreadPool( size_t );
		~ThreadPool();

		template <typename F, typename... Args>
		auto AppendTask( F &&f, Args &&... args );
		void Wait();

	private:
		vector<thread> workers;
		queue<function<void()>> tasks;
		mutex mut;
		atomic<size_t> idle;
		condition_variable cond;
		condition_variable waitCond;
		size_t nthreads;
		bool stop;
	};

	// the constructor just launches some amount of workers
	inline ThreadPool::ThreadPool( size_t threads ) :
	  idle( threads ),
	  nthreads( threads ),
	  stop( false )
	{
		for ( size_t i = 0; i < threads; ++i )
			workers.emplace_back(
			  [this] {
				  while ( true ) {
					  function<void()> task;
					  {
						  unique_lock<mutex> lock( this->mut );
						  this->cond.wait(
							lock, [this] { return this->stop || !this->tasks.empty(); } );
						  if ( this->stop && this->tasks.empty() ) {
							  return;
						  }
						  idle--;
						  task = std::move( this->tasks.front() );
						  this->tasks.pop();
					  }
					  task();
					  idle++;
					  {
						  lock_guard<mutex> lk( this->mut );
						  if ( idle.load() == this->nthreads && this->tasks.empty() ) {
							  waitCond.notify_all();
						  }
					  }
				  }
			  } );
	}

	// add new work item to the pool
	template <class F, class... Args>
	auto ThreadPool::AppendTask( F && f, Args && ... args )
	{
		using return_type = typename InvokeResultOf<F>::type;
		auto task = make_shared<packaged_task<return_type()>>(
		  std::bind( std::forward<F>( f ), std::forward<Args>( args )... ) );
		future<return_type> res = task->get_future();
		{
			unique_lock<mutex> lock( mut );
			// don't allow enqueueing after stopping the pool
			if ( stop ) {
				throw runtime_error( "enqueue on stopped ThreadPool" );
			}
			tasks.emplace( [task]() { ( *task )(); } );
		}
		cond.notify_one();
		return res;
	}

	inline void ThreadPool::Wait()
	{
    mutex m;
    unique_lock<mutex> l(m);
		waitCond.wait( l, [this]() { return this->idle.load() == nthreads && tasks.empty(); } );
	}

	// the destructor joins all threads
	inline ThreadPool::~ThreadPool()
	{
		{
			unique_lock<mutex> lock( mut );
			stop = true;
		}
		cond.notify_all();
		for ( thread &worker : workers ) {
			worker.join();
		}
	}
}

VM_END_MODULE()
