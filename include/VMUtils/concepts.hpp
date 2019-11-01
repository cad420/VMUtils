#pragma once

#include <atomic>
#include <utility>
#include <cstdint>
#include "modules.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	struct NoMove
	{
		NoMove() = default;

		NoMove( NoMove const & ) = default;
		NoMove &operator=( NoMove const & ) = default;
		NoMove( NoMove && ) = delete;
		NoMove &operator=( NoMove && ) = delete;
	};

	struct NoCopy
	{
		NoCopy() = default;

		NoCopy( NoCopy && ) = default;
		NoCopy &operator=( NoCopy && ) = default;
		NoCopy( NoCopy const & ) = delete;
		NoCopy &operator=( NoCopy const & ) = delete;
	};

	struct NoHeap
	{
	private:
		static void *operator new( size_t );
		static void *operator new[]( size_t );
	};

	struct Dynamic
	{
		virtual ~Dynamic() = default;
	};

	template <typename ClassName>
	class CountedBase
	{
	public:
		static uint64_t ObjectCount() { return objectNum; }
		virtual ~CountedBase()
		{
			--objectNum;
		}

	protected:
		CountedBase() { init(); }
		CountedBase( const CountedBase &obj ) { init(); }
		CountedBase &operator=( const CountedBase &obj )
		{
			init();
			return *this;
		}
		CountedBase( CountedBase && ) = default;
		CountedBase &operator=( CountedBase && ) = default;

	private:
		static std::atomic<uint64_t> objectNum;
		static void init() { ++objectNum; }
	};
}

VM_END_MODULE()
