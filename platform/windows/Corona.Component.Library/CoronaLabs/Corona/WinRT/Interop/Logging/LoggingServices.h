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

/// <summary>
///  <para>Provides easy access to the default logging features that Corona will use for it's Lua print() functions.</para>
///  <para>Makes it easy to log asynchronously for best performance or blocking for easy debugging.</para>
///  <para>
///   Also has the ability to swap out the logging implementation, such as to Microsoft's Native or Managed debuggers.
///  </para>
/// </summary>
public ref class LoggingServices sealed
{
	private:
		/// <summary>Constructor made private to force callers to use this class' static functions and properties.</summary>
		LoggingServices();

	public:
		/// <summary>Enables/disables asynchronous logging.</summary>
		/// <value>
		///  <para>
		///   Set true to set up the logging system to log messages on another thread for best performance.
		///   This property is set true by default.
		///  </para>
		///  <para>
		///   Set false to have all logging calls log immediately, blocking the calling thread.
		///   Note that this has worst performance if logging via Microsoft's Managed or Native debugging functions
		///   because they block the calling thread for about 4 milliseconds per call.
		///  </para>
		/// </value>
		static property bool AsyncLoggingEnabled { bool get(); void set(bool value); }

		/// <summary>Gets or sets the logger this class uses to log all messages.</summary>
		/// <value>
		///  <para>
		///   Set to an ILogger instance used by this class to log all messages.
		///   This is typically set to an instance of the DotNetDebuggerLogger or NativeDebuggerLogger classes which
		///   log to Visual Studio's "Output" panel if Microsoft's .NET "Managed" or "Native" debuggers are attached.
		///  </para>
		///  <para>
		///   Set to null if a logger has not been assigned to this class, which means calls to this class'
		///   <see cref="Log"/> function or Lua' print() function will output nothing.
		///  </para>
		/// </value>
		static property ILogger^ Logger { ILogger^ get(); void set(ILogger^ logger); }

		/// <summary>
		///  <para>Logs the given message to the logging system.</para>
		///  <para>Will use the logging system assigned to this class via the <see cref="Logger"/> property.</para>
		///  <para>
		///   If property <see cref="AsyncLoggingEnabled"/> is true, then this a non-blocking call where the given
		///   message will logged later on another thread. (Provides best performance.)
		///  </para>
		///  <para>
		///   If property <see cref="AsyncLoggingEnabled"/> is false, then this is a blocking call
		///   and will log the given message immediately.
		///  </para>
		/// </summary>
		/// <param name="message">The message to be logged. Ignored if null or empty.</param>
		static void Log(Platform::String^ message);
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Logging
