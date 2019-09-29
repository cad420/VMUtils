#ifndef _VMNEW_H_
#define _VMNEW_H_

#include "ieverything.hpp"
#include "modules.hpp"

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
	template <typename ObjectType, typename Allocator>
	class ObjectWrapper
	{
	public:
		ObjectWrapper( ObjectType *obj, Allocator *allocator ) :
		  m_pObject( obj ),
		  m_allocator( allocator ) {}
		void DestroyObject() const
		{
			if ( m_allocator ) {
				m_pObject->~ObjectType();
				m_allocator->Free( m_pObject );
			} else {
				delete m_pObject;
			}
		}

	private:
		ObjectType *const m_pObject = nullptr;
		Allocator *const m_allocator = nullptr;
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
			reinterpret_cast<ObjectWrapper<IEverything, IAllocator> *>( objectBuffer )->DestroyObject();
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
	template <typename Allocator, typename ObjectType>
	void Init( Allocator *allocator, ObjectType *obj )
	{
		new ( objectBuffer ) ObjectWrapper<IEverything, IAllocator>( obj, allocator );
	}

	atomic_size_t m_counter = 0;
	static size_t constexpr bufSize = sizeof( ObjectWrapper<IEverything, IAllocator> );
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
			auto refCnt = new RefCounterImpl<Allocator>();

			ObjectType *obj = nullptr;
			try {
				if ( m_allocator )
					obj = new ( *m_allocator ) ObjectType( refCnt, std::forward<Args>( args )... );
				else
					obj = new ObjectType( refCnt, std::forward<Args>( args )... );

				refCnt->Init( m_allocator, obj );

			} catch ( ... ) {
				delete refCnt;
			}

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

#endif