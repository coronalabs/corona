//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_TargetDevice.h"
#include "BuildResult.h"


namespace Interop
{
	class SimulatorRuntimeEnvironment;
}
namespace Rtt
{
	class MouseEvent;
	class WebServicesSession;
	class WinSimulator;
	struct SimulatorOptions;
};


// Authorization functions
CBuildResult appAndroidBuild(Interop::SimulatorRuntimeEnvironment *pSim,
                            const char *srcDir,
                            const char *applicationName, const char *versionName, const char *package,
                            const char *keystore, const char *keystore_pwd,
                            const char *alias, const char *alias_pwd, const char *dstDir,
							const Rtt::TargetDevice::Platform targetPlatform,
							const char *targetAppStoreName,
							bool isDistribution, int versionCode, bool createLiveBuild
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
							, bool includeFusePlugins
							bool enableMonetization
#endif
							);

CBuildResult appWebBuild(Interop::SimulatorRuntimeEnvironment *pSim,
						 const char *srcDir,
						 const char *applicationName, const char *versionName,
						 const char *dstDir,
						 const Rtt::TargetDevice::Platform targetPlatform,
						 const char * targetos, bool isDistribution, int versionCode,
						 bool useStandartResources, bool createFBInstantArchive);

CBuildResult appLinuxBuild(Interop::SimulatorRuntimeEnvironment *pSim,
						 const char *srcDir,
						 const char *applicationName, const char *versionName,
						 const char *dstDir,
						 const Rtt::TargetDevice::Platform targetPlatform,
						 const char * targetos, bool isDistribution, int versionCode, bool useStandartResources);

void appEndNativeAlert(void *pLuaResource, int nButtonIndex, bool bCanceled);
