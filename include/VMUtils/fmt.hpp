#pragma once

#include <string>
#include <sstream>
#include <utility>
#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <type_traits>
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

template <typename T>
struct FmtStrategy
{
	static void apply( ostream &os, T const &t )
	{
		os << t;
	}
};

template <typename T>
int fmt_impl_one( ostream &os, T &&t )
{
	using RmCV = typename remove_cv<T>::type;
	using RmRef = typename remove_reference<RmCV>::type;
	FmtStrategy<RmRef>::apply( os, std::forward<T>( t ) );
	return 0;
}

template <typename... Args>
void fmt_impl( ostream &os, Args &&... args )
{
	int _[] = { fmt_impl_one( os, std::forward<Args>( args ) )... };
}

template <typename T>
void fmt_array( ostream &os, T const &t, size_t len )
{
	fmt_impl( os, "[" );
	if ( len )
	{
		fmt_impl( os, t[0] );
	}
	for ( auto i = 1; i < len; ++i )
	{
		fmt_impl( os, ", ", t[i] );
	}
	fmt_impl( os << "]" );
}

template <typename T, typename A>
struct FmtStrategy<vector<T, A>>
{
	static void apply( ostream &os, vector<T, A> const &t )
	{
		fmt_array( os, t, t.size() );
	}
};
template <typename T, size_t N>
struct FmtStrategy<array<T, N>>
{
	static void apply( ostream &os, array<T, N> const &t )
	{
		fmt_array( os, t, N );
	}
};
template <typename T, size_t N>
struct FmtStrategy<T[N]>
{
	static void apply( ostream &os, T t[N] )
	{
		fmt_array( os, t, N );
	}
};
template <size_t N>
struct FmtStrategy<char[N]>: FmtStrategy<char*>
{
};
template <size_t N>
struct FmtStrategy<const char[N]>: FmtStrategy<const char*>
{
};
template <size_t N>
struct FmtStrategy<volatile char[N]>: FmtStrategy<volatile char*>
{
};
template <size_t N>
struct FmtStrategy<const volatile char[N]>: FmtStrategy<const volatile char*>
{
};

template <typename T>
void fmt_map( ostream &os, T const &t )
{
	fmt_impl( os, "{" );
	bool first = true;
	for ( auto &e: t )
	{
		auto &k = e.first;
		auto &v = e.second;
		if ( !first )
		{
			fmt_impl( os, ", " );
		}
		fmt_impl( os, k, ": ", v );
		first = false;
	}
	fmt_impl( os, "}" );
}

template <typename K, typename V, typename C, typename A>
struct FmtStrategy<map<K, V, C, A>>
{
	static void apply( ostream &os, map<K, V, C, A> const &t )
	{
		fmt_map( os, t );
	}
};
template <typename K, typename V, typename H, typename E, typename A>
struct FmtStrategy<unordered_map<K, V, H, E, A>>
{
	static void apply( ostream &os, unordered_map<K, V, H, E, A> const &t )
	{
		fmt_map( os, t );
	}
};

template <size_t K, size_t N>
struct FmtAtK
{
	template <typename T>
	static void apply( ostream &os, T const &t )
	{
		fmt_impl( os, ", ", std::get<K>( t ) );
		FmtAtK<K + 1, N>::apply( os, t );
	}
};
template <size_t N>
struct FmtAtK<0, N>
{
	template <typename T>
	static void apply( ostream &os, T const &t )
	{
		fmt_impl( os, "(", std::get<0>( t ) );
		FmtAtK<1, N>::apply( os, t );
	}
};
template <size_t N>
struct FmtAtK<N, N>
{
	template <typename T>
	static void apply( ostream &os, T const &t )
	{
		fmt_impl( os, ")" );
	}
};
template <>
struct FmtAtK<0, 0>
{
	template <typename T>
	static void apply( ostream &os, T const &t )
	{
		fmt_impl( os, "()" );
	}
};
template <typename A, typename B>
struct FmtStrategy<pair<A, B>>
{
	static void apply( ostream &os, pair<A, B> const &t )
	{
		FmtAtK<0, 2>::apply( os, t );
	}
};
template <typename... Args>
struct FmtStrategy<tuple<Args...>>
{
	static void apply( ostream &os, tuple<Args...> const &t )
	{
		FmtAtK<0, sizeof...( Args )>::apply( os, t );
	}
};

struct FmtImpl
{
	template <typename T, typename... Rest>
	static void apply( ostream &os, string const &raw, const char *patt,
					   T &&t, Rest &&... rest )
	{
		patt = move_fn(
		  patt, raw, 
		  [&]( auto _ ) { fmt_impl( os, std::forward<T>( t ) ); }, 
		  [&]( auto _ ) { os << _; } );
		if ( patt ) {
			apply( os, raw, patt, std::forward<Rest>( rest )... );
		}
	}
	static void apply( ostream &os, string const &raw, const char *patt )
	{
		while ( patt = move_fn(
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

VM_EXPORT
{
	template <typename... Args>
	string fmt( string const &patt, Args &&... args )
	{
		ostringstream os;
		FmtImpl::apply( os, patt, patt.data(), std::forward<Args>( args )... );
		return os.str();
	}
	template <typename... Args>
	void println( string const &patt, Args &&... args )
	{
		FmtImpl::apply( cout, patt, patt.data(), std::forward<Args>( args )... );
		cout << endl;
	}
	template <typename... Args>
	void eprintln( string const &patt, Args &&... args )
	{
		FmtImpl::apply( cerr, patt, patt.data(), std::forward<Args>( args )... );
		cerr << endl;
	}
}

VM_END_MODULE()
