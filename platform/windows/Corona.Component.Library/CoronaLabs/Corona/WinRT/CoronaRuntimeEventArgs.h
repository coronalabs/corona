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


namespace CoronaLabs { namespace Corona { namespace WinRT {

ref class CoronaRuntimeEnvironment;

/// <summary>Provides information about a Corona runtime event.</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class CoronaRuntimeEventArgs sealed
{
	public:
		/// <summary>Creates a new object for storing a Corona runtime event's information.</summary>
		/// <param name="environment">
		///  <para>The Corona runtime environment the event originated from.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		CoronaRuntimeEventArgs(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment);

		/// <summary>Gets the Corona runtime environment that the event originated from.</summary>
		/// <value>The Corona runtime environment the event originated from.</value>
		property CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ CoronaRuntimeEnvironment
		{
			CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ get();
		}

	private:
		/// <summary>Reference to the Corona environment that raised the event.</summary>
		CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ fEnvironment;
};

} } }	// namespace CoronaLabs::Corona::WinRT
