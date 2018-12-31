// ----------------------------------------------------------------------------
// 
// CoronaLuaEventArgs.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT {

ref class CoronaLuaEventProperties;
ref class ReadOnlyCoronaLuaEventProperties;

/// <summary>Provides a generic collection of all property values belonging to a Corona event dispatched to/from Lua.</summary>
public ref class CoronaLuaEventArgs sealed
{
	public:
		/// <summary>Creates a new object providing a Corona event's properties to be dispatched to/from Lua.</summary>
		/// <param name="properties">
		///  <para>Collection of name/value property pairs that make up the Corona event table in Lua.</para>
		///  <para>
		///   This collection must contain a "name" property having a unique Corona event name or
		///   else an InvalidArgumentException will be thrown.
		///  </para>
		///  <para>The given argument cannot be null or else a NullReferenceException will be thrown.</para>
		/// </param>
		CoronaLuaEventArgs(CoronaLuaEventProperties^ properties);

		/// <summary>
		///  <para>
		///   Gets the event's unique name from its properties collection such as "system", "timer", "enterFrame", etc.
		///  </para>
		///  <para>In Lua, this is the name provided via the "event.name" field in the Lua listener function.</para>
		/// </summary>
		/// <value>The event's unique name such as "system", "timer", "enterFrame", etc.</value>
		property Platform::String^ EventName { Platform::String^ get(); }

		/// <summary>Gets a read-only collection of name/value property pairs making up the Corona event.</summary>
		/// <value>
		///  <para>Read-only collection of the Corona event's properties. For example, "name", "type", etc.</para>
		///  <para>
		///   The event properties are defined here:
		///   <a href="http://docs.coronalabs.com/api/event/index.html">http://docs.coronalabs.com/api/event/index.html</a>
		///  </para>
		/// </value>
		property ReadOnlyCoronaLuaEventProperties^ Properties { ReadOnlyCoronaLuaEventProperties^ get(); }

	private:
		/// <summary>The event's unique name fetched from the properties collection for fast access.</summary>
		Platform::String^ fEventName;

		/// <summary>Read-only collection of the Corona event's properties.</summary>
		ReadOnlyCoronaLuaEventProperties^ fReadOnlyProperties;
};

} } }	// namespace CoronaLabs::Corona::WinRT
