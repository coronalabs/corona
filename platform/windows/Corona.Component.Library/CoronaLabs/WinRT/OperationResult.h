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

#include "IOperationResult.h"


namespace CoronaLabs { namespace WinRT {

/// <summary>Indicates if an operation has succeeded or failed.</summary>
/// <remarks>Instances of this class are immutable.</remarks>
public ref class OperationResult sealed : public IOperationResult
{
	private:
		/// <summary>Creates a new result object with the given information.</summary>
		/// <remarks>
		///  This is a private constructor that is only expected to be called by this class' static
		///  SucceededWith() and FailedWith() functions.
		/// </remarks>
		/// <param name="hasSucceeded">Set to true if the operation succeeded. Set to false if failed.</param>
		/// <param name="message">
		///  Message providing details about the final results of the operation. If the operation failed,
		///  then this message is expected to explain why. Can be set to null or empty string.
		/// </param>
		OperationResult(bool hasSucceeded, Platform::String^ message);

	public:
		/// <summary>Determines if the operation succeeded.</summary>
		/// <value>
		///  <para>Set to true if the operation was executed successfully.</para>
		///  <para>
		///   Set to false if the operation failed, in which case the Message property will likely provide details
		///   as to what went wrong.
		///  </para>
		/// </value>
		virtual property bool HasSucceeded { bool get(); }

		/// <summary>Determines if the operation failed.</summary>
		/// <value>
		///  <para>
		///   Set to true if the operation failed, in which case the Message property will likely provide details
		///   as to what went wrong.
		///  </para>
		///  <para>Set to false if the operation was executed successfully.</para>
		/// </value>
		virtual property bool HasFailed { bool get(); }

		/// <summary>Message providing details about the final result of the operation.</summary>
		/// <returns>
		///  Returns a message providing details about the final result of the operation.
		///  If the operation failed, then this message typically provides details indicating what went wrong.
		///  Operations that are successful typically provide an empty message string.
		/// </returns>
		virtual property Platform::String^ Message { Platform::String^ get(); }

		/// <summary>Gets a reusable OperationResult object set to succeeded and with an empty message.</summary>
		/// <value>A reusable OperationResult object set to succeeded and with an empty message.</value>
		static property OperationResult^ Succeeded { OperationResult^ get(); }

		/// <summary>Creates a new OperationResult object set to succeeded and with the given message.</summary>
		/// <param name="message">The status message to be provided by returned object. Can be null or empty.</param>
		/// <returns>Returns a new OperationResult object set to succeeded and with the given message.</returns>
		static OperationResult^ SucceededWith(Platform::String^ message);

		/// <summary>Creates a new OperationResult object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new OperationResult object set to failed and with the given message.</returns>
		static OperationResult^ FailedWith(Platform::String^ message);

	private:
		bool fHasSucceeded;
		Platform::String^ fMessage;
};

} }	// namespace CoronaLabs::WinRT
