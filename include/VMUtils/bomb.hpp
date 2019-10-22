#pragma once

#include <functional>
#include "concepts.hpp"
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

template <typename F>
struct Bomb final : NoCopy, NoHeap
{
	Bomb( F &&fn ) :
	  fn( std::move( fn ) )
	{
	}
	Bomb( Bomb && ) = default;
	Bomb &operator=( Bomb && ) = default;

	~Bomb() { fn(); }

private:
	F fn;
};

VM_EXPORT
{
	template <typename F>
	auto make_bomb( F && fn )
	{
		return Bomb<F>( std::forward<F>( fn ) );
	}
}

VM_END_MODULE()
