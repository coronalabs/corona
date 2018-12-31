// ----------------------------------------------------------------------------
// 
// HandledEventArgs.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
	#error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace WinRT {

/// <summary>
///  Event arguments providing a flag indicating whether or not an event handler has handled the raised event.
/// </summary>
public ref class HandledEventArgs sealed
{
	public:
		/// <summary>Creates a new event arguments object with the Handled property defaulted to false.</summary>
		HandledEventArgs();

		/// <summary>Gets or sets whether or not the event was handled.</summary>
		/// <value>
		///  <para>
		///   Set to true if an event handler has handled this event. This would typically override the default
		///   behavior of the object that raised this event.
		///  </para>
		///  <para>Set to false if an event handler has not handled this event.</para>
		/// </value>
		property bool Handled { bool get(); void set(bool value); }

	private:
		bool fWasHandled;
};

} }	// namespace CoronaLabs::WinRT
