//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformFBConnect_H__
#define _Rtt_PlatformFBConnect_H__

#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class LuaResource;
class MEvent;

// ----------------------------------------------------------------------------

class PlatformFBConnect
{
	public:
		PlatformFBConnect();
		virtual ~PlatformFBConnect();

	public:
		virtual void Login( const char *appId, const char *permissions[], int numPermissions ) const = 0;
		virtual void Logout() const = 0;
		virtual void Request( lua_State *L, const char *path, const char *httpMethod, int index ) const = 0;
		virtual void RequestOld( lua_State *L, const char *method, const char *httpMethod, int index ) const = 0;
		virtual void ShowDialog( lua_State *L, int index ) const = 0;
		virtual void PublishInstall( const char *appId ) const = 0;

	public:
		void SetListener( LuaResource *listener );
		void DispatchEvent( const MEvent& e );

	private:
		LuaResource *fListener;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformFBConnect_H__
