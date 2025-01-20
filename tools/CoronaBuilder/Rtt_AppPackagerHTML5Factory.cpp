//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppPackagerFactory.h"

#include "Rtt_WebAppPackager.h"

#include <string>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#if defined(CORONABUILDER_HTML5)

AppPackagerParams*
AppPackagerFactory::CreatePackagerParamsHTML5(
	lua_State *L,
	int index,
	TargetDevice::Platform targetPlatform,
	TargetDevice::Version targetPlatformVersion,
	const char *appName,
	const char *version,
	const char *certificatePath,
	const char *projectPath,
	const char *dstPath,
	const char *sdkPath,
	const char *customBuildId,
	const char *templateType ) const
{
	AppPackagerParams *result = NULL;

	if(targetPlatform != TargetDevice::kWebPlatform)
	{
		Rtt_ASSERT_NOT_REACHED();
		return NULL;
	}

	bool createFBInstantArchive = false;
	lua_getfield(L, index, "createFBInstantArchive");
	if(lua_type(L, -1) == LUA_TBOOLEAN)
	{
		createFBInstantArchive = lua_toboolean(L, -1);
	}
	lua_pop(L, 1);

	bool includeStandardResources = true;
	lua_getfield(L, index, "includeStandardResources");
	if(lua_type(L, -1) == LUA_TBOOLEAN)
	{
		includeStandardResources = lua_toboolean(L, -1);
	}
	lua_pop(L, 1);

	String webtemplate;
	lua_getfield(L, index, "webtemplate");
	if(lua_type(L, -1) == LUA_TSTRING)
	{
		webtemplate.Set(lua_tostring(L, -1));
	}
	lua_pop(L,1);

	result = new WebAppPackagerParams(appName,
									  version,
									  NULL,
									  NULL,
									  projectPath,
									  dstPath,
									  NULL,
									  targetPlatform,
									  targetPlatformVersion,
									  TargetDevice::kWebGenericBrowser,
									  customBuildId,
									  NULL,
									  "bundleId",
									  true,
									  includeStandardResources,
									  webtemplate.GetString(),
										createFBInstantArchive);


	if ( ! result )
	{
		fprintf( stderr, "ERROR: Unsupported platform: %s\n", TargetDevice::StringForPlatform( targetPlatform ) );
	}

	return result;
}

#endif // defined(CORONABUILDER_HTML5)

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

