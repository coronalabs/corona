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
		enum { None = -1 };

		Profiling( Rtt_Allocator* allocator, const char* name );
		~Profiling();

	public:
		void Commit();
		void Push();
		const char* GetName() const { return fName; }
		Profiling* GetBelow() const { return fBelow; }
		void AddEntry( const char* name, bool isListName = false );
		int VisitEntries( lua_State* L ) const;

	private:
		static void Bookmark( short mark, int push );

	public:
		static Profiling* Open( int id );
		static void AddEntry( void* profiling, const char* name );
		static void Close( void* profiling );
		static void ResetSums();
		static int VisitSums( lua_State* L );

	public:
		static Profiling* GetFirst();

	public:
		static bool Find( const Profiling* profiling );
		static void Init( struct lua_State* L, Rtt_Allocator* allocator );
		static int DestroyAll( struct lua_State* L );
		static int Create( Rtt_Allocator* allocator, const char* name );
		static Profiling* Get( const char* name );

	public:
		#ifdef Rtt_AUTHORING_SIMULATOR
			static int GetSimulatorRun() { return sSimulatorRun; }
		#endif

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
			EntryRAII( Rtt_Allocator* allocator, int& id, const char* name );
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
		static PtrArray<Profiling>* sLists;
		static Profiling* sTopList;
		static Sum* sFirstSum;

	#ifdef Rtt_AUTHORING_SIMULATOR
		static int sSimulatorRun;
	#endif

		Array<Entry>* fArray1;
		Array<Entry>* fArray2;
		Entry::ShortName fName;
		Profiling* fBelow;
};

#define PROFILE_SUMS 0 // include sums in profiling?

#if PROFILE_SUMS != 0
	#define SUMMED_TIMING( var, name ) static Profiling::Sum s_##var( name ); Profiling::SumRAII var##_w( s_##var )
	#define ENABLE_SUMMED_TIMING( enable ) Profiling::Sum::EnableSums( enable )
#else
	#define SUMMED_TIMING( var, name )
	#define ENABLE_SUMMED_TIMING( enable )
#endif

#ifdef Rtt_AUTHORING_SIMULATOR
	// Invalidate static indices on each simulator launch:
	#define PROFILING_BEGIN( allocator, var, name ) static int s_id_##var, s_id_##var##_run = Profiling::None;	\
													if ( s_id_##var##_run != Profiling::GetSimulatorRun() )		\
													{															\
														s_id_##var = Profiling::None;							\
														s_id_##var##_run = Profiling::GetSimulatorRun();		\
													}															\
													Profiling::EntryRAII var( allocator, s_id_##var, name )
#else
	// Otherwise, compute each index once up front:
	#define PROFILING_BEGIN( allocator, var, name ) static int s_id_##var = Profiling::None; Profiling::EntryRAII var( allocator, s_id_##var, name )
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Profiling_H__
