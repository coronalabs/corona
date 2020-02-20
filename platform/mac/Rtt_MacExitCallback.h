//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacExitCallback_H__
#define _Rtt_MacExitCallback_H__

#include "Rtt_PlatformExitCallback.h"
// ----------------------------------------------------------------------------

namespace Rtt
{

class MacExitCallback : public PlatformExitCallback
{
	public:
		virtual void operator()();
		virtual void operator()(int code);
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif
