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

#ifndef _Rtt_AppPackagerFactory_H__
#define _Rtt_AppPackagerFactory_H__

#include "Rtt_Lua.h"
#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class PlatformAppPackager;
class AppPackagerParams;
class MPlatformServices;

// ----------------------------------------------------------------------------

class AppPackagerContext
{
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION

	public:
		AppPackagerContext( TargetDevice::Platform platform );
		~AppPackagerContext();

	private:
		bool fRequiresJava;
};

// ----------------------------------------------------------------------------

#define CHECK_VALUE( str, key )			\
										\
do {									\
	if ( ! ( str ) )					\
	{									\
		fprintf( stderr, "ERROR: Missing '%s' key in build arguments\n", (key) ); \
		return NULL;					\
	}									\
} while ( 0 )

class AppPackagerFactory
{
	public:
		typedef AppPackagerFactory Self;

	public:
		AppPackagerFactory( const MPlatformServices& services );
		virtual ~AppPackagerFactory();

	public:
		AppPackagerParams* CreatePackagerParams( lua_State *L, int index, TargetDevice::Platform targetPlatform ) const;

		PlatformAppPackager* CreatePackager( lua_State *L, int index, TargetDevice::Platform targetPlatform ) const;

		const char *GetResourceDirectory() const;
#if defined(Rtt_MAC_ENV)
		const char *GetResourceDirectoryOSX() const;
#elif defined(Rtt_WIN_ENV)
		const char *GetResourceDirectoryWin() const;
#endif

	protected:
#if defined(CORONABUILDER_IOS) || defined(CORONABUILDER_TVOS) || defined(CORONABUILD_OSX)
	virtual AppPackagerParams* CreatePackagerParamsApple(
			lua_State *L,
			int index,
			TargetDevice::Platform targetPlatform,
			TargetDevice::Version targetPlatformVersion,
			const char *appName,
			const char *version,
			const char *certificatePath,
			const char *projectPath,
			const char *dstPath,
			const char *sdkPath,
			const char *customBuildId,
			const char *templateType ) const;
#endif

#if defined(CORONABUILDER_ANDROID)
		virtual AppPackagerParams* CreatePackagerParamsAndroid(
			lua_State *L,
			int index,
			TargetDevice::Platform targetPlatform,
			TargetDevice::Version targetPlatformVersion,
			const char *appName,
			const char *version,
			const char *certificatePath,
			const char *projectPath,
			const char *dstPath,
			const char *sdkPath,
			const char *customBuildId,
			const char *templateType ) const;
#endif

#if defined(CORONABUILDER_WIN32)
		virtual AppPackagerParams* CreatePackagerParamsWin32(
			lua_State *L,
			int index,
			TargetDevice::Platform targetPlatform,
			TargetDevice::Version targetPlatformVersion,
			const char *appName,
			const char *version,
			const char *certificatePath,
			const char *projectPath,
			const char *dstPath,
			const char *sdkPath,
			const char *customBuildId,
			const char *templateType ) const;
#endif

#if defined(CORONABUILDER_HTML5)
	virtual AppPackagerParams* CreatePackagerParamsHTML5(
			   lua_State *L,
			   int index,
			   TargetDevice::Platform targetPlatform,
			   TargetDevice::Version targetPlatformVersion,
			   const char *appName,
			   const char *version,
			   const char *certificatePath,
			   const char *projectPath,
			   const char *dstPath,
			   const char *sdkPath,
			   const char *customBuildId,
			   const char *templateType ) const;
#endif

#if defined(CORONABUILDER_LINUX)
	virtual AppPackagerParams* CreatePackagerParamsLinux(
			   lua_State *L,
			   int index,
			   TargetDevice::Platform targetPlatform,
			   TargetDevice::Version targetPlatformVersion,
			   const char *appName,
			   const char *version,
			   const char *certificatePath,
			   const char *projectPath,
			   const char *dstPath,
			   const char *sdkPath,
			   const char *customBuildId,
			   const char *templateType ) const;
#endif

	private:
		const MPlatformServices& fServices;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_AppPackagerFactory_H__
