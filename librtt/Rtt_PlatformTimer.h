//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformTimer_H__
#define _Rtt_PlatformTimer_H__

#include "Rtt_MCallback.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class PlatformTimer
{
	Rtt_CLASS_NO_COPIES( PlatformTimer )

	public:
		PlatformTimer( MCallback& callback );
		virtual ~PlatformTimer();

	public:
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void SetInterval( U32 milliseconds ) = 0;
		virtual bool IsRunning() const = 0;

		// Problem: On iOS, when the screen locks and the display turns off,
		// CADisplayLink timers stop firing.
		// In my experiments, backgrounding allows the CADisplayLink to continue firing, though if the display sleeps/locks, the timer stops firing.
		// To avoid this problem, we need to switch off of CADisplayLink to NSTimer.
		// Since we can't easily distinguish between a screen lock and backgrounding, we treat both events as the same and switch to the background timer.
		// Note that CVDisplayLink may have similar issues on Mac when there is no display, offscreen rendering, and possibly VNC remote desktop.
		// But as a more general principle, we may want all platforms to use a "nice-to-CPU" background timer which invokes less frequently to ease CPU cost. 
		virtual void SwitchToForegroundTimer();
		virtual void SwitchToBackgroundTimer();
	
	public:
		// Allow manual invocation
		Rtt_FORCE_INLINE void operator()() { fCallback(); }

//	protected:
//		Rtt_FORCE_INLINE MCallback& Callback() { return fCallback; }

	private:
		MCallback& fCallback;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformTimer_H__
