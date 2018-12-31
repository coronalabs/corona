// ----------------------------------------------------------------------------
// 
// BitmapInfoResult.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include "CoronaLabs\WinRT\IOperationResult.h"


// Forward declarations.
namespace CoronaLabs { namespace WinRT {
	ref class OperationResult;
} }


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

// Forward declarations.
ref class BitmapInfo;

/// <summary>Indicates if an operation has succeeded in producing a bitmap.</summary>
public ref class BitmapInfoResult sealed : public CoronaLabs::WinRT::IOperationResult
{
	private:
		#pragma region Constructors
		/// <summary>Creates a new result with the given information.</summary>
		/// <remarks>
		///  This is a private constructor that is only expected to be called by this class' static
		///  SucceededWith() and FailedWith() functions.
		/// </remarks>
		/// <param name="hasSucceeded">
		///  Set to true if the operation succeeded in fetching bitmap information. Set to false if not.
		/// </param>
		/// <param name="message">
		///  Message providing details about the final results of the operation. If the operation failed to fetch bitmap
		///  information, then this message is expected to explain why. Can be set to null or empty string.
		/// </param>
		/// <param name="info">
		///  <para>The bitmap information that was retrieved by the operation, if successful.</para>
		///  <para>Set to null if the operation failed.</para>
		/// </param>
		BitmapInfoResult(bool hasSucceeded, Platform::String^ message, BitmapInfo^ info);

		#pragma endregion

	public:
		#pragma region Public Methods/Properties
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

		/// <summary>Gets the bitmap information that was retrieved by the operation, if successful.</summary>
		/// <value>
		///  <para>Reference to the bitmap information if the operation was successful.</para>
		///  <para>Set to null if the operation failed.</para>
		/// </value>
		property CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo^ BitmapInfo
		{
			CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo^ get();
		}

		#pragma endregion


		#pragma region Public Static Functions
		/// <summary>Creates a new success result object providing the given bitmap information.</summary>
		/// <param name="info">
		///  <para>The bitmap information that was retrieved by the operation.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <returns>Returns a new result object set to succeeded and providing the given information.</returns>
		static BitmapInfoResult^ SucceededWith(CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo^ info);

		/// <summary>Creates a new failure result object with the given error message.</summary>
		/// <remarks>This function is expected to be used if the operation failed to retrieve bitmap information.</remarks>
		/// <param name="message">
		///  <para>Message providing details as to why the operation failed to retrieve bitmap information.</para>
		///  <para>Can be null or empty string.</para>
		/// </param>
		/// <returns>Returns a new result object set to failed and with the given error message.</returns>
		static BitmapInfoResult^ FailedWith(Platform::String^ message);

		#pragma endregion

	private:
		#pragma region Private Member Variables
		CoronaLabs::WinRT::OperationResult^ fBaseResult;
		CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo^ fBitmapInfo;

		#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
