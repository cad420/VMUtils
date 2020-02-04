#pragma once

#include <atomic>
#include "ieverything.hpp"
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	template <typename T>
	class Ref final
	{
		T *rawPtr;
		using RefEverythingType = IEverything;
		template <typename Other>
		friend class Ref;

		class WeakRefRAII
		{
			T *rawPtr;
			Ref *ref;

			WeakRefRAII( const WeakRefRAII & ) = delete;
			WeakRefRAII &operator=( const WeakRefRAII & ) = delete;
			WeakRefRAII &operator=( WeakRefRAII && ) = delete;

		public:
			WeakRefRAII( Ref &ptr ) noexcept :
			  rawPtr( static_cast<T *>( ptr ) ), ref( std::addressof( ptr ) )
			{
			}

			WeakRefRAII( WeakRefRAII &&other ) noexcept :
			  rawPtr( other.rawPtr ), ref( other.ref )
			{
				other.rawPtr = nullptr;
				other.ref = nullptr;
			}
			~WeakRefRAII()
			{
				if ( ref && static_cast<T *>( *ref ) != rawPtr ) {
					ref->Attach( rawPtr );
				}
			}

			T *&operator*() noexcept { return rawPtr; }
			const T *operator*() const noexcept { return rawPtr; }

			operator T **() noexcept { return &rawPtr; }
			operator const T **() const noexcept { return &rawPtr; }
		};

	public:
		WeakRefRAII operator&()
		{
			return WeakRefRAII( *this );
		}

		const WeakRefRAII operator&() const
		{
			return WeakRefRAII( *this );
		}

	public:
		Ref( T *p = nullptr ) :
		  rawPtr( p )
		{
			if ( rawPtr != nullptr ) {
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

		template <typename U>
		Ref( const Ref<U> &r )
		{
			//	static_assert( is_base_of<IEverything, T>::value );
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

		Ref( Ref &&r ) noexcept
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

		template <typename Derived, typename = typename enable_if<is_base_of<T, Derived>::value>::type>
		Ref &operator=( const Ref<Derived> &r )
		{
			return *this = static_cast<T *>( r.rawPtr );
		}

		template <typename Derived, typename = typename enable_if<is_base_of<T, Derived>::value>::type>
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

		template <typename U>
		U *Get()
		{
			return static_cast<U *>( rawPtr );
		}

		template <typename U>
		const U *Get() const
		{
			return rawPtr;
		}

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
	inline bool operator==( nullptr_t A, const Ref<T> &B )
	{
		return !B;
	}

	template <class T>
	bool operator==( const Ref<T> &A, nullptr_t B )
	{
		return B == A;
	}

	template <class T>
	bool operator!=( nullptr_t A, const Ref<T> &B )
	{
		return !( A == B );
	}

	template <class T>
	bool operator!=( const Ref<T> &A, nullptr_t B )
	{
		return !( A == B );
	}

	template <typename T>
	class WeakRef
	{
		IRefCnt *cnt = nullptr;
		T *object = nullptr;

	public:
		WeakRef( T *object = nullptr ) noexcept :
		  object( object )
		{
			if ( object ) {
				cnt = object->GetRefCounter();
				cnt->AddWeakRef();
			}
		}

		explicit WeakRef( Ref<T> &p ) noexcept :
		  cnt( p ? p->GetRefCounter() : nullptr ), object( static_cast<T *>( p ) )
		{
			if ( cnt ) {
				cnt->AddWeakRef();
			}
		}

		WeakRef( const WeakRef &p ) noexcept :
		  cnt( p.cnt ), object( p.object )
		{
			if ( cnt ) {
				cnt->AddWeakRef();
			}
		}

		WeakRef( WeakRef &&p ) noexcept :
		  cnt( std::move( p.cnt ) ), object( std::move( p.object ) )
		{
			p.cnt = nullptr;
			p.object = nullptr;
		}

		WeakRef &operator=( const WeakRef &p ) noexcept
		{
			if ( *this == p )
				return *this;
			Release();

			object = p.object;
			cnt = p.cnt;
			if ( cnt ) {
				cnt->AddWeakRef();
			}
			return *this;
		}

		WeakRef &operator=( T *object ) noexcept
		{
			return operator=( WeakRef( object ) );
		}

		WeakRef &operator=( Ref<T> &p ) noexcept
		{
			Release();
			object = static_cast<T *>( p );
			cnt = object ? object->GetRefCounter() : nullptr;
			if ( cnt ) {
				cnt->AddWeakRef();
			}
			return *this;
		}

		WeakRef &operator=( WeakRef &&p ) noexcept
		{
			if ( *this == p )
				return *this;

			Release();
			object = std::move( p.object );
			cnt = std::move( p.cnt );
			p.object = nullptr;
			p.cnt = nullptr;
			return *this;
		}

		bool operator==( const WeakRef &other ) noexcept
		{
			return cnt == other.cnt;
		}

		bool operator!=( const WeakRef &other ) noexcept
		{
			return cnt != other.cnt;
		}

		bool Expired() const
		{
			return !( object != nullptr && cnt != nullptr && cnt->GetStrongRefCount() > 0 );
		}

		Ref<T> Lock()
		{
			Ref<T> ret;
			if ( cnt ) {
				Ref<IEverything> p;
				cnt->GetObject( &p );
				if ( p ) {
					ret = object;  // why
				} else {
					Release();
				}
			}
			return ret;
		}

		void Release()
		{
			if ( cnt ) {
				cnt->ReleaseWeakRef();
			}
			object = nullptr;
			cnt = nullptr;
		}

		~WeakRef()
		{
			Release();
		}
	};
}

VM_END_MODULE()
