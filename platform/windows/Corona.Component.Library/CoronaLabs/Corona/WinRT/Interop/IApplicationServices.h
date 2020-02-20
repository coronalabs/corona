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

/// <summary>
///  <para>Provides access to the currently running application's information and services.</para>
///  <para>Services such as the ability to exit the app or enable/disable automatic screen lock when idle.</para>
/// </summary>
public interface class IApplicationServices
{
	/// <summary>
	///  <para>Enables/disables automatic screen lock when the device has been idle for a period of time.</para>
	///  <para>This property is typically invoked by Corona's system.setIdleTimer() function in Lua.</para>
	/// </summary>
	/// <value>
	///  <para>
	///   Set true to enable automatic screen lock. This means the screen will automatically turn off if the device
	///   has not been touched for a period of time. Note that this property will be ignored if this feature has
	///   not been enabled in the operating system's "Lock Screen" settings screen too.
	///  </para>
	///  <para>
	///   Set false to prevent the screen from turning off when the device hasn't been touched for a period of time.
	///   This is useful for applications that display media content for several minutes or for applications that
	///   use the accelerometer or gyroscope for control input.
	///  </para>
	/// </value>
	property bool LockScreenWhenIdleEnabled { bool get(); void set(bool value); }

//TODO: Add ability to fetch the app's name and version string for Lua's system.getInfo() function.
};

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop
