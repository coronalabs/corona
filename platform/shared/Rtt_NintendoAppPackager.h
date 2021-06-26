//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_NintendoAppPackager_H__
#define _Rtt_NintendoAppPackager_H__

// ----------------------------------------------------------------------------

#include "Rtt_PlatformAppPackager.h"

namespace Rtt
{

class LuaContext;
class MPlatformServices;

// ----------------------------------------------------------------------------

class NintendoAppPackagerParams : public AppPackagerParams
{
	public:
		typedef AppPackagerParams Super;

	public:
		NintendoAppPackagerParams( 
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
			const char * nintendotemplate,
			bool useStandartResources
			)
		: AppPackagerParams( 
			appName, version, identity, provisionFile, srcDir, dstDir, sdkRoot,
			targetPlatform, (char*)"nintendo", targetVersion, targetDevice, customBuildId, productId,
			appPackage, isDistributionBuild )
		, fNintendoTemplate(nintendotemplate)
		, fUseStandartResources(useStandartResources)
		{
		}

	public:
		const String fNintendoTemplate;
		const bool fUseStandartResources;
};

class NintendoAppPackager : public PlatformAppPackager
{
	public:
		typedef PlatformAppPackager Super;

	public:
		NintendoAppPackager( const MPlatformServices& services );
		virtual ~NintendoAppPackager();

	public:
		virtual int Build( AppPackagerParams *params, const char *tmpDirBase);

	private:
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_NintendoAppPackager_H__
