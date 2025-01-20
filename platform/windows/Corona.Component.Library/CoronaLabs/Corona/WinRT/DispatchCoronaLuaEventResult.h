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


#include "CoronaLabs\WinRT\IOperationResult.h"
#include "ICoronaBoxedData.h"


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>
///  <para>Indicates if the Corona runtime has succeeded in dispatching a Corona event to/from Lua.</para>
///  <para>Provides the result returned by Corona event handler/listener if dispatched successfully.</para>
/// </summary>
public ref class DispatchCoronaLuaEventResult sealed : CoronaLabs::WinRT::IOperationResult
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
		/// <param name="returnedValue">
		///  <para>The value returned by the Corona event's handler.</para>
		///  <para>Expected to be set to a Lua compatible value such as a bool, int, double, string, list, or table.</para>
		///  <para>Can be set to null, indicating that nothing was returned or when a failure occurred.</para>
		/// </param>
		DispatchCoronaLuaEventResult(bool hasSucceeded, Platform::String^ message, ICoronaBoxedData^ returnedValue);

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

		/// <summary>Provides the value returned by the Corona event handler/listener.</summary>
		/// <remarks>This property should be ignored if property <see cref="HasFailed"/> returns true.</remarks>
		/// <value>
		///  <para>The value returned by the Corona event handler/listener.</para>
		///  <para>
		///   This is a boxed value compatible with Lua which is typically of type bool, double, string, list, or table.
		///  </para>
		///  <para>
		///   Will be null if the Corona event handler did not return a value. Will also be null if the Corona runtime
		///   failed to dispatch the event, which is typically the case if the runtime hasn't been started yet
		///   or has been terminated.
		///  </para>
		/// </value>
		property ICoronaBoxedData^ ReturnedValue { ICoronaBoxedData^ get(); }

		/// <summary>Creates a new result set to succeeded and with the given return value.</summary>
		/// <param name="returnedValue">The value returned by the Corona event handler. Can be null.</param>
		/// <returns>Returns a new result object set to succeeded and with the given return value.</returns>
		static DispatchCoronaLuaEventResult^ SucceededWith(ICoronaBoxedData^ returnedValue);

		/// <summary>Creates a new result object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new result object set to failed and with the given message.</returns>
		static DispatchCoronaLuaEventResult^ FailedWith(Platform::String^ message);

	private:
		/// <summary>Determines if the operation has succeeded or failed.</summary>
		bool fHasSucceeded;

		/// <summary>The result's message. Typically an error message indicating why the operation failed.</summary>
		Platform::String^ fMessage;

		/// <summary>The result returned by the Corona event handler, if any. Can be null.</summary>
		ICoronaBoxedData^ fReturnedValue;
};

} } }	// namespace CoronaLabs::Corona::WinRT
