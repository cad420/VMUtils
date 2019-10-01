
#ifndef _IEVERYTHING_H_
#define _IEVERYTHING_H_
#include <cassert>
#include <type_traits>
#include <atomic>
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	class InterfaceID
	{
	public:
		bool operator==( const InterfaceID &id ) const
		{
			return true;
		}
	};

	class IRefCnt
	{
	public:
		virtual size_t AddStrongRef() = 0;
		virtual size_t ReleaseStrongRef() = 0;
		virtual size_t GetStrongRefCount() const = 0;
		virtual ~IRefCnt() = default;
	};

	class IEverything
	{
	public:
		IEverything( const IEverything & ) = delete;
		IEverything( const IEverything && ) = delete;
		IEverything &operator=( const IEverything & ) = delete;
		IEverything &operator=( const IEverything && ) = delete;

		virtual void QueryInterface( const InterfaceID &iid, IEverything **interface ) = 0;
		virtual size_t AddRef() = 0;
		virtual size_t Release() = 0;
		virtual ~IEverything() = default;

	protected:
		IEverything() = default;
	};

	template <typename Base>
	class RefCountedBase : public Base
	{
		template <typename T, typename U>
		friend class VMNew;

	public:
		RefCountedBase( IRefCnt *refCounter ) :
		  refCounter( refCounter )
		{
			//static_assert( std::is_base_of<IEverything, Base>::value );
			//static_assert( is_base_of<IEverything, Base>::value );
		}
		virtual size_t AddRef() override final
		{
			return refCounter->AddStrongRef();
		}
		virtual size_t Release() override final
		{
			return refCounter->ReleaseStrongRef();	// destroy the object in the implementation of a counter
		}
		size_t GetCount() const { return refCounter->GetStrongRefCount(); }

		~RefCountedBase()
		{
			assert( GetCount() == 0 );
		}

		IRefCnt *GetRefCounter() const { return refCounter; }

	protected:
		void operator delete( void *ptr )
		{
			delete[] reinterpret_cast<uint8_t *>( ptr );
		}

		template <typename Allocator>
		void operator delete( void *ptr, Allocator &alloc )
		{
			alloc.Free( ptr );
		}

	private:
		void *operator new( size_t size )
		{
			return new uint8_t[ size ];
		}

		template <typename Allocator>
		void *operator new( size_t size, Allocator &alloc )	 // placement new
		{
			return alloc.Alloc( size );
		}

		IRefCnt *const refCounter = nullptr;
	};

	template <typename Interface>
	class EverythingBase : public RefCountedBase<Interface>
	{
	public:
		EverythingBase( IRefCnt *cnt ) :
		  RefCountedBase<Interface>( cnt ) {}
		void QueryInterface( const InterfaceID &iid, IEverything **interface )
		{
			*interface = nullptr;
		}
	};
}

VM_END_MODULE()

#endif
