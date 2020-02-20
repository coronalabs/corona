//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidTimer_H__
#define _Rtt_AndroidTimer_H__

#include "librtt/Rtt_PlatformTimer.h"

class NativeToJavaBridge;
// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidTimer : public PlatformTimer
{
	Rtt_CLASS_NO_COPIES( AndroidTimer )

	public:
		AndroidTimer( MCallback& callback, NativeToJavaBridge *ntjb );
		virtual ~AndroidTimer();

	public:
		virtual void Start();
		virtual void Stop();
		virtual void SetInterval( U32 milliseconds );
		virtual bool IsRunning() const;

	private:
		bool		fRunning;
		U32			fInterval;
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidTimer_H__
