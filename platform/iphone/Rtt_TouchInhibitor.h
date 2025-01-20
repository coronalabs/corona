//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TouchInhibitor_H__
#define _Rtt_TouchInhibitor_H__

// ----------------------------------------------------------------------------

@class CoronaView;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class TouchInhibitor
{
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION

	public:
		TouchInhibitor( CoronaView *view );
		~TouchInhibitor();

	private:
		CoronaView *fView;
};

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TouchInhibitor_H__
