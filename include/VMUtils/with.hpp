#pragma once

#include <functional>
#include "concepts.hpp"
#include "bomb.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	template <typename T>
	struct With final : NoCopy, NoHeap
	{
		template <typename F>
		void with( T &_, F fn )
		{
			auto old = this->_;
			auto x = make_bomb( [this, old] { this->_ = old; } );
			this->_ = &_;
			fn();
		}
		template <typename U, typename F,
				  typename = typename std::enable_if<std::is_constructible<T, U>::value>::type>
		void with( U &&_, F fn )
		{
			auto old = this->_;
			auto x = make_bomb( [this, old] { this->_ = old; } );
			T e( std::forward<U>( _ ) );
			this->_ = &e;
			fn();
		}

		T &operator*() const { return *_; }
		T *operator->() const { return _; }

		bool has() const { return _; }

	private:
		T *_ = nullptr;
	};
}

VM_END_MODULE()
