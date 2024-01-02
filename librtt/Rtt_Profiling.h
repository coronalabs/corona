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
		Profiling( Rtt_Allocator* allocator, const char* name );
		~Profiling();

	public:
		class Payload {
			public:
				enum Type {
					kString,

					// Listener functions
					kTable,
					kFunction,

					kNumTypes
				};

			public:
				Payload( const char* str = NULL )
				{
					SetString( str );
				}

				Payload( const void* ptr, bool isTable )
				{
					SetPointer( ptr, isTable );
				}

			public:
				void SetString( const char* str );
				void SetPointer( const void* ptr, bool isTable );

			public:
				const char* GetString() const;
				const void* GetPointer() const;

				bool HasTablePointer() const { return kTable == fType; }

			private:
				const void* fValue;
				Type fType;
		};

	public:
		void Commit( ProfilingState& state );
		void Push( ProfilingState& state );
		const char* GetName() const { return fName.GetString(); }
		Profiling* GetBelow() const { return fBelow; }
		void AddEntry( const Payload& payload, bool isListName = false );
		int VisitEntries( lua_State* L ) const;

	public:
		static void ResetSums();
		static int VisitSums( lua_State* L );

	private:
		struct Entry {
			static U64 SublistTime() { return ~0ULL; }
		
			Payload fPayload;
			U64 fTime;
		};

	public:	
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
			Payload fName;
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
		Payload fName;
		Profiling* fBelow;
};

class ProfilingState {
	public:
		ProfilingState( Rtt_Allocator* allocator );

	public:
		bool Find( const Profiling* profiling );
		int Create( const char* name );
		Profiling* GetByID( int id );
		Profiling* Get( const char* name );
		
	public:
		Profiling* Open( int id );
		void AddEntry( void* profiling, const Profiling::Payload& payload );
		void Close( void* profiling );

	public:
		static void Bookmark( short mark, int push, void* ud );

	public:
		Profiling* GetTopOfList() const { return fTopList; }
		void SetTopOfList( Profiling* profiling ) { fTopList = profiling; }

	public:
		int GetUpdateID() const { return fUpdateID; }
		int GetRenderID() const { return fRenderID; }
	
	private:
		PtrArray<Profiling> fLists;
		Profiling* fTopList;
		int fUpdateID;
		int fRenderID;
};

class ProfilingEntryRAII {
	public:
		ProfilingEntryRAII( ProfilingState& state, int id );
		~ProfilingEntryRAII();

	public:
		void Add( const Profiling::Payload& payload ) const;

		Profiling* GetProfiling() const { return fProfiling; }

	private:
		ProfilingState& fState;
		Profiling* fProfiling;
};

#define PROFILE_SUMS 0 // include sums in profiling?

// see https://stackoverflow.com/a/8075408 for ensuring string literals, and thus static lifetimes

#if PROFILE_SUMS != 0
	#define SUMMED_TIMING( var, name ) static Profiling::Sum s_##var( name "" ); Profiling::SumRAII var##_w( s_##var )
	#define ENABLE_SUMMED_TIMING( enable ) Profiling::Sum::EnableSums( enable )
#else
	#define SUMMED_TIMING( var, name )
	#define ENABLE_SUMMED_TIMING( enable )
#endif

#define PROFILING_BEGIN( state, var, name ) ProfilingEntryRAII var( state, ( state ).Get##name##ID() )
#define PROFILING_ADD( var, name ) ( var ).Add( Profiling::Payload( name "" ) )

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Profiling_H__
