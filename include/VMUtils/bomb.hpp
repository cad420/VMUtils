#pragma once

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

namespace __exported__
{
struct Bomb final
{
	Bomb( function<void()> &&fn ) :
	  fn( std::move( fn ) )
	{
	}
	~Bomb() { fn(); }

private:
	function<void()> fn;
};

}  // namespace __exported__

}  // namespace __inner__

using namespace __inner__::__exported__;

}  // namespace vm
