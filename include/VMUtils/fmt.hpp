#pragma once

#include <string>
#include <sstream>
#include <utility>
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

struct FmtImpl
{
	template <typename T, typename... Rest>
	static void apply( ostringstream &os, string const &raw,
					   const char *patt, T &&t, Rest &&... rest )
	{
		patt = move_fn(
		  patt, raw, [&]( auto _ ) { os << t; }, [&]( auto _ ) { os << _; } );
		if ( patt ) {
			apply( os, raw, patt, std::forward<Rest>( rest )... );
		}
	}
	static void apply( ostringstream &os, string const &raw, const char *patt )
	{
		while ( move_fn(
		  patt, raw, [&]( auto _ ) {}, [&]( auto _ ) { os << _; } ) )
			;
	}
	template <typename F, typename G>
	static const char *move_fn( const char *patt, string const &raw, F &&f, G &&g )
	{
		for ( auto p = patt; *p; ++p ) {
			switch ( *p ) {
			case '{': {
				if ( p[ 1 ] != '{' ) {
					ostringstream os;
					for ( auto q = p + 1; *q; ++q ) {
						if ( *q == '}' ) {
							if ( q[ 1 ] != '}' ) {
								f( os.str() );
								return q + 1;
							}
							++q;
						}
						os << *q;
					}
					// should be error {
					throw std::logic_error( "invalid format template: \"" + raw + "\"" );
				}
				++p;
			} break;
			case '}': {
				if ( p[ 1 ] != '}' ) {
					throw std::logic_error( "invalid format template: \"" + raw + "\"" );
				}
				++p;
			} break;
			}
			g( *p );
		}
		return nullptr;
	}
};

struct Fmt
{
	template <typename... Args>
	static string apply( string const &patt, Args &&... args )
	{
		ostringstream os;
		FmtImpl::apply( os, patt, patt.data(), std::forward<Args>( args )... );
		return os.str();
	}
};

VM_EXPORT
{
	template <typename... Args>
	string fmt( string const &patt, Args &&... args )
	{
		return Fmt::apply( patt, std::forward<Args>( args )... );
	}
}

VM_END_MODULE()
