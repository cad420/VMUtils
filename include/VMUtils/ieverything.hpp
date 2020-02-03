#pragma once

#include <cassert>
#include <cstring>
#include <type_traits>
#include <atomic>
#include "modules.hpp"


VM_BEGIN_MODULE( vm )

using namespace std;



VM_EXPORT
{
	template <typename T, typename U>
	class VMNew;

	class IEverything;

	class InterfaceID
	{
	public:
		uint32_t Part0;		 // 4 bytes
		uint16_t Part1;		 // 2 bytes
		uint16_t Part2;		 // 2 bytes
		uint8_t Part3[ 8 ];	 // 8 bytes
		bool operator==( const InterfaceID &id ) const
		{
			return Part0 == id.Part0 && Part1 == id.Part1 && Part2 == id.Part2 && std::memcmp( Part3, id.Part3, sizeof( Part3 ) ) == 0;
		}
	};
	
	static constexpr InterfaceID Everything_IID = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

	class IRefCnt
	{
	public:
		virtual size_t AddStrongRef() = 0;
		virtual size_t ReleaseStrongRef() = 0;
		virtual size_t GetStrongRefCount() const = 0;
		virtual size_t AddWeakRef() = 0;
		virtual size_t ReleaseWeakRef() = 0;
		virtual size_t GetWeakRefCount() const = 0;
		virtual void GetObject( IEverything **object ) = 0;
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
		virtual IRefCnt *GetRefCounter()const = 0;
		
		//virtual size_t AddWeakRef() = 0;
		//virtual size_t ReleaseWeakRef() = 0;

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
		size_t GetCount() const{ return refCounter->GetStrongRefCount(); }


		~RefCountedBase()
		{
			assert( GetCount() == 0 );
		}

		IRefCnt *GetRefCounter() const override{ return refCounter; }

	protected:
		void operator delete( void *ptr )
		{
			delete[] reinterpret_cast<uint8_t *>( ptr );
			//::operator delete( ptr );
		}

		template <typename Allocator>
		void operator delete( void *ptr, Allocator &alloc )
		{
			alloc.Free( ptr );
		}

	private:
		void *operator new( size_t size )
		{
			void *t = new uint8_t[ size ];
			return t;
			//return ::operator new( size );
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
		void QueryInterface( const InterfaceID &iid, IEverything **interface )override
		{
			if ( interface == nullptr )
				return;

			*interface = nullptr;
			if ( iid == Everything_IID ) {
				*interface = this;
				( *interface )->AddRef();
			}
		}
	};
}

VM_END_MODULE()
