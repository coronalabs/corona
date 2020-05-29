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

#include "CoronaBoxedBoolean.h"
#include "CoronaBoxedNumber.h"
#include "CoronaBoxedString.h"
#include "CoronaBoxedList.h"
#include "CoronaBoxedTable.h"


namespace CoronaLabs { namespace Corona { namespace WinRT {

ref class CoronaRuntimeEnvironment;
ref class CoronaLuaEventArgs;

/// <summary>
///  <para>Delegate to be invoked by the Corona runtime when an event has been dispatched from Lua.</para>
///  <para>
///   An instance of this handler is expected to be given to the CoronaRuntimeEnvironment::AddEventListener() method
///   in order to subscribe to a particular Corona event.
///  </para>
/// </summary>
/// <param name="sender">Reference to the Corona runtime that raised/dispatched the event.</param>
/// <param name="e">Provides the Corona event's properties.</param>
/// <returns>
///  <para>Returns a value back to Lua, if applicable to the event.</para>
///  <para>
///   Can return a <see cref="CoronaBoxedBoolean"/>, <see cref="CoronaBoxedNumber"/>, <see cref="CoronaBoxedString"/>,
///   <see cref="CoronaBoxedList"/>, or <see cref="CoronaBoxedTable"/>.
///  </para>
///  <para>Returning null will return nil in Lua.</para>
/// </returns>
[Windows::Foundation::Metadata::WebHostHidden]
public delegate ICoronaBoxedData^ CoronaLuaEventHandler(CoronaRuntimeEnvironment^ sender, CoronaLuaEventArgs^ e);

} } }	// namespace CoronaLabs::Corona::WinRT
