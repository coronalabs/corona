//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleTimer_H__
#define _Rtt_AppleTimer_H__

#include "Rtt_PlatformTimer.h"

// ----------------------------------------------------------------------------

#if defined(__cplusplus) && ! defined(__OBJC__)
class NSTimer;
class AppleCallback;
class NSObject;
#else
@class NSTimer;
@class AppleCallback;
@class NSObject;
#endif


namespace Rtt
{

// ----------------------------------------------------------------------------

class AppleTimer : public PlatformTimer
{
	Rtt_CLASS_NO_COPIES( AppleTimer )

	public:
		typedef PlatformTimer Super;

	public:
		AppleTimer( MCallback& callback );
		virtual ~AppleTimer();

	public:
		virtual void Start();
		virtual void Stop();
		virtual void SetInterval( U32 milliseconds );
		virtual bool IsRunning() const;

		// Problem: On iOS, when the screen locks and the display turns off,
		// CADisplayLink timers stop firing.
		// In my experiments, backgrounding allows the CADisplayLink to continue firing, though if the display sleeps/locks, the timer stops firing.
		// To avoid this problem, we need to switch off of CADisplayLink to NSTimer.
		// Since we can't easily distinguish between a screen lock and backgrounding, we treat both events as the same and switch to the background timer.
		// Note that CVDisplayLink may have similar issues on Mac when there is no display, offscreen rendering, and possibly VNC remote desktop.
		virtual void SwitchToForegroundTimer();
		virtual void SwitchToBackgroundTimer();

	public:
		AppleCallback* GetTarget() { return fTarget; }
	
	private:
		id fDisplayLink;
		NSTimer* fTimer;
		AppleCallback* fTarget;
		U32 fInterval;
		bool fDisplayLinkSupported;
		U32 fSavedInterval; 
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleTimer_H__
