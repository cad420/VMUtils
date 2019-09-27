
#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <thread>
#include <condition_variable>
#include <queue>
#include <vector>
#include <mutex>
#include <future>

namespace ysl
{
class ThreadPool
{
public:
	ThreadPool( size_t );
	template <class F, class... Args>
	auto AppendTask( F &&f, Args &&... args );
	~ThreadPool();

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex mut;
	std::condition_variable cond;
	bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool( size_t threads ) :
  stop( false )
{
	for ( size_t i = 0; i < threads; ++i )
		workers.emplace_back(
		  [this] {
			  for ( ;; ) {
				  std::function<void()> task;

				  {
					  std::unique_lock<std::mutex> lock( this->mut );
					  this->cond.wait( lock,
									   [this] { return this->stop || !this->tasks.empty(); } );
					  if ( this->stop && this->tasks.empty() )
						  return;
					  task = std::move( this->tasks.front() );
					  this->tasks.pop();
				  }
				  task();
			  }
		  } );
}

// add new work item to the pool
template <class F, class... Args>
auto ThreadPool::AppendTask( F &&f, Args &&... args )
{
	using return_type = std::invoke_result_t<F, Args...>;
	auto task = std::make_shared<std::packaged_task<return_type()>>( std::bind( std::forward<F>( f ), std::forward<Args>( args )... ) );
	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock( mut );
		// don't allow enqueueing after stopping the pool
		if ( stop )
			throw std::runtime_error( "enqueue on stopped ThreadPool" );
		tasks.emplace( [task]() { ( *task )(); } );
	}
	cond.notify_one();
	return res;
}
// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock( mut );
		stop = true;
	}
	cond.notify_all();
	for ( std::thread &worker : workers )
		worker.join();
}

}  // namespace ysl

#endif
