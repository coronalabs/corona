//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AndroidAppPackager_H__
#define _Rtt_AndroidAppPackager_H__

// ----------------------------------------------------------------------------

#include "Rtt_PlatformAppPackager.h"
#include "Core/Rtt_String.h"
#include "Rtt_DeviceOrientation.h"
#include "Rtt_TargetDevice.h"
#include <string>

namespace Rtt
{

class LuaContext;
class MPlatformServices;

// ----------------------------------------------------------------------------

class AndroidAppPackagerParams : public AppPackagerParams
{
	public:
		typedef AppPackagerParams Super;

	private:
		String fKeyStore;
		String fKeyAlias;
		String fKeyStorePassword;
		String fKeyAliasPassword;
		U32 fVersionCode;
		bool fWindowsNonAscii;

	public:
		AndroidAppPackagerParams( const char* appName, 
						  const char* versionName,
						  const char* identity,
						  const char* provisionFile,
						  const char* srcDir,
						  const char* dstDir,
						  const char* sdkRoot,
						  TargetDevice::Platform targetPlatform,
						  const char * targetAppStore,
						  S32 targetVersion,
						  const char * customBuildId,
						  const char * productId,
						  const char * appPackage,
						  bool isDistributionBuild,
						  const char * keyStore,
						  const char * storePassword,
						  const char * keyAlias,
						  const char * aliasPassword,
						  U32 versionCode
		);
		
		const char * GetAndroidKeyStore() const { return fKeyStore.GetString(); }
		const char * GetAndroidKeyAlias() const { return fKeyAlias.GetString(); }
		const char * GetAndroidKeyStorePassword() const { return fKeyStorePassword.GetString(); }
		const char * GetAndroidKeyAliasPassword() const { return fKeyAliasPassword.GetString(); }
		U32 GetVersionCode() const { return fVersionCode; }

		void SetWindowsNonAsciiUser(bool value) { fWindowsNonAscii = value; }
		bool IsWindowsNonAsciiUser() const { return fWindowsNonAscii; }
	public:
		virtual void Print();
};

class AndroidAppPackager : public PlatformAppPackager
{
	public:
		AndroidAppPackager( const MPlatformServices& services, const char * resourcesDir );
		virtual ~AndroidAppPackager();

	public:
		// TODO: caller should make dstDir a unique directory
		virtual int Build( AppPackagerParams *params, const char *tmpDirBase );

		virtual bool VerifyConfiguration() const;
		bool IsUsingExpansionFile() const { return fIsUsingExpansionFile; }

//		const StringArray & GetAndroidPermissions() const { return fPermissions; }
//		DeviceOrientation::Type GetDefaultOrientation() const { return fDefaultOrientation; }
//		bool SupportsOrientationChange() const { return fSupportsOrientationChange; }

	protected:
		bool CreateBuildProperties( const AppPackagerParams& params, const char *tmpDir );
		virtual char* Prepackage( AppPackagerParams * params, const char* tmpDir );
		virtual void OnReadingBuildSettings( lua_State *L, int index );
		std::string EscapeArgument(std::string arg);

	private:
		String fResourcesDir;
		bool fIsUsingExpansionFile;
		
		 // TODO: This belongs in params
//		StringArray fPermissions;
//		String fVersionCode;
//		DeviceOrientation::Type fDefaultOrientation;
//		bool fSupportsOrientationChange;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidAppPackager_H__
