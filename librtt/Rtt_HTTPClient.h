//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_HTTPClientPlatform_H__
#define Rtt_HTTPClientPlatform_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_String.h"
#include <map>
#include <string>

struct lua_State;

namespace Rtt {
	struct HTTPClient {
		static bool HttpDownload( const char* url, Rtt::String &resultData, String& errorMesg, const std::map<std::string, std::string>& headers );
		static bool HttpDownloadFile( const char* url, const char *filename, String& errorMesg, const std::map<std::string, std::string>& headers );
		static int fetch( lua_State *L );
		static int download( lua_State *L );
		
		static void registerFetcherModuleLoaders( lua_State *L );
	};
}


#endif // _Rtt_Platform_H__
