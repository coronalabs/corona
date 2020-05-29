//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacFBConnect_H__
#define _Rtt_MacFBConnect_H__

#include "Rtt_PlatformFBConnect.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacFBConnect : public PlatformFBConnect
{
	public:
		typedef MacFBConnect Self;
		typedef PlatformFBConnect Super;

	public:
		MacFBConnect();

	public:
		virtual void Login( const char *appId, const char *permissions[], int numPermissions ) const;
		virtual void Logout() const;
		virtual void Request( lua_State *L, const char *path, const char *httpMethod, int index ) const;
		virtual void RequestOld( lua_State *L, const char *method, const char *httpMethod, int index ) const;
		virtual void ShowDialog( lua_State *L, int index ) const;
		virtual void PublishInstall( const char *appId ) const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacFBConnect_H__
