#pragma once

#include <memory>
#include "ieverything.hpp"
#include "modules.hpp"
#include "bomb.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	struct IAllocator
	{
		virtual void *Alloc( size_t size ) = 0;
		virtual void Free( void *ptr ) = 0;
		virtual ~IAllocator() = default;
	};

	template <typename ObjectType, typename Allocator>
	class VMNew;
}

template <typename Allocator>
class RefCounterImpl : public IRefCnt
{
	class ObjectWrapper
	{
	public:
		ObjectWrapper( IEverything *obj, IAllocator *allocator ) :
		  m_pObject( obj ),
		  m_allocator( allocator ) {}
		void DestroyObject() const
		{
			if ( m_allocator ) {
				m_pObject->~IEverything();
				m_allocator->Free( m_pObject );
			} else {
				delete m_pObject;
			}
		}

	private:
		IEverything *const m_pObject = nullptr;
		IAllocator *const m_allocator = nullptr;
	};

	template <typename T, typename U>
	friend class VMNew;

public:
	size_t AddStrongRef() override
	{
		return ++m_counter;
	}
	size_t ReleaseStrongRef() override
	{
		const size_t cnt = --m_counter;
		if ( cnt == 0 ) {
			reinterpret_cast<ObjectWrapper *>( objectBuffer )->DestroyObject();
			delete this;  // delete the ref counter object
		}
		return cnt;
	}
	size_t GetStrongRefCount() const override
	{
		return m_counter;
	}

private:
	RefCounterImpl() = default;
	template <typename ObjectType>
	void Init( Allocator *allocator, ObjectType *obj )
	{
		new ( objectBuffer ) ObjectWrapper( obj, allocator );
	}

	atomic_size_t m_counter = { 0 };
	static size_t constexpr bufSize = sizeof( ObjectWrapper );
	uint8_t objectBuffer[ bufSize ];
};

VM_EXPORT
{
	template <typename ObjectType, typename Allocator>
	class VMNew
	{
		Allocator *const m_allocator;

	public:
		VMNew( Allocator *allocator ) :
		  m_allocator( allocator ) {}

		VMNew( const VMNew & ) = delete;
		VMNew( VMNew && ) = delete;
		VMNew &operator=( const VMNew & ) = delete;
		VMNew &operator=( VMNew && ) = delete;

		template <typename... Args>
		ObjectType *operator()( Args &&... args )
		{
			auto refcnt = unique_ptr<RefCounterImpl<Allocator>>( new RefCounterImpl<Allocator> );

			ObjectType *obj = nullptr;
			if ( m_allocator )
				obj = new ( *m_allocator ) ObjectType( refcnt.get(), std::forward<Args>( args )... );
			else
				obj = new ObjectType( refcnt.get(), std::forward<Args>( args )... );

			refcnt->Init( m_allocator, obj );

			refcnt.release();
			return obj;
		}
	};
}

VM_END_MODULE()

template <typename Allocator, typename Type, typename... Args>
Type *VM_NEW( Allocator *alloc, Args &&... args )
{
	using namespace vm;
	return VMNew<Type, Allocator>( alloc )( std::forward<Args>( args )... );
}

template <typename Type, typename... Args>
Type *VM_NEW( Args &&... args )
{
	using namespace vm;
	return VMNew<Type, IAllocator>( nullptr )( std::forward<Args>( args )... );
}
