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
bool appAuthorizeInstance();
bool appDeauthorize();
int appLoginToServer(Rtt::WebServicesSession *pSession);

// Build functions
bool appAllowFullBuild(const Rtt::TargetDevice::Platform targetPlatform);
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
