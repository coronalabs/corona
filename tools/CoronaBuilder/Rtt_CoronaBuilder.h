//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CoronaBuilder_H__
#define _Rtt_CoronaBuilder_H__

#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class MPlatform;
class MPlatformServices;

class BuildParams;

// ----------------------------------------------------------------------------

class CoronaBuilder
{
	public:
		typedef CoronaBuilder Self;

	public:
		typedef enum _Error
		{
			kNoError = 0,
			kBuildError,
			
			kNumErrors
		}
		Error;

	public:
		typedef enum _Permission
		{
			kUnknownPermission = 0,
			kBuildPermission,
			kAppSignPermission,
			kCarPermission,
			kSplashScreenPermission,

			kNumPermissions
		}
		Permission;

	public:
		CoronaBuilder( const MPlatform& platform, const MPlatformServices& services );
		virtual ~CoronaBuilder();

	public:
		void Usage( const char *arg0 );

	public:
		int Main( int argc, const char *argv[] );
		const MPlatform& GetPlatform() { return fPlatform; }
		const MPlatformServices& GetServices() { return fServices; }

	protected:
		int Build( const BuildParams& params ) const;
		bool CanCustomizeSplashScreen(const char *platformName, const char *bundleID);

	protected:
		

	private:
		const MPlatform& fPlatform;
		const MPlatformServices& fServices;
		String fCommandPath;
		lua_State *fL;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_CoronaBuilder_H__
