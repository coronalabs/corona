//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_PlatformFBConnect.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTFBConnect : public PlatformFBConnect
{
	public:
		typedef WinRTFBConnect Self;
		typedef PlatformFBConnect Super;

		WinRTFBConnect();

		virtual void Login(const char *appId, const char *permissions[], int numPermissions) const;
		virtual void Logout() const;
		virtual void Request(lua_State *L, const char *path, const char *httpMethod, int index) const;
		virtual void RequestOld(lua_State *L, const char *method, const char *httpMethod, int index) const;
		virtual void ShowDialog(lua_State *L, int index) const;
		virtual void PublishInstall(const char *appId) const;
};

} // namespace Rtt
