//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaLibSystem_H__
#define _Rtt_LuaLibSystem_H__

#include "Rtt_Lua.h"
#include "Rtt_LuaProxyVTable.h"

#include "Rtt_MPlatform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class LuaLibSystem : public LuaProxyVTable
{
	public:
		typedef LuaLibSystem Self;
		typedef LuaProxyVTable Super;

	public:
        static int getInfo( lua_State *L );
        static int getPreference( lua_State *L );

	public:
		static const char* Directories();
		// static MPlatform::Directory DirForUserdata( void* p );

	public:
		typedef enum FileType
		{
			kUnknownFileType = 0,
			kImageFileType,

			kNumFileTypes
		}
		FileType;
		// For tables that look like { baseDir=system.DocumentsDirectory , filename="foo.png", type="image" }
		// Returns 0 if the table at 'index' is a normal table.
		// Returns 1 if table represents a file path. If a valid string path can
		// be made it will be pushed on top of the stack; otherwise nil is pushed.
		static int PathForTable( lua_State *L, int index, FileType& fileType );
		static int PathForTable( lua_State *L );

		// This returns MPlatform::kResourceDir by default.
		static MPlatform::Directory ToDirectory( lua_State *L, int index );
		// This returns defaultDir by default.
		static MPlatform::Directory ToDirectory( lua_State *L, int index, MPlatform::Directory defaultDir );
		static void PushDirectory( lua_State *L, MPlatform::Directory directory );

		static bool IsWritableDirectory( MPlatform::Directory dir );

	public:
		static int PathForFile( lua_State *L, int index );
		static int PathForFile( lua_State *L );
		static int BeginListener( lua_State *L );
		static int EndListener( lua_State *L );
		static int HasEventSource( lua_State *L );
		static int Activate( lua_State *L );
		static int Deactivate( lua_State *L );


	public:
		// Looks for the following ( filename [, baseDirectory] )
		// Returns index of next argument
		static const char* GetFilename( lua_State *L, int& index, MPlatform::Directory& baseDir );

	public:
		LuaLibSystem();
		static void Initialize( lua_State *L );

	public:
		virtual int ValueForKey( lua_State *L, const MLuaProxyable& object, const char key[], bool overrideRestriction = false ) const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaLibSystem_H__
