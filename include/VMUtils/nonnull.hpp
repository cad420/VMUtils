#pragma once

#include <memory>
#include <utility>

#include "concepts.hpp"
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	template <typename T, typename>
	struct Option;

	template <typename T>
	struct NonNull final
	{
		using Reference = T const &;
		using Pointer = T const *;

		NonNull( Pointer _ ) :
		_( _ ) {}
		NonNull( nullptr_t ) = delete;

		Reference operator*() const { return *_; }
		Pointer operator->() const { return _; }

		Pointer get() const { return _; }

	private:
		NonNull() = default;

	private:
		Pointer _ = nullptr;
		template <typename X, typename>
		friend struct Option;
	};

	template <typename T>
	struct Box final
	{
		using Reference = T &;
		using Pointer = T *;

		explicit Box( Pointer _ ) :
		_( _ ) {}
		Box( nullptr_t ) = delete;

		Reference operator*() const { return *_; }
		Pointer operator->() const { return &*_; }

		Pointer get() const { return _.get(); }

	private:
		Box() = default;

	private:
		unique_ptr<T> _;
		template <typename X, typename>
		friend struct Option;
	};

	template <typename T>
	struct Arc final
	{
		using Reference = T &;
		using Pointer = T *;

		explicit Arc( Pointer _ ) :
		_( _ ) {}
		Arc( nullptr_t ) = delete;

		Reference operator*() const { return *_; }
		Pointer operator->() const { return &*_; }

		Pointer get() const { return _.get(); }

		template <typename U, typename = typename enable_if<is_base_of<U, T>::value>::type>
		Arc<U> downcast() const
		{
			return Arc<U>( _.get() );
		}

	private:
		Arc() = default;

	private:
		shared_ptr<T> _;
		template <typename X, typename>
		friend struct Option;
	};

}

VM_END_MODULE()
