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

/// <summary>Indicates if an operation has succeeded or failed.</summary>
/// <remarks>This interface is expected to be implemented by all OperationResult like classes.</remarks>
public interface class IOperationResult
{
	/// <summary>Determines if the operation succeeded.</summary>
	/// <value>
	///  <para>Set to true if the operation was executed successfully.</para>
	///  <para>
	///   Set to false if the operation failed, in which case the Message property will likely provide details
	///   as to what went wrong.
	///  </para>
	/// </value>
	property bool HasSucceeded { bool get(); }

	/// <summary>Determines if the operation failed.</summary>
	/// <value>
	///  <para>
	///   Set to true if the operation failed, in which case the Message property will likely provide details
	///   as to what went wrong.
	///  </para>
	///  <para>Set to false if the operation was executed successfully.</para>
	/// </value>
	property bool HasFailed { bool get(); }

	/// <summary>Message providing details about the final result of the operation.</summary>
	/// <returns>
	///  Returns a message providing details about the final result of the operation.
	///  If the operation failed, then this message typically provides details indicating what went wrong.
	///  Operations that are successful typically provide an empty message string.
	/// </returns>
	property Platform::String^ Message { Platform::String^ get(); }
};

} }	// namespace CoronaLabs::WinRT
