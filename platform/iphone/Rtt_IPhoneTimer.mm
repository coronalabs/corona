//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_IPhoneTimer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------


IPhoneTimer::IPhoneTimer( MCallback& callback, Rtt_GLKViewController *viewController )
:	Super( callback ),
	fViewController(viewController),
	fInterval( 0x8000000 )
{

}

IPhoneTimer::~IPhoneTimer()
{
	Stop();
	fViewController = nil;
}
	
void
IPhoneTimer::Start()
{
	if ( IsRunning() )
	{
		return;
	}
	
	SetViewControllerFPS();
}

void
IPhoneTimer::Stop()
{

}

void
IPhoneTimer::SetViewControllerFPS()
{
	NSInteger fps = 1000.0 / fInterval;
	fViewController.preferredFramesPerSecond = fps;
}

void
IPhoneTimer::SetInterval( U32 milliseconds )
{
	if ( fInterval != milliseconds )
	{
		bool wasRunning = IsRunning();

		if ( wasRunning ) { Stop(); }

		fInterval = milliseconds;
		SetViewControllerFPS();
		
		if ( wasRunning ) { Start(); }
	}
}

bool
IPhoneTimer::IsRunning() const
{
	return !fViewController.paused;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

