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

#ifndef _Rtt_Analytics_H__
#define _Rtt_Analytics_H__

#include "Rtt_PlatformAppPackager.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class LuaContext;
class MPlatform;
class AuthorizationTicket;

// ----------------------------------------------------------------------------

class Analytics
{
	public:
		Analytics( const MPlatform& platform, const char* baseUrl, const char* deviceId );
		~Analytics();

	public:
		enum
		{
			kUndefinedValue = 0
		};
	
		// TODO: Standardize these constants
		enum
		{
			// Never use 0 as a value
			kRun = 1,
			kRelaunch = 2,
			kClose = 3,
			kDeviceBuild = 4
		};

		// Relaunch subtypes
		enum
		{
			// Never use 0 as a value
			kIPhoneSkin = 1,
			kIPadSkin = 2,
			kDroidSkin = 3,
			kNexusOneSkin = 4,
			kHTCMyTouchSkin = 5,
			kIPhone4Skin = 6,
			kGalaxyTabSkin = 7
		};

		// Device build subtypes
		enum
		{
			// Never use 0 as a value
			kIPhonePlatform = TargetDevice::kIPhonePlatform,
			kAndroidPlatform = TargetDevice::kAndroidPlatform
		};

	public:
		void BeginSession( const char *sessionId );
		void Log( int event, int eventdata = kUndefinedValue, const char *eventstring = NULL );
		void EndSession();
		bool IsSessionStarted() const { return fSessionStarted; }
		void SetEnabled( bool newValue ) { fEnabled = newValue; }

		static const char* GetProductVersionNameFrom( const AuthorizationTicket *ticket );
		static int WriteBuildMessage(char *messageBuffer, int targetDeviceVersionId, int targetDeviceTypeId);

	private:
		const MPlatform& fPlatform;
		LuaContext* fVM;
		bool fSessionStarted;
		bool fEnabled;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Analytics_H__
