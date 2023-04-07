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
		int Visit( lua_State* L ) const;

		static Profiling* Open( Rtt_Allocator* allocator, const char* name );
		static void AddEntry( void* profiling, const char* name );
		static void Close( void* profiling );
	
	public:
		static bool Find( const Profiling* profiling );
		static int DestroyAll( struct lua_State* L );
		static Profiling* GetOrCreate( Rtt_Allocator* allocator, const char* name );
		static Profiling* Get( const char* name );

	public:
		struct RAII {
			RAII( Rtt_Allocator* allocator, const char* name );
			~RAII();

			Profiling* fProfiling;
		};
	
	private:
		struct Entry {
			typedef char ShortName[64];
		
			static void SetShortName( ShortName out, const char* name );
			static U64 SublistTime() { return ~0ULL; }
		
			void SetName( const char* name );
		
			ShortName fName;
			U64 fTime;
		};
	
	private:
		static InstrumentList* fFirst;
		static InstrumentList* fTop;

		Array<Entry>* fArray1;
		Array<Entry>* fArray2;
		Entry::ShortName fName;
		Profiling* fNext{NULL};
		Profiling* fBelow{NULL};
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Profiling_H__
