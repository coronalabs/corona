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


namespace CoronaLabs { namespace WinRT {

/// <summary>Provides a message for an event.</summary>
public ref class MessageEventArgs sealed
{
	public:
		/// <summary>Creates a new event arguments object with the given message.</summary>
		/// <param name="message">
		///  <para>The message to be provided.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		MessageEventArgs(Platform::String^ message);

		/// <summary>Gets the event's message.</summary>
		/// <value>The event's message.</value>
		property Platform::String^ Message { Platform::String^ get(); }

	private:
		/// <summary>The event's message.</summary>
		Platform::String^ fMessage;
};

} }	// namespace CoronaLabs::WinRT
