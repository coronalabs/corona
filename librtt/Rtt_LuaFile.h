//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_LuaFile_H__
#define _Rtt_LuaFile_H__

// ----------------------------------------------------------------------------

#define Rtt_LUA_SCRIPT_FILE_EXTENSION "lua"
#define Rtt_LUA_OBJECT_FILE_EXTENSION "lu"

#if defined( Rtt_MAC_ENV ) || defined( Rtt_LUA_C_MODULE_DYLIB )
	#define Rtt_LUA_C_MODULE_FILE_EXTENSION "dylib"
#elif defined( Rtt_WIN_ENV )
	#define Rtt_LUA_C_MODULE_FILE_EXTENSION "dll"
#else
	#define Rtt_LUA_C_MODULE_FILE_EXTENSION "so"
#endif

#define Rtt_LUA_SCRIPT_FILE( basename )   basename "." Rtt_LUA_SCRIPT_FILE_EXTENSION
#define Rtt_LUA_OBJECT_FILE( basename )   basename "." Rtt_LUA_OBJECT_FILE_EXTENSION

// Corona ARchive
#if defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
#define Rtt_CAR_FILE_EXTENSION "corona-archive"
#else
#define Rtt_CAR_FILE_EXTENSION "car"
#endif
#define Rtt_CAR_FILE( basename )	basename "." Rtt_CAR_FILE_EXTENSION

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaFile_H__
