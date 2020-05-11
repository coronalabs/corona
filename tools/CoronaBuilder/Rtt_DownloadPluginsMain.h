//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_DownloadPluginsMain_H__
#define _Rtt_DownloadPluginsMain_H__

#include "Rtt_Lua.h"

#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class AppPackagerParams;
class AppPackagerFactory;
class PlatformAppPackager;

// ----------------------------------------------------------------------------

class DownloadPluginsMain
{
	public:
		typedef DownloadPluginsMain Self;


	public:
		DownloadPluginsMain(lua_State* L);

		int Run(int argc, const char* args[]);

	private:
		lua_State *fL;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_CoronaBuilder_H__
