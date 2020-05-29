//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidSystemOpenEvent_H__
#define _Rtt_AndroidSystemOpenEvent_H__

#include "Rtt_Event.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidSystemOpenEvent : public SystemOpenEvent
{
	public:
		AndroidSystemOpenEvent(NativeToJavaBridge *ntjb);
		virtual ~AndroidSystemOpenEvent();
		
		virtual int Push(lua_State *L) const;

	private:
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidSystemOpenEvent_H__
