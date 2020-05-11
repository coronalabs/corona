//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IOSAppPackager_H__
#define _Rtt_IOSAppPackager_H__

// ----------------------------------------------------------------------------

#include "Rtt_PlatformAppPackager.h"
namespace Rtt
{

class LuaContext;
class MPlatformServices;
class MacSimulatorServices;

// ----------------------------------------------------------------------------

// TODO: This isn't really a Mac packager so much as it is an iPhone packager.
// The Mac packager ought to be the projector. Refactor.
	
class IOSAppPackagerParams : public AppPackagerParams
{
	public:
		typedef AppPackagerParams Super;

	public:
		IOSAppPackagerParams( 
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

class IOSAppPackager : public PlatformAppPackager
{
	public:
		typedef PlatformAppPackager Super;

	public:
		IOSAppPackager( const MPlatformServices& services, MacSimulatorServices *simulatorServices = NULL );
		virtual ~IOSAppPackager();

	public:
		// TODO: caller should make dstDir a unique directory
		virtual int Build( AppPackagerParams *params, const char *tmpDirBase );
        int SendToAppStore( IOSAppPackagerParams *osxParams, const char *itunesConnectUsername, const char *itunesConnectPassword );

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

#endif // _Rtt_IOSAppPackager_H__
