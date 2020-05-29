//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
