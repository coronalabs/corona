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

#ifndef _Rtt_DeviceBuildData_H__
#define _Rtt_DeviceBuildData_H__

#include "Core/Rtt_String.h"
#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

class DeviceBuildData
{
	public:
		DeviceBuildData(
			Rtt_Allocator *pAllocator,
			const char *appName,
			S32 targetDevice,
			TargetDevice::Platform targetPlatform,
			S32 targetPlatformVersion,
			const char *targetAppStoreName,
			const char *targetCertType, // bool isDistribution
			const char *clientDeviceId,
			const char *clientProductId,
			const char *clientPlatform );
		virtual ~DeviceBuildData();

	protected:
		bool ReadAppSettings( lua_State *L, const char *appSettingsPath );
		bool ReadBuildSettings( lua_State *L, const char *buildSettingsPath );

	public:
		bool Initialize(
				const char *appSettingsPath,
				const char *buildSettingsPath,
				bool includeFusePlugins,
				bool usesMonetization,
				bool liveBuild,
				int debugBuildProcess);

		void SetBuild( int buildYear, int buildRevision );

	private:
		void PushCoronaPluginMetadata( lua_State *L );
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
		void AddRequiredPlugin( lua_State *L, const char *name );
#endif
		void AddPlugin( lua_State *L, const char *moduleName, int index );

	protected:
		virtual void PushTable( lua_State *L ) const;

	public:
		void GetJSON( String& result ) const;

	private:
		lua_State *fL;
		String fAppName;
		S32 fTargetDevice;
		S32 fTargetPlatform;
		S32 fTargetPlatformVersion;
		String fTargetAppStoreName;
		String fTargetCertType;
		String fClientDeviceId;
		String fClientProductId;
		String fClientPlatform;
		String fBuildQueue;
		String fBuildBucket;
		int fBuildYear;
		int fBuildRevision;
		int fDebugBuildProcess;
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
		String fFuseStagingSuffix;
#endif
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_DeviceBuildData_H__
