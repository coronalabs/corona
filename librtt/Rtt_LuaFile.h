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
