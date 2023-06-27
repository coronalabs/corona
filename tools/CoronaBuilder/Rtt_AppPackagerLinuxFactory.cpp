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
#include "Rtt_LinuxAppPackager.h"
#include <string>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#if defined(CORONABUILDER_LINUX)

	AppPackagerParams*
	AppPackagerFactory::CreatePackagerParamsLinux(
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

		if(targetPlatform != TargetDevice::kLinuxPlatform)
		{
			Rtt_ASSERT_NOT_REACHED();
			return NULL;
		}

		bool includeStandardResources = true;
		lua_getfield(L, index, "includeStandardResources");
		if(lua_type(L, -1) == LUA_TBOOLEAN)
		{
			includeStandardResources = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);

		String linuxtemplate;
		lua_getfield(L, index, "linuxtemplate");
		if(lua_type(L, -1) == LUA_TSTRING)
		{
			linuxtemplate.Set(lua_tostring(L, -1));
		}
		lua_pop(L, 1);

		result = new LinuxAppPackagerParams(appName,
		                                    version,
		                                    NULL,
		                                    NULL,
		                                    projectPath,
		                                    dstPath,
		                                    NULL,
		                                    targetPlatform,
		                                    targetPlatformVersion,
		                                    TargetDevice::kLinuxPlatform,
		                                    customBuildId,
		                                    NULL,
		                                    "bundleId",
		                                    true,
		                                    linuxtemplate.GetString(),
		                                    includeStandardResources, false, false);


		if ( ! result )
		{
			fprintf( stderr, "ERROR: Unsupported platform: %s\n", TargetDevice::StringForPlatform( targetPlatform ) );
		}

		return result;
	}

#endif // defined(CORONABUILDER_LINUX)

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
