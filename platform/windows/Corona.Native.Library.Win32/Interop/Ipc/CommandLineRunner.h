//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CommandLineRunResult.h"
#include "AsyncPipeReader.h"
#include "IODataEventArgs.h"

namespace Interop { namespace Ipc {

/// <summary>
///  <para>Executes a command line string or shell command until it exits.</para>
///  <para>Can be easily configured to hide the launched process' window or log its stdout/stderr output.</para>
/// </summary>
class CommandLineRunner
{
	public:
		/// <summary>Creates a new command line string runner.</summary>
		CommandLineRunner();

		/// <summary>Destroys this object.</summary>
		virtual ~CommandLineRunner();

		/// <summary>
		///  Determines if the command line executed by the RunUntilExecuted() methods should launch the process
		///  invisibly in the background.
		/// </summary>
		/// <returns>
		///  <para>Returns true if the process will be executed invisibly in the background.</para>
		///  <para>Returns false if the launched process will show its window as normal.</para>
		/// </returns>
		bool IsHideWindowEnabled() const;

		/// <summary>
		///  Sets whether or not the command line executed by the RunUntilExit() methods should launch the process
		///  invisibly in the background.
		/// </summary>
		/// <param name="value">
		///  <para>Set true to run the executed process invisibly in the background.</para>
		///  <para>Set false to show the launched process' window as normal, if it has a window.</para>
		/// </param>
		void SetHideWindowEnabled(bool value);

		/// <summary>
		///  Determines if the command line executed by the RunUntilExit() methods should redirect the process'
		///  stdout/stderr to this app's Rtt_LogException() functions.
		/// </summary>
		/// <returns>
		///  <para>Returns true if the executed process will have its stdout/stderr redirected and its output logged.</para>
		///  <para>Returns false if the executed process will not have its stdout/stderr redirected.</para>
		/// </returns>
		bool IsLogOutputEnabled() const;

		/// <summary>
		///  Sets whether or not the command line executed by the RunUntilExit() methods should redirect the process'
		///  stdout/stderr to this app's Rtt_LogException() functions.
		/// </summary>
		/// <param name="value">
		///  <para>Set true to redirect the executed process' stdout/stderr and log its output.</para>
		///  <para>Set false to not redirect the executed process' stdout/stderr.</para>
		/// </param>
		void SetLogOutputEnabled(bool value);

		/// <summary>
		///  Determines if the command line executed by the RunUntilExit() methods should redirect the process'
		///  stdout/stderr to a string.  The string can be retrieved by calling CommandLineRunResult::GetOutput()
		/// </summary>
		/// <returns>
		///  <para>Returns true if the executed process will have its stdout/stderr redirected and its output captured.</para>
		///  <para>Returns false if the executed process will not have its stdout/stderr captured.</para>
		/// </returns>
		bool IsOutputCaptureEnabled() const;

		/// <summary>
		///  Sets whether or not the command line executed by the RunUntilExit() methods should redirect the process'
		///  stdout/stderr to a string.  The string can be retrieved by calling CommandLineRunResult::GetOutput()
		/// </summary>
		/// <param name="value">
		///  <para>Set true to capture the executed process' stdout/stderr.</para>
		///  <para>Set false to not capture the executed process' stdout/stderr.</para>
		/// </param>
		void SetOutputCaptureEnabled(bool value);

		/// <summary>
		///  Determines if the command line string to be passed into the RunUntilExit() methods is a
		///  Windows shell command such as "dir", "copy", "rmdir", etc.
		/// </summary>
		/// <returns>
		///  <para>Returns true if the command line string is expected to be a Windows shell command string.</para>
		///  <para>Returns false if the command line string is expected to reference an EXE file.</para>
		/// </returns>
		bool IsShellCommand() const;

		/// <summary>
		///  Sets whether or not the command line string to be passed into the RunUntilExit() methods is a
		///  Windows shell command such as "dir", "copy", "rmdir", etc.
		/// </summary>
		/// <param name="value">
		///  <para>Set true if the command line string will be a Windows shell command string.</para>
		///  <para>Set false if the command line string will be referencing an EXE file path.</para>
		/// </param>
		void SetIsShellCommand(bool value);

		/// <summary>
		///  Executes the given command line string, blocking the calling thread until the launched process exits.
		/// </summary>
		/// <param name="commandLine">The UTF-8 encoded command line string to execute.</param>
		/// <returns>
		///  <para>Returns the executed process' exit code if successful.</para>
		///  <para>
		///   Returns a failure result if unable to execute the given command line string.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///   The result object's returned exit code will always be set to -1 for launch failures.
		///  </para>
		/// </returns>
		CommandLineRunResult RunUntilExit(const char* commandLine);

		/// <summary>
		///  Executes the given command line string, blocking the calling thread until the launched process exits.
		/// </summary>
		/// <param name="commandLine">The UTF-16 encoded command line string to execute.</param>
		/// <returns>
		///  <para>Returns the executed process' exit code if successful.</para>
		///  <para>
		///   Returns a failure result if unable to execute the given command line string.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///   The result object's returned exit code will always be set to -1 for launch failures.
		///  </para>
		/// </returns>
		CommandLineRunResult RunUntilExit(const wchar_t* commandLine);

		/// <summary>Gets the result of the last call to the RunUntilExit() method.</summary>
		/// <returns>
		///  <para>Returns the result of the last call to the RunUntilExit() method.</para>
		///  <para>Returns a failure result if the RunUntilExit() method has never been called before.</para>
		/// </returns>
		CommandLineRunResult GetLastRunResult();
		static void receivedDataHandler(AsyncPipeReader& sender, const IODataEventArgs& arguments);

	private:
		bool fIsHideWindowEnabled;
		bool fIsLogOutputEnabled;
		bool fIsOutputCaptureEnabled;
		bool fIsShellCommand;
		CommandLineRunResult fLastRunResult;
		static std::string sCapturedOutput;
		static bool sStaticOutputCaptureEnabled;
};

} }	// namespace Interop::Ipc
