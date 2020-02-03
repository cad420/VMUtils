
#pragma once

#include "modules.hpp"

#include "ref.hpp"

VM_BEGIN_MODULE( vm )

VM_EXPORT
{
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

		explicit WeakRef( Ref<T> &p ) noexcept:
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

		WeakRef &operator=( const WeakRef & p ) noexcept
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
			return !(object != nullptr && cnt != nullptr && cnt->GetStrongRefCount() > 0);
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
