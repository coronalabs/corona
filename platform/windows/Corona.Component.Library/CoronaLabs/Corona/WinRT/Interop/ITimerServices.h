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


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {

interface class ITimer;


/// <summary>Interface used to create timers.</summary>
/// <remarks>
///  This interface is used to implement a platform specific timer for Windows Phone or a Windows Store app.
/// </remarks>
public interface class ITimerServices
{
	/// <summary>Creates a new timer object.</summary>
	/// <returns>
	///  <para>Returns a new timer.</para>
	///  <para>
	///   The returned timer is not running by default. The caller is expected to give it an
	///   interval, set up an Elapsed event handler, and start it manually.
	///  </para>
	/// </returns>
	ITimer^ CreateTimer();
};

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop
