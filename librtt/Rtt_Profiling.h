//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Profiling_H__
#define _Rtt_Profiling_H__

#include "Core/Rtt_Array.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

struct lua_State;
struct Rtt_Allocator;

namespace Rtt
{

// ----------------------------------------------------------------------------

class Profiling {
	public:
		Profiling( Rtt_Allocator* allocator, const char* name );
		~Profiling();

	public:
		void Commit();
		void Push();
		void AddEntry( const char* name, bool isListName = false );
		int VisitEntries( lua_State* L ) const;

		static Profiling* Open( Rtt_Allocator* allocator, const char* name );
		static void AddEntry( void* profiling, const char* name );
		static void Close( void* profiling );
		static void ResetSums();
		static int VisitSums( lua_State* L );
	
	public:
		static bool Find( const Profiling* profiling );
		static int DestroyAll( struct lua_State* L );
		static Profiling* GetOrCreate( Rtt_Allocator* allocator, const char* name );
		static Profiling* Get( const char* name );

	private:
		struct Entry {
			typedef char ShortName[64];
		
			static void SetShortName( ShortName out, const char* name );
			static U64 SublistTime() { return ~0ULL; }
		
			void SetName( const char* name );
		
			ShortName fName;
			U64 fTime;
		};

	public:
		class EntryRAII {
		public:
			EntryRAII( Rtt_Allocator* allocator, const char* name );
			~EntryRAII();

		public:
			void Add( const char* name ) const;

			Profiling* GetProfiling() const { return fProfiling; }

		private:
			Profiling* fProfiling;
		};
	
		class Sum {
		public:
			Sum( const char* name );
			
			void AddTiming( U64 diff );
			bool Acquire();
			bool Release();

			static void EnableSums( bool newValue );

		private:
			void Reset();
			
			static bool fEnabled; // make sums no-op in not-yet-profiled scopes (not ideal)

		private:
			Sum* fNext;
			Entry::ShortName fName;
			U64 fTotalTime;
			U32 fTimingCount;
			U32 fRefCount;

			friend class Profiling;
		};

		class SumRAII {
		public:
			SumRAII( Sum& sum );
			~SumRAII();
			
		private:
			Sum& fSum;
			U64 fBegan;
		};

	private:
		static Profiling* fFirstList;
		static Profiling* fTopList;
		static Sum* fFirstSum;

		Array<Entry>* fArray1;
		Array<Entry>* fArray2;
		Entry::ShortName fName;
		Profiling* fNext;
		Profiling* fBelow;
};

#define PROFILE_SUMS 1 // set this to non-0 to include sums in profiling

#ifdef PROFILE_SUMS
	#define SUMMED_TIMING( var, name ) static Profiling::Sum var( name ); Profiling::SumRAII var##w( var )
	#define ENABLE_SUMMED_TIMING( enable ) Profiling::Sum::EnableSums( enable )
#else
	#define SUMMED_TIMING( var, name )
	#define ENABLE_SUMMED_TIMING( enable )
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Profiling_H__
