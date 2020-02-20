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

#include "CoronaLabs\WinRT\EmptyEventArgs.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {

/// <summary>Interface used to set up and control a single timer.</summary>
/// <remarks>Instances of this interface are returned by the ITimerServices.CreateTimer() method.</remarks>
public interface class ITimer
{

#pragma region Events
	/// <summary>Raised every time the timer's interval has elapsed.</summary>
	event Windows::Foundation::TypedEventHandler<ITimer^, CoronaLabs::WinRT::EmptyEventArgs^>^ Elapsed;

#pragma endregion
	

#pragma region Functions/Properties
	/// <summary>Determines if the timer is currently running.</summary>
	/// <value>
	///  <para>Returns true if this timer has been started and is running.</para>
	///  <para>Returns false if this timer has been stopped.</para>
	/// </value>
	property bool IsRunning { bool get(); }

	/// <summary>Gets or sets the how often this timer's Elapsed event should be raised.</summary>
	/// <value>
	///  <para>The amount of time this timer will raise Elapsed events.</para>
	///  <para>Cannot be set to a value less than 1 millisecond.</para>
	/// </value>
	property Windows::Foundation::TimeSpan Interval;

	/// <summary>Starts the timer using the TimeSpan given to the Interval property.</summary>
	/// <remarks>
	///  <para>Once started, this timer's Elapsed event will be called every time an interval completes.</para>
	///  <para>This method will do nothing if the timer is already running.</para>
	/// </remarks>
	void Start();

	/// <summary>Stops the timer if currently running.</summary>
	void Stop();

#pragma endregion

};

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop
