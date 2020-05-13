//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_WebAppPackager_H__
#define _Rtt_WebAppPackager_H__

// ----------------------------------------------------------------------------

#include "Rtt_PlatformAppPackager.h"

namespace Rtt
{

class LuaContext;
class MPlatformServices;

// ----------------------------------------------------------------------------

class WebAppPackagerParams : public AppPackagerParams
{
	public:
		typedef AppPackagerParams Super;

	public:
		WebAppPackagerParams( 
			const char* appName, 
			const char* version,
			const char* identity,
			const char* provisionFile,
			const char* srcDir,
			const char* dstDir,
			const char* sdkRoot,
			TargetDevice::Platform targetPlatform,
			S32 targetVersion,
			S32 targetDevice,
			const char * customBuildId,
			const char * productId,
			const char * appPackage,
			bool isDistributionBuild,
			bool useStandartResources,
			const char * webtemplate,
			bool createFBInstant
			)
		: AppPackagerParams( 
			appName, version, identity, provisionFile, srcDir, dstDir, sdkRoot,
			targetPlatform, (char*)"web", targetVersion, targetDevice, customBuildId, productId,
			appPackage, isDistributionBuild )
		, useStandartResources(useStandartResources)
		, webtemplate(webtemplate)
		, createFBInstantArchive(createFBInstant)
		{
		}

	public:
		const bool useStandartResources;
		const String webtemplate;
		const bool createFBInstantArchive;
};

class WebAppPackager : public PlatformAppPackager
{
	public:
		typedef PlatformAppPackager Super;

	public:
		WebAppPackager( const MPlatformServices& services );
		virtual ~WebAppPackager();

	public:
		virtual int Build( AppPackagerParams *params, const char *tmpDirBase);

	private:
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WebAppPackager_H__
