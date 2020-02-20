//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>


namespace Interop {

/// <summary>Indicates if an operation has succeeded or failed.</summary>
class OperationResult
{
	protected:
		/// <summary>Creates a new result object with the given information.</summary>
		/// <remarks>
		///  This is a protected constructor that is only expected to be called by this class' static
		///  SucceededWith() and FailedWith() functions or by derived versions of this class.
		/// </remarks>
		/// <param name="hasSucceeded">Set to true if the operation succeeded. Set to false if failed.</param>
		/// <param name="message">
		///  Message providing details about the final results of the operation. If the operation failed,
		///  then this message is expected to explain why. Can be set to null or empty string.
		/// </param>
		OperationResult(bool hasSucceeded, const char *message);

		/// <summary>Creates a new result object with the given information.</summary>
		/// <remarks>
		///  This is a protected constructor that is only expected to be called by this class' static
		///  SucceededWith() and FailedWith() functions or by derived versions of this class.
		/// </remarks>
		/// <param name="hasSucceeded">Set to true if the operation succeeded. Set to false if failed.</param>
		/// <param name="message">
		///  Message providing details about the final results of the operation. If the operation failed,
		///  then this message is expected to explain why. Can be set to null or empty string.
		/// </param>
		OperationResult(bool hasSucceeded, const wchar_t *message);

		/// <summary>Creates a new result object with the given information.</summary>
		/// <remarks>
		///  This is a protected constructor that is only expected to be called by this class' static
		///  SucceededWith() and FailedWith() functions or by derived versions of this class.
		/// </remarks>
		/// <param name="hasSucceeded">Set to true if the operation succeeded. Set to false if failed.</param>
		/// <param name="message">
		///  Message providing details about the final results of the operation. If the operation failed,
		///  then this message is expected to explain why. Can be set to null or empty string.
		/// </param>
		OperationResult(bool hasSucceeded, const std::shared_ptr<const std::wstring> &message);

	public:
		/// <summary>Destroys this object.</summary>
		virtual ~OperationResult();

		/// <summary>Determines if the operation succeeded.</summary>
		/// <returns>
		///  <para>Returns true if the operation was executed successfully.</para>
		///  <para>
		///   Returns false if the operation failed, in which case the GetMessage() method will likely provide details
		///   as to what went wrong.
		///  </para>
		/// </returns>
		bool HasSucceeded() const;

		/// <summary>Determines if the operation failed.</summary>
		/// <returns>
		///  <para>
		///   Returns true if the operation failed, in which case the GetMessage() method will likely provide details
		///   as to what went wrong.
		///  </para>
		///  <para>Returns false if the operation was executed successfully.</para>
		/// </returns>
		bool HasFailed() const;

		/// <summary>Message providing details about the final result of the operation.</summary>
		/// <returns>
		///  Returns a message providing details about the final result of the operation.
		///  If the operation failed, then this message typically provides details indicating what went wrong.
		///  Operations that are successful typically provide an empty message string.
		/// </returns>
		const wchar_t* GetMessage() const;

		/// <summary>
		///  Gets a shared pointer to the message providing details about the final result of the operation.
		/// </summary>
		/// <returns>
		///  Returns a message providing details about the final result of the operation.
		///  If the operation failed, then this message typically provides details indicating what went wrong.
		///  Operations that are successful typically provide an empty message string.
		/// </returns>
		std::shared_ptr<const std::wstring> GetMessageAsSharedPointer() const;

		/// <summary>Determines if this result matches the given result.</summary>
		/// <param name="result">Reference to the result object to compare against.</param>
		/// <returns>Returns true if the objects match. Returns false if not.</returns>
		bool operator==(const OperationResult &result) const;

		/// <summary>Determines if this result does not match the given result.</summary>
		/// <param name="result">Reference to the result object to compare against.</param>
		/// <returns>Returns true if the objects do not match. Returns false if they do match.</returns>
		bool operator!=(const OperationResult &result) const;

		/// <summary>Gets a reusable OperationResult object set to succeeded and with an empty message.</summary>
		static const OperationResult kSucceeded;

		/// <summary>Creates a new OperationResult object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new OperationResult object set to failed and with the given message.</returns>
		static OperationResult FailedWith(const char *message);

		/// <summary>Creates a new OperationResult object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new OperationResult object set to failed and with the given message.</returns>
		static OperationResult FailedWith(const wchar_t *message);

		/// <summary>Creates a new OperationResult object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new OperationResult object set to failed and with the given message.</returns>
		static OperationResult FailedWith(const std::shared_ptr<const std::wstring> &message);

	private:
		/// <summary>Set to true if the operation succeeded. Set to false if failed.</summary>
		bool fHasSucceeded;

		/// <summary>Shared pointer to a UTF-16 encoded message. Will be null if no message was provided.</summary>
		std::shared_ptr<const std::wstring> fMessage;
};

}	// namespace Interop
