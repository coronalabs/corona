//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Solar2D game engine.
// With contributions from Dianchu Technology
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Archive_H__
#define _Rtt_Archive_H__

#if !defined( Rtt_NO_ARCHIVE )
	#include "Rtt_Lua.h"
	#if defined( Rtt_EMSCRIPTEN_ENV )
		#define Rtt_ARCHIVE_COPY_DATA 1
	#endif
#endif

#ifdef Rtt_ARCHIVE_COPY_DATA
	#include "Core/Rtt_Data.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class Archive
{
	public:
		enum
		{
			kLuaObjectResource = 0x1
		};

		typedef enum Tag
		{
			kUnknownTag = 0x0,
			kContentsTag = 0x1,
			kDataTag = 0x2,
			
			kEOFTag = 0xFFFFFFFF
		}
		Tag;

	private:
		// TODO: Create a hash table (keyed by resource type and then name)
		struct ArchiveEntry
		{
			U32 type;
			U32 offset;
			const char* name;
		};

	public:
		static void Serialize( const char *dstPath, int numSrcPaths, const char *srcPaths[] );
		static size_t Deserialize( const char *dstDir, const char *srcCarFile );
		static void List(const char *srcCarFile);

#if !defined( Rtt_NO_ARCHIVE )
	public:
		Archive( Rtt_Allocator& allocator, const char *srcPath );
		~Archive();

	public:
		static int ResourceLoader( lua_State *L );

	public:
		int LoadResource( lua_State *L, const char* name );
		int DoResource( lua_State *L, const char *name, int narg );

	private:
		Rtt_Allocator& fAllocator;
//		int fDescriptor;
		ArchiveEntry* fEntries;
		size_t fNumEntries;
		const void* fData;
		size_t fDataLen;
#if defined( Rtt_ARCHIVE_COPY_DATA )
		Data<char> fBits;
#endif
#endif
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Archive_H__
