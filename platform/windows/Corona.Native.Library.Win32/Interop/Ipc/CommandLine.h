//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CommandLineRunResult.h"


namespace Interop { namespace Ipc {

/// <summary>
///  <para>Provides an easy means of executing an EXE file or shell command via a command line string.</para>
///  <para>This class hides the launched process' window by default unless configured otherwise.</para>
///  <para>You cannot create instances of this class. Instead, you are expected to call its static functions.</para>
/// </summary>
class CommandLine final
{
	private:
		/// <summary>Constructor made private to prevent instances from being made.</summary>
		CommandLine();

	public:
		/// <summary>
		///  Determines if the process executed by the Run*() functions will have their window's hidden by default.
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns true if all processes executed by the Run*() functions will have their windows hidden.
		///   (ie: They will run invisibly in the background.)
		///  </para>
		///  <para>Returns false if the process will display their window as normal.</para>
		/// </returns>
		static bool IsHideWindowEnabled();

		/// <summary>
		///  <para>Sets whether or not the process executed by the Run*() functions will have its windows hidden or not.</para>
		///  <para>That is, this controls if the executed process will be running invisibly in the background.</para>
		/// </summary>
		/// <param name="value">
		///  <para>Set true to make all subsequent calls to the Run*() function hide the launched process' window.</para>
		///  <para>Set false to show the ran process' window as normal.</para>
		/// </param>
		static void SetHideWindowEnabled(bool value);

		/// <summary>
		///  Determines if processes executed by the Run*() function will have their stdout/stderr automatically
		///  redirected and logged to this app's Rtt_LogException() function.
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns true if all processes executed by the Run*() functions will have their stdout/stderr
		///   redirected to this app's Rtt_LogException() function.
		///  </para>
		///  <para>Returns false if all processes executed will not have their stdout/stderr pipes redirected.</para>
		/// </returns>
		static bool IsLogOutputEnabled();

		/// <summary>
		///  Sets whether or not process executed by the Run*() functions will have their stdout/stderr redirected
		///  to this app's Rtt_LogException() function.
		/// </summary>
		/// <param name="value">
		///  <para>
		///   Set true to make all subsequent calls to the Run*() functions redirect stdout/stderr to this app
		///   and have it log their output.
		///  </para>
		///  <para>Set false to not redirect the stdout/stderr of all executed command lines.</para>
		/// </param>
		static void SetLogOutputEnabled(bool value);

		/// <summary>
		///  Determines if processes executed by the Run*() function will have their stdout/stderr captured.
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns true if all processes executed by the Run*() functions will have their stdout/stderr
		///   captured.
		///  </para>
		///  <para>Returns false if all processes executed will not have their stdout/stderr pipes redirected.</para>
		/// </returns>
		static bool IsOutputCaptureEnabled();

		/// <summary>
		///  Sets whether or not process executed by the Run*() functions will have their stdout/stderr captured.
		/// </summary>
		/// <param name="value">
		///  <para>
		///   Set true to make all subsequent calls to the Run*() functions redirect stdout/stderr to this app
		///   and have it capture their output.
		///  </para>
		///  <para>Set false to not redirect the stdout/stderr of all executed command lines.</para>
		/// </param>
		static void SetOutputCaptureEnabled(bool value);

		/// <summary>
		///  <para>Executes the given command line string until the launched process exits.</para>
		///  <para>Applies this class' IsHideWindowEnabled() and IsLogOutputEnabled() settings upon launch.</para>
		/// </summary>
		/// <param name="commandLine">The UTF-8 encoded command line string to be executed.</param>
		/// <returns>
		///  <para>Returns the executed process' exit code if successful.</para>
		///  <para>
		///   Returns a failure result if unable to execute the given command line string.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///   The result object's returned exit code will always be set to -1 for launch failures.
		///  </para>
		/// </returns>
		static CommandLineRunResult RunUntilExit(const char* commandLine);

		/// <summary>
		///  <para>Executes the given command line string until the launched process exits.</para>
		///  <para>Applies this class' IsHideWindowEnabled() and IsLogOutputEnabled() settings upon launch.</para>
		/// </summary>
		/// <param name="commandLine">The UTF-16 encoded command line string to be executed.</param>
		/// <returns>
		///  <para>Returns the executed process' exit code if successful.</para>
		///  <para>
		///   Returns a failure result if unable to execute the given command line string.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///   The result object's returned exit code will always be set to -1 for launch failures.
		///  </para>
		/// </returns>
		static CommandLineRunResult RunUntilExit(const wchar_t* commandLine);

		/// <summary>
		///  <para>Executes a Windows shell command such as "dir", "copy", "rmdir", etc. until it exits.</para>
		///  <para>Applies this class' IsHideWindowEnabled() and IsLogOutputEnabled() settings upon launch.</para>
		/// </summary>
		/// <param name="commandLine">The UTF-8 encoded shell command string and its command line arguments.</param>
		/// <returns>
		///  <para>Returns the executed process' exit code if successful.</para>
		///  <para>
		///   Returns a failure result if unable to execute the given command line string.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///   The result object's returned exit code will always be set to -1 for launch failures.
		///  </para>
		/// </returns>
		static CommandLineRunResult RunShellCommandUntilExit(const char* commandLine);

		/// <summary>
		///  <para>Executes a Windows shell command such as "dir", "copy", "rmdir", etc. until it exits.</para>
		///  <para>Applies this class' IsHideWindowEnabled() and IsLogOutputEnabled() settings upon launch.</para>
		/// </summary>
		/// <param name="commandLine">The UTF-16 encoded shell command string and its command line arguments.</param>
		/// <returns>
		///  <para>Returns the executed process' exit code if successful.</para>
		///  <para>
		///   Returns a failure result if unable to execute the given command line string.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///   The result object's returned exit code will always be set to -1 for launch failures.
		///  </para>
		/// </returns>
		static CommandLineRunResult RunShellCommandUntilExit(const wchar_t* commandLine);

	private:
		static bool sIsHideWindowEnabled;
		static bool sIsLogOutputEnabled;
		static bool sIsOutputCaptureEnabled;
};

} }	// namespace Interop::Ipc
