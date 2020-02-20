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


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Logging {

/// <summary>Outputs messages to a logging system.</summary>
public interface class ILogger
{
	/// <summary>Determines if this logger is able to output messages to its logging system.</summary>
	/// <value>
	///  <para>Returns true if this logger's Write() method is able to output messages.</para>
	///  <para>
	///   Returns false if this logger is unable to log anything.
	///   This means that calling the Write() method will do nothing.
	///  </para>
	/// </value>
	property bool CanWrite { bool get(); }

	/// <summary>
	///  <para>Outputs the given message to the logging system.</para>
	///  <para>Does nothing if this logger's CanWrite property is false.</para>
	/// </summary>
	/// <param name="message">The message to be logged. Ignored if given a null or empty string.</param>
	[Windows::Foundation::Metadata::DefaultOverload]
	void Write(Platform::String^ message);

	/// <summary>
	///  <para>Outputs the given message to the logging system.</para>
	///  <para>Does nothing if this logger's CanWrite property is false.</para>
	/// </summary>
	/// <param name="message">The message to be logged. Ignored if given a null or empty array.</param>
	void Write(const Platform::Array<wchar_t>^ message);
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Logging
