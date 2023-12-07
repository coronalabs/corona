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

class ProfilingState;

class Profiling {
	public:
		enum { None = -1 };

		Profiling( Rtt_Allocator* allocator, const char* name );
		~Profiling();

	public:
		void Commit( ProfilingState& state );
		void Push( ProfilingState& state );
		const char* GetName() const { return fName; }
		Profiling* GetBelow() const { return fBelow; }
		void AddEntry( const char* name, bool isListName = false );
		int VisitEntries( lua_State* L ) const;

	public:
		static void ResetSums();
		static int VisitSums( lua_State* L );

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
			EntryRAII( ProfilingState& state, int& id, const char* name );
			~EntryRAII();

		public:
			void Add( const char* name ) const;

			Profiling* GetProfiling() const { return fProfiling; }

		private:
			ProfilingState& fState;
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
		static Sum* sFirstSum;

		Array<Entry>* fArray1;
		Array<Entry>* fArray2;
		Entry::ShortName fName;
		Profiling* fBelow;
};

class ProfilingState {
	public:
		ProfilingState( Rtt_Allocator* allocator );

	public:
		bool Find( const Profiling* profiling );
		int Create( const char* name );
		Profiling* GetByID( int id );
		Profiling* GetOrCreate( int& id, const char* name );
		Profiling* Get( const char* name );
		
	public:
		Profiling* Open( int id );
		void AddEntry( void* profiling, const char* name );
		void Close( void* profiling );

	public:
		static void Bookmark( short mark, int push, void* ud );

	public:
		Profiling* GetTopOfList() const { return fTopList; }
		void SetTopOfList( Profiling* profiling ) { fTopList = profiling; }

	private:
		PtrArray<Profiling> fLists;
		Profiling* fTopList;
};

#define PROFILE_SUMS 0 // include sums in profiling?

#if PROFILE_SUMS != 0
	#define SUMMED_TIMING( var, name ) static Profiling::Sum s_##var( name ); Profiling::SumRAII var##_w( s_##var )
	#define ENABLE_SUMMED_TIMING( enable ) Profiling::Sum::EnableSums( enable )
#else
	#define SUMMED_TIMING( var, name )
	#define ENABLE_SUMMED_TIMING( enable )
#endif

// Invalidate static indices on each launch:
#define PROFILING_BEGIN( state, runtime, var, name )	static int s_id_##var, s_id_##var##_run = -1;			\
														if ( s_id_##var##_run != runtime.GetGenerationID() )	\
														{														\
															s_id_##var = Profiling::None;						\
															s_id_##var##_run = runtime.GetGenerationID();		\
														}														\
														Profiling::EntryRAII var( state, s_id_##var, name )

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Profiling_H__
