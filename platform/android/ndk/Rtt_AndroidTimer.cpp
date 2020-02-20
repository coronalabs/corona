//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidTimer.h"
#include "NativeToJavaBridge.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidTimer::AndroidTimer( MCallback& callback, NativeToJavaBridge *ntjb ) 
	: PlatformTimer( callback ), fRunning( false ), fNativeToJavaBridge(ntjb)
{
}

AndroidTimer::~AndroidTimer()
{
}

void
AndroidTimer::Start()
{
	fNativeToJavaBridge->SetTimer( fInterval );
	fRunning = true;
}

void
AndroidTimer::Stop()
{
	fNativeToJavaBridge->CancelTimer();
	fRunning = false;
}

void
AndroidTimer::SetInterval( U32 milliseconds )
{
	fInterval = milliseconds;
}

bool
AndroidTimer::IsRunning() const
{
	return fRunning;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

