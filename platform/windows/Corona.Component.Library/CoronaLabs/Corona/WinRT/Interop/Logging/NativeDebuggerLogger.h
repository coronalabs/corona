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


#include "ILogger.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Logging {

/// <summary>Logs messages to Visual Studio's "Output" panel if the "Native" debugger is attached to the application.</summary>
/// <remarks>
///  You cannot create instances of this class. Instead, you must access a single instance of
///  this class made available via its static <see cref="Instance"/> property.
/// </remarks>
public ref class NativeDebuggerLogger sealed : public ILogger
{
	private:
		/// <summary>Creates a new logger which outputs to Visual Studio "Native" debugger, if attached.</summary>
		/// <remarks>
		///  Constructor made private to force the caller to acquire a single instance of this class
		///  via its static <see cref="Instance"/> property.
		/// </remarks>
		NativeDebuggerLogger();

	public:
		/// <summary>Determines if this logger is able to output messages to Visual Studio's native debugger.</summary>
		/// <value>
		///  <para>Returns true if this logger's Write() method is able to output messages.</para>
		///  <para>
		///   Returns false if this logger is unable to log anything.
		///   This occurs when Visual Studio's "Native" debugger is not attached to the application.
		///  </para>
		/// </value>
		virtual property bool CanWrite { bool get(); }

		/// <summary>
		///  <para>Outputs the given message to the logging system.</para>
		///  <para>Does nothing if this logger's <see cref="CanWrite"/> property is false.</para>
		/// </summary>
		/// <param name="message">The message to be logged. Ignored if given a null or empty string.</param>
		[Windows::Foundation::Metadata::DefaultOverload]
		virtual void Write(Platform::String^ message);

		/// <summary>
		///  <para>Outputs the given message to the logging system.</para>
		///  <para>Does nothing if this logger's <see cref="CanWrite"/> property is false.</para>
		/// </summary>
		/// <param name="message">The message to be logged. Ignored if given a null or empty array.</param>
		virtual void Write(const Platform::Array<wchar_t>^ message);
		
		/// <summary>Provides a pre-allocated instance of this class.</summary>
		static property NativeDebuggerLogger^ Instance { NativeDebuggerLogger^ get(); }
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Logging
