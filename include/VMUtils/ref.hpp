
#ifndef _REF_H_
#define _REF_H_

#include <atomic>
#include "ieverything.hpp"
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

VM_EXPORT
{
	template <typename T>
	class Ref
	{
		T *rawPtr;
		using RefEverythingType = IEverything;
		template <typename Other>
		friend class Ref;

	public:
		Ref( T *p = nullptr ) :
		  rawPtr( p )
		{
			if ( rawPtr != nullptr ) 
			{
				static_cast<RefEverythingType *>( rawPtr )->AddRef();
			}
		}

		Ref( const Ref &r )
		{
		//	static_assert( std::is_base_of<IEverything, T>::value );
			rawPtr = r.rawPtr;
			if ( rawPtr ) {
				static_cast<RefEverythingType *>( rawPtr )->AddRef();
			}
		}
		

		template<typename U>
		Ref( const Ref<U> &r )
		{
		//	static_assert( std::is_base_of<IEverything, T>::value );
			rawPtr = r.rawPtr;
			if ( rawPtr ) {
				static_cast<RefEverythingType *>( rawPtr )->AddRef();
			}
		}

		template <typename U>
		Ref<U> Cast()
		{
			return Ref<U>( dynamic_cast<U *>( Get() ) );
		}

		Ref( const Ref &&r ) noexcept
		{
			rawPtr = r.Take();
		}

		void Attach( T *p )
		{
			Release();
			rawPtr = p;
		}

		Ref &operator=( T *p )
		{
			if ( rawPtr != p ) {
				if ( rawPtr ) {
					static_cast<RefEverythingType *>( rawPtr )->Release();
				}
				rawPtr = p;
				if ( rawPtr ) {
					static_cast<RefEverythingType *>( p )->AddRef();
				}
			}
			return *this;
		}

		Ref &operator=( const Ref &r )
		{
			return *this = r.rawPtr;
		}

		Ref &operator=( Ref &&r ) noexcept
		{
			if ( rawPtr != r.rawPtr ) {
				Attach( r.Take() );
			}
			return *this;
		}

		template <typename Derived, typename = typename std::enable_if<std::is_base_of<T, Derived>::value>::type>
		Ref &operator=( const Ref<Derived> &r )
		{
			return *this = static_cast<T *>( r.rawPtr );
		}

		template <typename Derived, typename = typename std::enable_if<std::is_base_of<T, Derived>::value>::type>
		Ref &operator=( Ref<Derived> &&r ) noexcept
		{
			if ( rawPtr != r.rawPtr ) {
				Attach( r.Take() );
			}
			return *this;
		}

		const T &operator*() const { return *rawPtr; }
		T &operator*() { return *rawPtr; }

		const T *operator->() const { return rawPtr; }
		T *operator->() { return rawPtr; }

		operator T *() { return rawPtr; }
		operator const T *() const { return rawPtr; }

		T *Get() { return rawPtr; }

		const T *Get() const { return rawPtr; }

		void Reset()
		{
			Release();
			rawPtr = nullptr;
		}

		T *Take()
		{
			T *p = rawPtr;
			rawPtr = nullptr;
			return p;
		}
		void Release()
		{
			if ( rawPtr ) {
				static_cast<RefEverythingType *>( rawPtr )->Release();
			}
		}
		~Ref()
		{
			Release();
		}
	};

	// ==, != operators for two different Ref types with different orders

	template <class T, class U>
	inline bool operator==( const Ref<T> &A,
							const Ref<U> &B )
	{
		return A.Get() == B.Get();
	}

	template <class T, class U>
	inline bool operator!=( const Ref<T> &A,
							const Ref<U> &B )
	{
		return A.Get() != B.Get();
	}

	template <class T, class U>
	inline bool operator==( const Ref<T> &A, U *B )
	{
		return A.Get() == B;
	}

	template <class T, class U>
	inline bool operator!=( const Ref<T> &A, U *B )
	{
		return A.Get() != B;
	}
	template <class T, class U>
	inline bool operator==( T *A, const Ref<U> &B )
	{
		return A == B.Get();
	}

	template <class T, class U>
	inline bool operator!=( T *A, const Ref<U> &B )
	{
		return A != B.Get();
	}

	// ==, != for nullptr and Ref<T> with different orders

	template <class T>
	inline bool operator==( std::nullptr_t A, const Ref<T> &B )
	{
		return !B;
	}

	template <class T>
	bool operator==( const Ref<T> &A, std::nullptr_t B )
	{
		return B == A;
	}

	template <class T>
	bool operator!=( std::nullptr_t A, const Ref<T> &B )
	{
		return !( A == B );
	}

	template <class T>
	bool operator!=( const Ref<T> &A, std::nullptr_t B )
	{
		return !( A == B );
	}
}


VM_END_MODULE()

#endif