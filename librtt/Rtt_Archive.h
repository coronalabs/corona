//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Archive_H__
#define _Rtt_Archive_H__

#if !defined( Rtt_NO_ARCHIVE )
	#include "Rtt_Lua.h"
	#if defined( Rtt_ANDROID_ENV ) || defined( Rtt_EMSCRIPTEN_ENV )
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
