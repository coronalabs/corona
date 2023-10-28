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
		enum { None = -1 }; // <- STEVE CHANGE

		Profiling( Rtt_Allocator* allocator, const char* name );
		~Profiling();

	public:
		void Commit();
		void Push();
		void AddEntry( const char* name, bool isListName = false );
		int VisitEntries( lua_State* L ) const;
// STEVE CHANGE
	private:
		static void Bookmark( short mark, int push );

	public:
// /STEVE CHANGE
		static Profiling* Open( int id/*Rtt_Allocator* allocator, const char* name*/ ); // <- STEVE CHANGE
		static void AddEntry( void* profiling, const char* name );
		static void Close( void* profiling );
		static void ResetSums();
		static int VisitSums( lua_State* L );

	public:
		static bool Find( const Profiling* profiling );
		static void Init( struct lua_State* L, Rtt_Allocator* allocator ); // <- STEVE CHANGE
		static int DestroyAll( struct lua_State* L );
		static int Create( Rtt_Allocator* allocator, const char* name ); // <- STEVE CHANGE
		//static Profiling* GetOrCreate( Rtt_Allocator* allocator, const char* name ); // <- STEVE CHANGE
		static Profiling* Get( const char* name );

	public:
		// STEVE CHANGE
		#ifdef Rtt_AUTHORING_SIMULATOR
			static int GetSimulatorRun() { return sSimulatorRun; }
		#endif
		// /STEVE CHANGE

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
			EntryRAII( Rtt_Allocator* allocator, int& id, const char* name ); // <- STEVE CHANGE
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
		static PtrArray<Profiling>* sLists;//* fFirstList; <- STEVE CHANGE
		static Profiling* /*f*/sTopList; // <- STEVE CHANGE
		static Sum* /*f*/sFirstSum; // STEVE CHANGE
// STEVE CHANGE
	#ifdef Rtt_AUTHORING_SIMULATOR
		static int sSimulatorRun;
	#endif
// /STEVE CHANGE
		Array<Entry>* fArray1;
		Array<Entry>* fArray2;
		Entry::ShortName fName;
	//	Profiling* fNext; <- STEVE CHANGE
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

// STEVE CHANGE
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
// /STEVE CHANGE

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Profiling_H__
