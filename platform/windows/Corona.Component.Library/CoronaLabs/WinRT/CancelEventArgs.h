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
	#error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace WinRT {

/// <summary>Provides a flag indicating if the event's associated operation should be canceled.</summary>
/// <remarks>This class is the equivalent to the .NET "System.CancelEventArgs" class.</remarks>
public ref class CancelEventArgs sealed
{
	public:
		/// <summary>Creates a new event arguments object with the Cancel property defaulted to false.</summary>
		CancelEventArgs();

		/// <summary>Gets or sets whether or not the event's associated operation should be canceled.</summary>
		/// <value>
		///  <para>Set to true to cancel the event's associated operation.</para>
		///  <para>Set to false to allow the operation to be executed.</para>
		/// </value>
		property bool Cancel { bool get(); void set(bool value); }

	private:
		bool fWasCanceled;
};

} }	// namespace CoronaLabs::WinRT
