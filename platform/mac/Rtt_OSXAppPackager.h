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

#ifndef _Rtt_OSXAppPackager_H__
#define _Rtt_OSXAppPackager_H__

// ----------------------------------------------------------------------------

#include "Rtt_PlatformAppPackager.h"

namespace Rtt
{

class LuaContext;
class MPlatformServices;
class WebServicesSession;
class Runtime;
class MacSimulatorServices;

// ----------------------------------------------------------------------------

class OSXAppPackagerParams : public AppPackagerParams
{
	public:
		typedef AppPackagerParams Super;

	public:
		OSXAppPackagerParams(
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
							 const char * appSigningIdentity = NULL,
							 const char * installerSigningIdentity = NULL
							 )
		: AppPackagerParams(
							appName, version, identity, provisionFile, srcDir, dstDir, sdkRoot,
							targetPlatform, (char*)"apple", targetVersion, targetDevice, customBuildId, productId,
							appPackage, isDistributionBuild ),
		fAppSigningIdentity(appSigningIdentity),
		fInstallerSigningIdentity(installerSigningIdentity)
		{
		}
		
		// Create a minimal OS X packager params for internal use
		OSXAppPackagerParams(
							 const char* appName,
							 const char* version,
							 const char* srcDir,
							 const char* dstDir
							 )
		: AppPackagerParams(
							appName, version, "", "", srcDir, dstDir, "",
							TargetDevice::kOSXPlatform, (char*)"apple", -1, -1, "", "",
							"", false )
		{
		}
		
	public:
		virtual void Print();
	
		/// <summary>
		///  <para>Gets a pointer to the Corona runtime associated with the project that is being built.</para>
		///  <para>The runtime is needed to acquire and unzip its plugins for local builds.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to the Corona runtime associated with the project being built.</para>
		///  <para>Returns null if not assigned yet.</para>
		/// </returns>
		Runtime* GetRuntime() const { return fRuntime; }
		const char* GetAppSigningIdentity( ) { return fAppSigningIdentity; }
		const char* GetInstallerSigningIdentity( ) { return fInstallerSigningIdentity; }

		/// <summary>
		///  <para>Sets a pointer to the Corona runtime associated with the project that is being built.</para>
		///  <para>The runtime is needed to acquire and unzip its plugins for local builds.</para>
		/// </summary>
		/// <param name="value">Pointer to the Corona runtime. Can be null.</param>
		void SetRuntime(Runtime* value) { fRuntime = value; }
		void SetAppSigningIdentity(const char * appSigningIdentity ) { fAppSigningIdentity = appSigningIdentity; }
		void SetInstallerSigningIdentity(const char * installerSigningIdentity ) { fInstallerSigningIdentity = installerSigningIdentity; }

	public:
		Rtt::Runtime* fRuntime;
		const char* fAppSigningIdentity;
		const char* fInstallerSigningIdentity;
};

class OSXAppPackager : public PlatformAppPackager
{
	public:
		typedef PlatformAppPackager Super;

	public:
		OSXAppPackager( const MPlatformServices& services, MacSimulatorServices *simulatorServices = NULL );
		virtual ~OSXAppPackager();

	public:
		virtual int Build( AppPackagerParams *params, WebServicesSession& session, const char *tmpDirBase );
		virtual int PackageForAppStore( OSXAppPackagerParams *osxParams, bool sendToAppStore, const char *itunesConnectUsername, const char *itunesConnectPassword );
		virtual int PackageForSelfDistribution( OSXAppPackagerParams *osxParams, bool createDMG );

		virtual bool VerifyConfiguration() const;

	public:
		const char * GetBundleId( const char *provisionFile, const char *appName ) const; 

	protected:
		virtual char* Prepackage( AppPackagerParams * params, const char* tmpDir );
		virtual int PrepackagePlugins(OSXAppPackagerParams * params, String& tmpPluginsDir, String& outputDir);

	private:
		MacSimulatorServices *fSimulatorServices;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_OSXAppPackager_H__
