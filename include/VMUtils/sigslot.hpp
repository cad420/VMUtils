#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

/**
 * \brief The internal class is used to manage the destruction of the slot.
 *
 * When a slot instance is deleted, the destructor will automatically remove
 * itself from the callback list of the corresponding signal.
 */

VM_EXPORT
{
	template <typename RetType, typename Type, typename... Args>
	function<RetType( Args... )> BindMember( Type * instance,
											 RetType( Type::*method )( Args... ) )
	{
		return [=]( Args... args ) -> RetType {
			return ( instance->*method )( std::forward<Args>( args )... );
		};	// why not std::bind() ???
	}
}

template <typename FuncType>
struct _Signal_pImpl;

struct _Slot_pImpl_Base
{
	_Slot_pImpl_Base() = default;
	_Slot_pImpl_Base( const _Slot_pImpl_Base & ) = delete;
	_Slot_pImpl_Base &operator=( const _Slot_pImpl_Base & ) = delete;
	virtual ~_Slot_pImpl_Base() {}
};

template <typename FuncType>
struct _Slot_pImpl : _Slot_pImpl_Base
{
	_Slot_pImpl( const weak_ptr<_Signal_pImpl<FuncType>> signal,
				 const function<FuncType> &func ) :
	  signal( signal ),
	  func( func )
	{
	}
	~_Slot_pImpl()
	{
		// remove itself from the callback list of the signal
		auto real = signal.lock();
		if ( real ) {
			for ( auto it = real->slots.begin(); it != real->slots.end(); ++it ) {
				if ( it->expired() || it->lock().get() == this )  // delete the function which is already expired (The owner class has been deleted) or this one
				{
					it = real->slots.erase( it );
					if ( it == real->slots.end() )	// delete all
					{
						break;
					}
				}
			}
		}
	}
	weak_ptr<_Signal_pImpl<FuncType>> signal;
	function<FuncType> func;
};

/**
 * \brief  The internal class is used for sharing the slots between the assignment of two signals.
 */
template <typename FuncType>
struct _Signal_pImpl
{
	vector<weak_ptr<_Slot_pImpl<FuncType>>> slots;
};

VM_EXPORT
{
	/**
 * \brief This class is only used as a class member, or it will leading a dangle callback
 */
	struct Slot final
	{
		Slot() = default;
		~Slot() = default;
		template <typename Ty>
		explicit Slot( Ty impl ) :
		  impl( impl )
		{
		}
		operator bool() const
		{
			return static_cast<bool>( impl );
		}

	private:
		shared_ptr<_Slot_pImpl_Base> impl;
	};

	template <typename FuncType>
	struct Signal final
	{
		Signal() :
		  impl( make_shared<_Signal_pImpl<FuncType>>() )
		{
		}
		template <typename... Args>
		void operator()( Args &&... args )
		{
			const auto &slots = impl->slots;
			for ( const auto &each : slots ) {
				if ( auto slot = each.lock() ) {
					slot->func( std::forward<Args>( args )... );
				}
			}
		}

		Slot Connect( const function<FuncType> &func )
		{
			auto slot = make_shared<_Slot_pImpl<FuncType>>( impl, func );
			impl->slots.push_back( slot );
			return Slot( slot );
		}

		template <typename InstanceType, class MemberFuncType>
		Slot Connect( InstanceType instance, MemberFuncType func )
		{
			return Connect( BindMember( instance, func ) );
		}

	private:
		shared_ptr<_Signal_pImpl<FuncType>> impl;
	};
}

VM_END_MODULE()
