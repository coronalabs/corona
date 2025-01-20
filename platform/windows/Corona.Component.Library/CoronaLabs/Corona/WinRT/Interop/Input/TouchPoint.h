//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

/// <summary>Stores the coordinate of a single touch event and the time it occurred on.</summary>
public value struct TouchPoint sealed
{
	/// <summary>The touch point's x coordinate.</summary>
	double X;

	/// <summary>The touch point's y coordinate.</summary>
	double Y;
	
	/// <summary>The time when the touch event occurred.</summary>
	Windows::Foundation::DateTime Timestamp;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
