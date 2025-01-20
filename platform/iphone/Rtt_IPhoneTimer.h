//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneTimer_H__
#define _Rtt_IPhoneTimer_H__

#include "Rtt_PlatformTimer.h"
#include "Rtt_MetalAngleTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneTimer : public PlatformTimer
{
	Rtt_CLASS_NO_COPIES( IPhoneTimer )

	public:
		typedef PlatformTimer Super;

	public:
		IPhoneTimer( MCallback& callback, Rtt_GLKViewController *viewController );
		virtual ~IPhoneTimer();

	public:
		virtual void Start();
		virtual void Stop();
		virtual void SetInterval( U32 milliseconds );
		virtual bool IsRunning() const;

	private:
		U32 fInterval;
		U32 fSavedInterval;
		Rtt_GLKViewController *fViewController;
		void SetViewControllerFPS();
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneTimer_H__
