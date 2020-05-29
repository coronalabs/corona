//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
