#pragma once

#include <functional>
#include "concepts.hpp"
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	struct Bomb final : NoCopy, NoMove, NoHeap
	{
		Bomb( function<void()> &&fn ) :
		  fn( std::move( fn ) )
		{
		}
		~Bomb() { fn(); }

	private:
		function<void()> fn;
	};
}

VM_END_MODULE()
