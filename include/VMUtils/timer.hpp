#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <functional>

namespace vm
{
namespace __inner__
{
namespace __exported__
{
}
using namespace __exported__;
using namespace std;
using namespace chrono;

namespace time_unit
{
struct S
{
	using type = ratio<1>;
	static constexpr const char *c_str() { return "s"; }
};
struct Ms
{
	using type = milli;
	static constexpr const char *c_str() { return "ms"; }
};
struct Us
{
	using type = micro;
	static constexpr const char *c_str() { return "us"; }
};
struct Ns
{
	using type = nano;
	static constexpr const char *c_str() { return "ns"; }
};

}  // namespace time_unit

template <typename Unit>
struct UnittedDuration final
{
	auto cnt() const { return _.count(); }
	operator double() const { return cnt(); }
	string fmt() const
	{
		ostringstream os;
		os << cnt() << "(" << Unit::c_str() << ")";
		return os.str();
	}

	UnittedDuration( duration<double, typename system_clock::duration::period> const &_ ) :
	  _( duration_cast<decltype( this->_ )>( _ ) )
	{
	}

public:
	friend ostream &operator<<( ostream &os, UnittedDuration const &_ )
	{
		return os << _.fmt();
	}

private:
	duration<double, typename Unit::type> _;
};

struct Duration final
{
	auto s() const { return UnittedDuration<time_unit::S>( _ ); }
	auto ms() const { return UnittedDuration<time_unit::Ms>( _ ); }
	auto us() const { return UnittedDuration<time_unit::Us>( _ ); }
	auto ns() const { return UnittedDuration<time_unit::Ns>( _ ); }

	Duration() = default;
	template <typename Rep, typename Period>
	Duration( duration<Rep, Period> const &_ ) :
	  _( _ )
	{
	}

public:
	friend ostream &operator<<( ostream &os, Duration const &_ )
	{
		return os << _.ms();
	}

private:
	duration<double, typename system_clock::duration::period> _;
};

struct Scoped;

namespace __exported__
{
struct Timer final
{
	Timer() = default;

	using Scoped = __inner__::Scoped;

public:
	void start()
	{
		end_ = begin_ = system_clock::now();
	}
	void stop()
	{
		end_ = system_clock::now();
		duration_ = end_ - begin_;
	}

public:
	auto duration() const { return duration_; }
	auto elapsed() const { return Duration( system_clock::now() - begin_ ); }

public:
	double eval_remaining_time( float cur_percent ) const
	{
		return 1.0 * elapsed().us().cnt() * ( 1.0 - cur_percent ) / ( cur_percent );
	}
	auto eval_total_time( float cur_percent ) const
	{
		return elapsed().us().cnt() / cur_percent;
	}
	auto last_begin_time_point() const
	{
		return begin_;
	}
	void print() const
	{
		cout << "Duration:" << duration().ms() << ".\n";
	}

private:
	time_point<system_clock> begin_, end_;
	Duration duration_;
};

}  // namespace __exported__

struct Scoped final
{
	Scoped( std::function<void( Duration const & )> &&fn ) :
	  fn( std::move( fn ) )
	{
		_.start();
	}
	~Scoped()
	{
		_.stop();
		fn( _.duration() );
	}

private:
	Timer _;
	std::function<void( Duration const & )> fn;
};

}  // namespace __inner__

using namespace __inner__::__exported__;

}  // namespace vm
