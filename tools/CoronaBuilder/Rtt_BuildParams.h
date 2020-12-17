//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_BuildParams_H__
#define _Rtt_BuildParams_H__

#include "Rtt_Lua.h"

#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class AppPackagerParams;
class AppPackagerFactory;
class PlatformAppPackager;

// ----------------------------------------------------------------------------

class BuildParams
{
	public:
		typedef BuildParams Self;

	public:
		typedef enum _Format
		{
			kJsonFormat = 0,
			kLuaFormat,
			
			kNumFormats
		}
		Format;

	public:
		BuildParams( lua_State *L, const char *path, Format format, int argc, const char *argv[] );
		~BuildParams();

	public:
		bool IsValid() const;

	public:
		TargetDevice::Platform GetTargetPlatform() const;
		AppPackagerParams* CreatePackagerParams( const AppPackagerFactory& factory, TargetDevice::Platform targetPlatform ) const;
		PlatformAppPackager* CreatePackager( const AppPackagerFactory& factory, TargetDevice::Platform targetPlatform ) const;

	private:
		lua_State *fL;
		int fRef;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_CoronaBuilder_H__
