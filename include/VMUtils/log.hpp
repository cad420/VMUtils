#pragma once

#include <cassert>
#include "fmt.hpp"
#include "modules.hpp"

#ifdef NDEBUG
#define Assert( expr ) ( (void)( 0 ) )

#else
#define Assert( expr ) (((expr) == true)?((void)(0))(assert(expr)))
#endif


VM_BEGIN_MODULE( vm )

template <typename... Args>
inline void dump( const string &patt, bool same_line, Args &&... args )
{
	auto v = fmt( patt, std::forward<Args>( args )... );
	fprintf( stderr, same_line ? "%s\r" : "%s\n", v.c_str() );
}

VM_EXPORT
{
	template <typename... Args>
	void Error( const string &patt, Args &&... args )
	{
		dump( patt, false, std::forward<Args>( args )... );
		abort();
	}

	template <typename... Args>
	void Warning( const string &patt, Args &&... args )
	{
		dump( patt, false, std::forward<Args>( args )... );
	}

	template <typename... Args>
	void Log( const string &patt, Args &&... args )
	{
		dump( patt, false, std::forward<Args>( args )... );
	}

	template <typename... Args>
	void Display( const string &patt, Args &&... args )
	{
		dump( patt, true, std::forward<Args>( args )... );
		fprintf( stderr, "\n" );
	}

	template <typename... Args>
	void Debug( const string &patt, Args &&... args )
	{
#ifndef NDEBUG
		dump( patt, false, std::forward<Args>( args )... );
#else
		void( 0 );
#endif
	}

}  // namespace __exported__

VM_END_MODULE()
