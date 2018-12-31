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

#ifndef _Rtt_LaunchPad_H__
#define _Rtt_LaunchPad_H__

#include "Rtt_Lua.h"
#include "Core/Rtt_ResourceHandle.h"

//#include "Core/Rtt_String.h"
//#include "Core/Rtt_ResourceHandle.h"

//#include "Rtt_LuaArray.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

//class LaunchPad;
//class MCrypto;
//class MPlatform;

// ----------------------------------------------------------------------------
	
class LaunchPad
{
	public:
		static int Loader( lua_State *L );
		static int Open( lua_State *L );

	public:
		LaunchPad( const ResourceHandle< lua_State >& handle );
		~LaunchPad();

	public:
		bool Initialize( int index );
		bool ShouldLog( const char *eventName ) const;
		bool Log( const char *eventName, const char *eventData );

	public:
		void SetParticipating( bool newValue ) { fIsParticipating = newValue; }
		bool IsParticipating() const { return fIsParticipating; }

	private:
		ResourceHandle< lua_State > fHandle;
		int fRef;
		mutable int fRequireCount; // Number of times the "require" log event comes in
		bool fIsParticipating;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LaunchPad_H__
