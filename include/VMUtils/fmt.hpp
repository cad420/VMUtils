#pragma once

#include <string>
#include <sstream>
#include <utility>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <type_traits>
#include "bomb.hpp"
#include "modules.hpp"
#include "option.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	struct FmtSpec
	{
		static FmtSpec from_string( string const &spec )
		{
			auto _ = FmtSpec{};
			auto p = spec.data();
			// while ( *p ) {
			switch ( *p ) {
			case '<':
			case '^':
			case '>': p = get_alignment( p, _ ); break;
			case '+': spec.sign = true;
			case '#':
			case '0':
			case '.':
			default: {
			} break;
			case 0: break;
			}
			// }
			return _;
		}

	private:
		static char const *get_alignment( char const *p, FmtSpec &spec )
		{
			spec.align = FmtSpec::Align{};
			switch ( p[ 1 ] ) {
			case '<':
			case '>':
			case '^':
				spec.align.value().fill = *p++;
			default: {
				switch ( *p++ ) {
				case '<': spec.align.value().type = FmtSpec::Align::Type::Left; break;
				case '>': spec.align.value().type = FmtSpec::Align::Type::Right; break;
				case '^': spec.align.value().type = FmtSpec::Align::Type::Center; break;
				}
			}
			}
			return p;
		}

	public:
		struct Align
		{
			enum class Type : char
			{
				Left = 0,
				Center = 1,
				Right = 2
			};
			Type type;
			Option<char> fill;
		};
		enum class Alter : char
		{
			Debug = 0,
			LowerHex = 1,
			UpperHex = 2,
			Bin = 3,
			Oct = 4
		};

	public:
		Option<Align> align;
		bool sign;
		Option<Alter> alter;
		bool zero = false;
		Option<size_t> width;
		Option<size_t> precision;
	};
}

template <typename T, bool IsInt, bool IsFloat>
struct FmtDefault;
template <typename T>
struct FmtDefault<T, false, false>
{
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		os << t;
	}
};
template <typename T>
struct FmtDefault<T, true, false>  //int
{
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		auto old = os.flags();
		Bomb bprec( [&] { os.flags( old ); } );
		os.flags( std::ios::dec );
		bool upper = false;
		if ( spec.alter.has_value() ) {
			switch ( spec.alter.value() ) {
			case FmtSpec::Alter::Bin:  // bin is not implemented
				os << setbase( 2 );
				break;
			case FmtSpec::Alter::Oct:
				os << std::oct;
				break;
			case FmtSpec::Alter::UpperHex:
				upper = true;
			case FmtSpec::Alter::LowerHex:
				os << std::hex;
				break;
			}
		}
		os << t;
	}
};
template <typename T>
struct FmtDefault<T, false, true>  //fp
{
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		auto prec = os.precision();
		Bomb bprec( [&] { os.precision( prec ); } );
		if ( spec.precision.has_value() ) {
			os.precision( spec.precision.value() );
		}
		os << t;
	}
};

template <typename T>
struct FmtStrategy : FmtDefault<T, is_integral<T>::value, is_floating_point<T>::value>
{
};

template <typename T>
void fmt_impl( ostream &os, T &&t, FmtSpec const &spec = FmtSpec{} )
{
	using RmCV = typename remove_cv<T>::type;
	using RmRef = typename remove_reference<RmCV>::type;
	FmtStrategy<RmRef>::apply( os, std::forward<T>( t ), spec );
}

template <typename T>
void fmt_array( ostream &os, T const &t, size_t len, string const &spec )
{
	os << "[";
	if ( len ) {
		fmt_impl( os, t[ 0 ] );
	}
	for ( auto i = 1; i < len; ++i ) {
		os << ", ";
		fmt_impl( os, t[ i ] );
	}
	os << "]";
}

template <typename T, typename A>
struct FmtStrategy<vector<T, A>>
{
	static void apply( ostream &os, vector<T, A> const &t, FmtSpec const &spec )
	{
		fmt_array( os, t, t.size(), spec );
	}
};
template <typename T, size_t N>
struct FmtStrategy<array<T, N>>
{
	static void apply( ostream &os, array<T, N> const &t, FmtSpec const &spec )
	{
		fmt_array( os, t, N, spec );
	}
};
template <typename T, size_t N>
struct FmtStrategy<T[ N ]>
{
	static void apply( ostream &os, T t[ N ], FmtSpec const &spec )
	{
		fmt_array( os, t, N, spec );
	}
};
template <size_t N>
struct FmtStrategy<char[ N ]> : FmtStrategy<char *>
{
};
template <size_t N>
struct FmtStrategy<const char[ N ]> : FmtStrategy<const char *>
{
};
template <size_t N>
struct FmtStrategy<volatile char[ N ]> : FmtStrategy<volatile char *>
{
};
template <size_t N>
struct FmtStrategy<const volatile char[ N ]> : FmtStrategy<const volatile char *>
{
};

template <typename T>
void fmt_map( ostream &os, T const &t, string const &spec )
{
	os << "{";
	bool first = true;
	for ( auto &e : t ) {
		auto &k = e.first;
		auto &v = e.second;
		if ( !first ) {
			os << ", ";
		}
		fmt_impl( os, k );
		os << ": ";
		fmt_impl( os, v );
		first = false;
	}
	os << "}";
}

template <typename K, typename V, typename C, typename A>
struct FmtStrategy<map<K, V, C, A>>
{
	static void apply( ostream &os, map<K, V, C, A> const &t, FmtSpec const &spec )
	{
		fmt_map( os, t, spec );
	}
};
template <typename K, typename V, typename H, typename E, typename A>
struct FmtStrategy<unordered_map<K, V, H, E, A>>
{
	static void apply( ostream &os, unordered_map<K, V, H, E, A> const &t, FmtSpec const &spec )
	{
		fmt_map( os, t, spec );
	}
};

template <size_t K, size_t N>
struct FmtAtK
{
	template <typename T>
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		os << ", ";
		fmt_impl( os, std::get<K>( t ), spec );
		FmtAtK<K + 1, N>::apply( os, t, spec );
	}
};
template <size_t N>
struct FmtAtK<0, N>
{
	template <typename T>
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		os << "(";
		fmt_impl( os, std::get<0>( t ), spec );
		FmtAtK<1, N>::apply( os, t, spec );
	}
};
template <size_t N>
struct FmtAtK<N, N>
{
	template <typename T>
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		os << ")";
	}
};
template <>
struct FmtAtK<0, 0>
{
	template <typename T>
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		os << "()";
	}
};
template <typename A, typename B>
struct FmtStrategy<pair<A, B>>
{
	static void apply( ostream &os, pair<A, B> const &t, FmtSpec const &spec )
	{
		FmtAtK<0, 2>::apply( os, t, spec );
	}
};
template <typename... Args>
struct FmtStrategy<tuple<Args...>>
{
	static void apply( ostream &os, tuple<Args...> const &t, FmtSpec const &spec )
	{
		FmtAtK<0, sizeof...( Args )>::apply( os, t, spec );
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
		  [&]( auto _ ) { fmt_impl( os, std::forward<T>( t ), FmtSpec::from_string( _ ) ); },
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
	// fmt
	template <typename... Args>
	string fmt( string const &patt, Args &&... args )
	{
		ostringstream os;
		FmtImpl::apply( os, patt, patt.data(), std::forward<Args>( args )... );
		return os.str();
	}
	// print
	template <typename... Args>
	void fprint( ostream & os, string const &patt, Args &&... args )
	{
		FmtImpl::apply( os, patt, patt.data(), std::forward<Args>( args )... );
	}
	template <typename... Args>
	void print( string const &patt, Args &&... args )
	{
		fprint( cout, patt, std::forward<Args>( args )... );
	}
	template <typename... Args>
	void eprint( string const &patt, Args &&... args )
	{
		fprint( cerr, patt, std::forward<Args>( args )... );
	}
	// println
	template <typename... Args>
	void fprintln( ostream & os, string const &patt, Args &&... args )
	{
		fprint( os, patt, std::forward<Args>( args )... );
		os << endl;
	}
	template <typename... Args>
	void println( string const &patt, Args &&... args )
	{
		fprintln( cout, patt, std::forward<Args>( args )... );
	}
	template <typename... Args>
	void eprintln( string const &patt, Args &&... args )
	{
		fprintln( cerr, patt, std::forward<Args>( args )... );
	}
}

VM_END_MODULE()
