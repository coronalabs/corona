//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\OperationResult.h"
#include <cstdint>


namespace Interop { namespace Ipc {

/// <summary>
///  <para>Indicates if a command line process was successfully ran until termination and provides its exit code.</para>
///  <para>
///   If failed to run, then the result object returns an exit code of -1 and an error message explaining why it failed.
///  </para>
/// </summary>
class CommandLineRunResult : public OperationResult
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
		/// <param name="exitCode">The value to be returned by the GetExitCode() method.</param>
		CommandLineRunResult(bool hasSucceeded, const char *message, int32_t exitCode);

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
		/// <param name="exitCode">The value to be returned by the GetExitCode() method.</param>
		CommandLineRunResult(bool hasSucceeded, const wchar_t *message, int32_t exitCode);


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
		/// <param name="exitCode">The value to be returned by the GetExitCode() method.</param>
		/// <param name="output">Captured output to be returned by GetOutput() method.</param>
		CommandLineRunResult(bool hasSucceeded, const wchar_t *message, int32_t exitCode, std::string output);

	public:
		/// <summary>Destroys this object.</summary>
		virtual ~CommandLineRunResult();

		/// <summary>
		///  <para>Gets the exit code returned by the exited process, if successfully ran.</para>
		///  <para>Returns -1 if failed to run the process.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the exit code returned by the exited process, if successfully ran.</para>
		///  <para>Returns -1 if failed to run the process.</para>
		/// </returns>
		int32_t GetExitCode() const;

		/// <summary>
		///  <para>Gets the output emitted by the exited process, if it was recorded.</para>
		///  <para>Returns the empty string if no output was recorded.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the output emitted by the exited process, if any was recorded.</para>
		///  <para>Returns the empty string if no output was recorded.</para>
		/// </returns>
		std::string GetOutput() const;

		/// <summary>Creates a new success result object providing the exit code.</summary>
		/// <param name="exitCode">Value to be returned by the result object's GetExitCode() method.</param>
		/// <returns>Returns a new success result object with the given exit code.</returns>
		static CommandLineRunResult SucceededWith(int32_t exitCode);

		/// <summary>Creates a new success result object providing the exit code and the output.</summary>
		/// <param name="exitCode">Value to be returned by the result object's GetExitCode() method.</param>
		/// <param name="output">Value to be returned by the result object's GetOutput() method.</param>
		/// <returns>Returns a new success result object with the given exit code and output.</returns>
		static CommandLineRunResult SucceededWith(int32_t exitCode, std::string output);

		/// <summary>Creates a new result object set to failed with the given message and an exit code of -1.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new result object set to failed with the given message and an exit code of -1.</returns>
		static CommandLineRunResult FailedWith(const char *message);

		/// <summary>Creates a new result object set to failed with the given message and an exit code of -1.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new result object set to failed with the given message and an exit code of -1.</returns>
		static CommandLineRunResult FailedWith(const wchar_t *message);

		/// <summary>Creates a new result object set to failed with the given message and an exit code of -1.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <param name="output">Any output from the command. Can be empty.</param>
		/// <returns>Returns a new result object set to failed with the given message and an exit code of -1.</returns>
		static CommandLineRunResult FailedWith(const wchar_t *message, std::string output);

	private:
		/// <summary>The exit code returned by the process.</summary>
		int32_t fExitCode;
		std::string fOutput;
};

} }	// namespace Interop::Ipc
