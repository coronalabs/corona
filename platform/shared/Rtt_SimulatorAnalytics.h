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
