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
class WebServicesSession;

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
		virtual int Build( AppPackagerParams *params, WebServicesSession& session, const char *tmpDirBase );

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
