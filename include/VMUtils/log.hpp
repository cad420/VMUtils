#pragma once

#include <cassert>
#include "fmt.hpp"

#ifdef NDEBUG
#define Assert( expr ) ( (void)( 0 ) )
#else
#define Assert( expr ) (((expr) == true)?((void)(0))(assert(expr)))
#endif

namespace vm
{
namespace __inner__
{
template <typename... Args>
inline void dump( const string &patt, bool same_line, Args &&... args )
{
	fprintf( stderr, same_line ? "%s\r" : "%s\n",
			 fmt( patt, std::forward<Args>( args )... ).c_str() );
}

namespace __exported__
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

}  // namespace __inner__

using namespace __inner__::__exported__;

}  // namespace vm
