//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_SimulatorAnalytics_H__
#define _Rtt_SimulatorAnalytics_H__

#include <map>
#include <string>

extern "C"
{
#	include "lua.h"
}

// ----------------------------------------------------------------------------

namespace Rtt
{

class LuaContext;
class MPlatform;

// ----------------------------------------------------------------------------
	
class SimulatorAnalytics
{
	public:
		SimulatorAnalytics( const MPlatform& platform, lua_CFunction luaPackageLoaderCallback = NULL );
		~SimulatorAnalytics();

	public:
		bool Initialize( const char *subscriptionType, int uid );
		bool Log( const char *eventName );
		bool Log( const char *eventName, const char *eventDataValue );
		bool Log( const char *eventName, const char *eventDataKey, const char *eventDataValue );
		bool Log( const char *eventName, const size_t numItems, char **eventDataKeys, char **eventDataValues );
		bool Log( const char *eventName, std::map<std::string, std::string> keyValues );


	public:
		void BeginSession(int uid);
		void EndSession();
		bool IsSessionStarted() const { return fIsSessionStarted; }

	public:
		void SetParticipating( bool newValue ) { fIsParticipating = newValue; }
		bool IsParticipating() const { return fIsParticipating; }

	public:
		const MPlatform& GetPlatform() const { return fPlatform; }

	private:
		const MPlatform& fPlatform;
		LuaContext *fVMContext;
		int fRef;
		bool fIsParticipating;
		bool fIsSessionStarted;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_SimulatorAnalytics_H__
