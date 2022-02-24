//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LinuxAppPackager_H__
#define _Rtt_LinuxAppPackager_H__

#include "Rtt_PlatformAppPackager.h"

namespace Rtt
{
	class LuaContext;
	class MPlatformServices;

	class LinuxAppPackagerParams : public AppPackagerParams
	{
	public:
		typedef AppPackagerParams Super;

	public:
		LinuxAppPackagerParams(
		    const char *appName,
		    const char *version,
		    const char *identity,
		    const char *provisionFile,
		    const char *srcDir,
		    const char *dstDir,
		    const char *sdkRoot,
		    TargetDevice::Platform targetPlatform,
		    S32 targetVersion,
		    S32 targetDevice,
		    const char *customBuildId,
		    const char *productId,
		    const char *appPackage,
		    bool isDistributionBuild,
		    const char *debtemplate,
		    bool useWidgetResources,
		    bool runAfterBuild,
		    bool onlyGetPlugins
		)
			: AppPackagerParams(
			      appName, version, identity, provisionFile, srcDir, dstDir, sdkRoot,
			      targetPlatform, (char*)"linux", targetVersion, targetDevice, customBuildId, productId,
			      appPackage, isDistributionBuild )
			, fDebTemplate(debtemplate)
			, fUseWidgetResources(useWidgetResources)
			, fRunAfterBuild(runAfterBuild)
			, fOnlyGetPlugins(onlyGetPlugins)
		{
		}

	public:
		const String fDebTemplate;
		const bool fUseWidgetResources;
		const bool fRunAfterBuild;
		const bool fOnlyGetPlugins;
	};

	class LinuxAppPackager : public PlatformAppPackager
	{
	public:
		typedef PlatformAppPackager Super;

	public:
		LinuxAppPackager(const MPlatformServices& services);
		virtual ~LinuxAppPackager();

	public:
		virtual int Build(AppPackagerParams * params, const char * tmpDirBase);
	};
}; // namespace Rtt

#endif // _Rtt_LinuxAppPackager_H__
