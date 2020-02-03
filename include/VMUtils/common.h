
#pragma once

#include <memory>

template <typename T>
static inline T *GetImplPtrHelper( T *ptr )
{
	return ptr;
}
template <typename Wrapper>
static inline typename Wrapper::pointer GetImplPtrHelper( const Wrapper &p )
{
	return p.data();
}

#define VM_DECL_IMPL( Class )                                                                                                           \
	inline Class##__pImpl *d_func() { return reinterpret_cast<Class##__pImpl *>( GetImplPtrHelper( d_ptr.get() ) ); }                   \
	inline const Class##__pImpl *d_func() const { return reinterpret_cast<const Class##__pImpl *>( GetImplPtrHelper( d_ptr.get() ) ); } \
	friend class Class##__pImpl;                                                                                                        \
	std::unique_ptr<Class##__pImpl> const d_ptr;

#define VM_DECL_API( Class )                                                           \
	inline Class *q_func() { return static_cast<Class *>( q_ptr ); }                   \
	inline const Class *q_func() const { return static_cast<const Class *>( q_ptr ); } \
	friend class Class;                                                                \
	Class *const q_ptr = nullptr;

#define VM_IMPL( Class ) Class##__pImpl *const _ = d_func();
#define VM_API ( Class ) Class *const _ = q_func();

