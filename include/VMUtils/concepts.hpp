#pragma once

#include <utility>

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
struct NoMove
{
	NoMove() = default;

	NoMove( NoMove const & ) = default;
	NoMove &operator=( NoMove const & ) = default;
	NoMove( NoMove && ) = delete;
	NoMove &operator=( NoMove && ) = delete;
};

struct NoCopy
{
	NoCopy() = default;

	NoCopy( NoCopy && ) = default;
	NoCopy &operator=( NoCopy && ) = default;
	NoCopy( NoCopy const & ) = delete;
	NoCopy &operator=( NoCopy const & ) = delete;
};

struct NoHeap
{
private:
	static void *operator new( size_t );
	static void *operator new[]( size_t );
};

struct Dynamic
{
	virtual ~Dynamic() = default;
};

}  // namespace __exported__

}  // namespace __inner__

using namespace __inner__::__exported__;

}  // namespace vm
