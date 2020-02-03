#pragma once

#include <memory>
#include <mutex>
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
	enum class ObjectState
	{
		UNINITIALIZED,
		ALIVE,
		DESTROYED
	};

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

		void QueryInterface( const InterfaceID &iid, IEverything **interface )
		{
			m_pObject->QueryInterface( iid, interface );
		}

	private:
		IEverything *const m_pObject = nullptr;	 // this should be a specified type by using template
		IAllocator *const m_allocator = nullptr;
	};

	template <typename T, typename U>
	friend class VMNew;

public:
	size_t AddStrongRef() override final
	{
		return ++m_counter;
	}
	/**
	 * \brief 
	 */
	size_t ReleaseStrongRef() override final
	{
		const size_t cnt = --m_counter;
		if ( cnt == 0 ) {
			// destroy the object is a non-trivial routine. Because we must consider the manner of
			// GetObject() in multi-thread scenario
			//

			assert( ObjectState::ALIVE == objectState );

			const auto ref = m_counter.load();	// ref == 0 or ref == 1

			// case 1: ref == 0 indicates that there are no other threads using the object

			// case 2: ref == 1 indicates that GetObject() is being invoked in another thread.
			// because when GetObject() is called, the strong reference counter is increased and decreased.

			assert( ref == 0 || ref == 1 );

			using namespace std;
			unique_lock<mutex> lck( mtx );

			// when enter the critical section , the strong reference counter must be zero
			//assert( m_counter == 0 && ObjectState::ALIVE == objectState );
			if ( m_counter == 0 && ObjectState::ALIVE == objectState ) {
				uint8_t bufCopy[ bufSize ];
				memcpy( bufCopy, objectBuffer, bufSize );

				auto *p = reinterpret_cast<ObjectWrapper *>( bufCopy );

				objectState = ObjectState::DESTROYED;

				const auto deleteSelf = m_weakCounter == 0;
				lck.unlock();

				p->DestroyObject();

				if ( deleteSelf )
					delete this;
			}
			//reinterpret_cast<ObjectWrapper *>( objectBuffer )->DestroyObject();

			//if (m_weakCounter == 0)
			//	delete this;  // delete the ref counter object
		}
		return cnt;
	}
	size_t GetStrongRefCount() const override final
	{
		return m_counter;
	}

	size_t AddWeakRef() override final
	{
		return ++m_weakCounter;
	}

	size_t ReleaseWeakRef() override final
	{
		// Generally, When strong reference counter and weak reference
		// counter both are 0,the counter object is need to be destroyed.

		using namespace std;

		unique_lock<mutex> lck( mtx );

		const auto weakCnt = --m_weakCounter;

		if ( weakCnt == 0 && objectState == ObjectState::DESTROYED ) {
			assert( m_counter.load() == 0 );
			lck.unlock();
			delete this;
		}

		return weakCnt;
	}

	/**
	 * \brief This function is used to increase the reference count
	 * \note Because the process of destroying is not atomic. so anther
	 * locker should be used here
	 */
	void GetObject( IEverything **object ) override final
	{
		if ( objectState != ObjectState::ALIVE )
			return;

		using namespace std;
		unique_lock<mutex> lck( mtx );

		const auto strongCnt = ++m_counter;

		// Increasing the strong reference counter indicates that the object buffer is being used by this thread
		// so that not to be destroyed by ReleaseStrongRef() in another thread. see the ReleaseStrongRef() for details.
		if ( objectState == ObjectState::ALIVE && strongCnt > 1 ) {
			auto p = reinterpret_cast<ObjectWrapper *>( objectBuffer );
			p->QueryInterface( Everything_IID, object );
		}
		--m_counter;
	}

	size_t GetWeakRefCount() const override final
	{
		return m_weakCounter;
	}

private:
	RefCounterImpl() = default;
	template <typename ObjectType>
	void Init( Allocator *allocator, ObjectType *obj )
	{
		assert( objectState == ObjectState::UNINITIALIZED );
		new ( objectBuffer ) ObjectWrapper( obj, allocator );
		objectState = ObjectState::ALIVE;
	}

	void TryDestoryObject()
	{
	}

	atomic_size_t m_counter = { 0 };
	atomic_size_t m_weakCounter = { 0 };
	static size_t constexpr bufSize = sizeof( ObjectWrapper );
	uint8_t objectBuffer[ bufSize ];
	ObjectState objectState = ObjectState::UNINITIALIZED;
	std::mutex mtx;
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
			if ( m_allocator ) {
				obj = new ( *m_allocator ) ObjectType( refcnt.get(), std::forward<Args>( args )... );
			}
			else {
				obj = new ObjectType( refcnt.get(), std::forward<Args>( args )... );

			}

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
