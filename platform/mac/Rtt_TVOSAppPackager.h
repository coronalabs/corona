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

#ifndef _Rtt_TVOSAppPackager_H__
#define _Rtt_TVOSAppPackager_H__

// ----------------------------------------------------------------------------

#include "Rtt_PlatformAppPackager.h"
namespace Rtt
{

class LuaContext;
class MPlatformServices;
class WebServicesSession;
class MacSimulatorServices;

// ----------------------------------------------------------------------------

// TODO: This isn't really a Mac packager so much as it is an iPhone packager.
// The Mac packager ought to be the projector. Refactor.
	
class TVOSAppPackagerParams : public AppPackagerParams
{
	public:
		typedef AppPackagerParams Super;

	public:
		TVOSAppPackagerParams( 
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
			bool isDistributionBuild
			)
		: AppPackagerParams( 
			appName, version, identity, provisionFile, srcDir, dstDir, sdkRoot,
			targetPlatform, (char*)"apple", targetVersion, targetDevice, customBuildId, productId,
			appPackage, isDistributionBuild )
		{
		}

	public:
		virtual void Print();
};

class TVOSAppPackager : public PlatformAppPackager
{
	public:
		typedef PlatformAppPackager Super;

	public:
		TVOSAppPackager( const MPlatformServices& services, MacSimulatorServices *simulatorServices = NULL );
		virtual ~TVOSAppPackager();

	public:
		// TODO: caller should make dstDir a unique directory
		virtual int Build( AppPackagerParams *params, WebServicesSession& session, const char *tmpDirBase );
        int SendToAppStore( TVOSAppPackagerParams *osxParams, const char *itunesConnectUsername, const char *itunesConnectPassword );

		virtual bool VerifyConfiguration() const;

	public:
		const char * GetBundleId( const char *provisionFile, const char *appName ) const; 

	protected:
		virtual char* Prepackage( AppPackagerParams * params, const char* tmpDir );
		bool CopyProvisionFile( const AppPackagerParams * params, const char* tmpDir );

    private:
        MacSimulatorServices *fSimulatorServices;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TVOSAppPackager_H__
