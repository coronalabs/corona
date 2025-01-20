//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformPlayer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char*
PlatformPlayer::DefaultAppFilePath()
{
	return NULL;
}

const char*
PlatformPlayer::InteractiveFilePath()
{
	static const char sChar = 'i';
	return & sChar;
}

// ----------------------------------------------------------------------------

PlatformPlayer::PlatformPlayer( const MPlatform& platform, MCallback *viewCallback )
:	fPlatform( platform ),
	fRuntime( platform, viewCallback ),
	fRuntimeDelegate()
{
	fRuntime.SetDelegate( & fRuntimeDelegate );
}

PlatformPlayer::~PlatformPlayer()
{
}

void
PlatformPlayer::Start(
	const char *appFilePath,
	bool connectToDebugger,
	DeviceOrientation::Type launchOrientation )
{
	U32 launchOptions = Runtime::kSimulatorLaunchOption;

	if ( connectToDebugger )
	{
		launchOptions |= Runtime::kConnectToDebugger;
	}

	if ( Runtime::kSuccess == fRuntime.LoadApplication( launchOptions, launchOrientation ) )
	{
		fRuntime.BeginRunLoop();
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

