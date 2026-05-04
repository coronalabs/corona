//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
		fInstallerSigningIdentity(installerSigningIdentity),
		fDistributionMethod(NULL)
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
							"", false ),
		fAppSigningIdentity(NULL),
		fInstallerSigningIdentity(NULL),
		fDistributionMethod(NULL)
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
		const char* GetAppSigningIdentity() const { return fAppSigningIdentity; }
		const char* GetInstallerSigningIdentity() const { return fInstallerSigningIdentity; }

		// distributionMethod controls which build pipeline CoronaBuilder uses:
		//   "developer"        — standard .app build, unsigned or developer-signed (default)
		//   "app-store"        — Mac App Store .pkg (upload manually via Transporter)
		//   "developer-id"     — Developer ID signed .pkg for direct distribution
		//   "developer-id-dmg" — Developer ID signed .dmg for direct distribution
		const char* GetDistributionMethod() const { return fDistributionMethod ? fDistributionMethod : "developer"; }

		void SetRuntime(Runtime* value) { fRuntime = value; }
		void SetAppSigningIdentity(const char* v) { fAppSigningIdentity = v; }
		void SetInstallerSigningIdentity(const char* v) { fInstallerSigningIdentity = v; }
		void SetDistributionMethod(const char* v) { fDistributionMethod = v; }

	public:
		Rtt::Runtime* fRuntime;
		const char* fAppSigningIdentity;
		const char* fInstallerSigningIdentity;
		const char* fDistributionMethod;
};

class OSXAppPackager : public PlatformAppPackager
{
	public:
		typedef PlatformAppPackager Super;

	public:
		OSXAppPackager( const MPlatformServices& services, MacSimulatorServices *simulatorServices = NULL );
		virtual ~OSXAppPackager();

	public:
		virtual int Build( AppPackagerParams *params, const char *tmpDirBase );
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
		const char *GetAppTemplatePath();
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_OSXAppPackager_H__
