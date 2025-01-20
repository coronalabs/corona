//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include "Interop\UI\UIComponent.h"
#include "Interop\Event.h"
#include "Interop\EventArgs.h"
#include "Interop\OperationResult.h"
#include "Interop\ValueResult.h"
#include <memory>
#include <string>
#include <Windows.h>


#pragma region Forward Declarations
namespace Interop {
	namespace Ipc {
		class AsyncPipeReader;
	}
	namespace UI {
		class HandleMessageEventArgs;
		class MessageOnlyWindow;
	}
}

#pragma endregion


namespace Interop { namespace Ipc {

/// <summary>
///  <para>
///   Process class allowing you to easily launch an EXE, monitor its status, and acquire its stdout/stderr if redirected.
///  </para>
///  <para>New process instances can be made by calling this class' static LaunchUsing() function.</para>
/// </summary>
class Process final
{
	Rtt_CLASS_NO_COPIES(Process)

	public:
		#pragma region Public Event Types
		/// <summary>Defines the "Exited" event type which is raised when the process terminates.</summary>
		typedef Event<Process&, const EventArgs&> ExitedEvent;

		#pragma endregion


		#pragma region Public Result Types
		/// <summary>
		///  <para>This type is returned by the static Process::LaunchUsing() function.</para>
		///  <para>Indicates if the process was successfully launched via its HasSucceeded() or HasFailed() methods.</para>
		///  <para>
		///   If successfull, then the GetValue() method will return a "Process" object used to monitor
		///   and interact with that process.
		///  </para>
		///  <para>If failed to launch, the GetMessage() method will return a string detailing why it failed.</para>
		/// </summary>
		typedef ValueResult<std::shared_ptr<Process>> LaunchResult;

		#pragma endregion


		#pragma region LaunchSettings Struct
		/// <summary>
		///  <para>Settings needed to launch a new process via the static Process::LaunchUsing() function.</para>
		///  <para>Provides the path to the EXE file, optional command line arguments, and other settings.</para>
		/// </summary>
		struct LaunchSettings
		{
			/// <summary>The EXE path and file name to be be launched.</summary>
			const wchar_t* FileNamePath;

			/// <summary>Optional command line arguments to be passed to the EXE file upon launch. Can be null.</summary>
			const wchar_t* CommandLineArguments;

			/// <summary>
			///  <para>
			///   Set true to redirect the launched process stdin to receive data from this app. The caller can
			///   then send data to the process via the Process::GetStdInHandle() method's returned pipe handle.
			///  </para>
			///  <para>
			///   Set false to not redirect the launched process' stdin.
			///   The Process::GetStdInHandle() will return null in this case.
			///  </para>
			/// </summary>
			bool IsStdInRedirectionEnabled;

			/// <summary>
			///  <para>
			///   Set true to redirect the launched process' stdout to be received by this app.
			///   You can then read from stdout via the Process::GetStdOutReader() method.
			///  </para>
			///  <para>
			///   Set false to not receive the launched process' stdout data.
			///   The Process::GetStdOutReader() method will return null if not redirected.
			///  </para>
			///  <para>
			///   Warning! You must read from stdout when redirected or else the launched process will hang
			///   when its stdout buffer gets full.
			///  </para>
			/// </summary>
			bool IsStdOutRedirectionEnabled;

			/// <summary>
			///  <para>
			///   Set true to redirect the launched process' stderr to be received by this app.
			///   You can then read from stderr via the Process::GetStdErrReader() method.
			///  </para>
			///  <para>
			///   Set false to not receive the launched process' stderr data.
			///   The Process::GetStdErrReader() method will return null if not redirected.
			///  </para>
			///  <para>
			///   Warning! You must read from stderr when redirected or else the launched process will hang
			///   when its stderr buffer gets full.
			///  </para>
			/// </summary>
			bool IsStdErrRedirectionEnabled;

			/// <summary>Set true to hide the launched process application or console window on startup.</summary>
			bool IsWindowHidden;
		};

		#pragma endregion

	private:
		#pragma region Private Constructors
		/// <summary>
		///  <para>Creates a new process monitoring object initialized to its defaults.</para>
		///  <para>Constructor made private to force callers to use the static LaunchUsing() function.</para>
		/// </summary>
		Process();

		#pragma endregion

	public:
		#pragma region Destructor
		/// <summary>
		///  <para>Destroys this object and releases its resources.</para>
		///  <para>The destructor will not terminate the process it references. It will leave it running.</para>
		/// </summary>
		virtual ~Process();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets an object used to add or remove an event handler for the "Exited" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ExitedEvent::HandlerManager& GetExitedEventHandlers();

		/// <summary>Gets the path to the process' EXE file.</summary>
		/// <returns>Returns the path to the process' EXE file.</returns>
		const wchar_t* GetFilePath() const;

		/// <summary>Gets the command line arguments passed to the EXE file on launch.</summary>
		/// <returns>
		///  <para>Returns all command line arguments passed to the EXE file on launch as a single string.</para>
		///  <para>Returns an empty string if no arguments were passed to the EXE file.</para>
		/// </returns>
		const wchar_t* GetCommandLineArguments() const;

		/// <summary>
		///  <para>Gets a Win32 handle of the process, if currently running.</para>
		///  <para>Will return null if the process is no longer running (ie: has exited).</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the process' Win32 handle.</para>
		///  <para>Returns null if the process is no longer running.</para>
		/// </returns>
		HANDLE GetProcessHandle() const;

		/// <summary>
		///  <para>Gets unique ID assigned to the process by Windows.</para>
		///  <para>Will return zero if the process is no longer running (ie: has exited).</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the unique ID assigned to the process by Windows.</para>
		///  <para>Returns zero if the process is no longer running (ie: has exited).</para>
		/// </returns>
		DWORD GetProcessId() const;

		/// <summary>
		///  <para>Gets a Win32 handle to a pipe used to write to the process' stdin, if available.</para>
		///  <para>
		///   This handle will only be available if the "LaunchSettings::IsStdInRedirectionEnabled" member
		///   was set to true upon launch.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns a Win32 handle to a pipe used to write to the process' stdin.</para>
		///  <para>Returns null if the process' stdin was not redirected upon launch or if the process exited.</para>
		/// </returns>
		HANDLE GetStdInHandle() const;
		
		/// <summary>
		///  <para>Gets a pipe reader used to read the process' stdout, if available.</para>
		///  <para>
		///   This reader will only be available if the "LaunchSettings::IsStdOutRedirectionEnabled" member
		///   was set to true upon launch.
		///  </para>
		///  <para>
		///   If the process' stdout was redirected, then the caller will need to call this reader's Start() method
		///   after launch to read its buffer or else the child process will hang when the stdout buffer gets full.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to the process' stdout reader.</para>
		///  <para>Returns null if the process' stdout was not redirected on launch or if the process exited.</para>
		/// </returns>
		AsyncPipeReader* GetStdOutReader();

		/// <summary>
		///  <para>Gets a pipe reader used to read the process' stderr, if available.</para>
		///  <para>
		///   This reader will only be available if the "LaunchSettings::IsStdErrRedirectionEnabled" member
		///   was set to true upon launch.
		///  </para>
		///  <para>
		///   If the process' stderr was redirected, then the caller will need to call this reader's Start() method
		///   after launch to read its buffer or else the child process will hang when the stderr buffer gets full.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to the process' stderr reader.</para>
		///  <para>Returns null if the process' stderr was not redirected on launch or if the process exited.</para>
		/// </returns>
		AsyncPipeReader* GetStdErrReader();

		/// <summary>Determines if the process has exited/terminated.</summary>
		/// <returns>Returns true if the process is no longer running. Returns false if the process is running.</returns>
		bool HasExited() const;

		/// <summary>
		///  <para>Gets the process' exit code if it is no longer running.</para>
		///  <para>This method should only be called if HasExited() returns true.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a result object providing the exit code via GetValue() if successful.</para>
		///  <para>Returns a failure result if the process hasn't exited yet (still running).</para>
		/// </returns>
		ValueResult<int32_t> GetExitCode() const;

		/// <summary>Attempts to fetch the Win32 handle of the process' main/root window, if it has one.</summary>
		/// <returns>
		///  <para>Returns a Win32 handle to the process' main/root window on the desktop.</para>
		///  <para>Returns null if the process does not have a window or if it has exited.</para>
		/// </returns>
		HWND FetchMainWindowHandle();

		/// <summary>
		///  <para>
		///   Non-blocking call that requests the process' main/root window to be closed via a Win32 WM_CLOSE message.
		///  </para>
		///  <para>
		///   This is the most graceful way to exit a Windows desktop app and is recommended over calling
		///   the RequestForceQuit() method.
		///  </para>
		///  <para>
		///   Note that the process may not exit immediately or it can block/ignore the posted close request
		///   and not exit at all.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns a success result if the close request was successfully posted to the process' main window.
		///   Note that this does not mean that the process will honor the request and may choose to ignore the message.
		///  </para>
		///  <para>Returns a failure result if the process is no longer running or if its main was not found.</para>
		/// </returns>
		OperationResult RequestCloseMainWindow();

		/// <summary>
		///  <para>
		///   Non-blocking call that requests the process' main/root window to be shown via a Win32 WM_SHOWWINDOW message.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns a success result if the show window request was successfully posted to the process' main window.
		///   Note that this does not mean that the process will honor the request and may choose to ignore the message.
		///  </para>
		///  <para>Returns a failure result if the process is no longer running or if its main was not found.</para>
		/// </returns>
		OperationResult RequestShowMainWindow();

		/// <summary>
		///  <para>Non-blocking call which requests the Windows OS to forcefully terminate the process.</para>
		///  <para>This is the equivalent to doing an "End Task" in the Windows Task Manager.</para>
		///  <para>This is the least graceful way to exit an app and prevents it from cleaning up properly on exit.</para>
		///  <para>Calling the RequestCloseMainWindow() is recommended over this method if possible.</para>
		/// </summary>
		/// <param name="exitCode">The exit code to quit the process with.</param>
		/// <returns>
		///  <para>Returns a success result if the termination request was successfully sent.</para>
		///  <para>Returns a failure result if the process is no longer running.</para>
		/// </returns>
		OperationResult RequestForceQuit(int32_t exitCode);

		/// <summary>
		///  <para>Blocks the calling thread until the launched process has terminated.</para>
		///  <para>This method should only be called for short lived processes such as console/command-line apps.</para>
		/// </summary>
		void WaitForExit();

		#pragma endregion


		#pragma region Public Static Functions
		/// <summary>Launches a new application proccess specified by the given launch settings.</summary>
		/// <param name="settings">
		///  <para>Provides the EXE file path, command line arguments, and other launch settings.</para>
		///  <para>Note: The EXE file path is required. All other launch settings are optional.</para>
		/// </param>
		/// <returns>
		///  <para>
		///   Returns a success result if the given EXE file was successfully launched. The result object's
		///   GetValue() method will return a shared pointer to a new "Process" instance which you can use
		///   to monitor the process status/stdout/stderr, fetch info from the process, and request it to exit.
		///  </para>
		///  <para>
		///   Returns a failure result if failed to execute the EXE file provided by the given launch settings.
		///   The result object's GetMessage() method will provide details as to why it failed.
		///  </para>
		/// </returns>
		static Process::LaunchResult LaunchUsing(const Process::LaunchSettings& settings);

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>
		///  <para>Called when a Windows message has been received by this object's message-only window.</para>
		///  <para>
		///   The OnAsyncExited() function will send a message to this method to notify this object on the main
		///   thread when the application process has been exited/terminated.
		///  </para>
		/// </summary>
		/// <param name="sender">Reference to the message-only window.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments);

		/// <summary>Fetches the process' current status such as if it has exited or not.</summary>
		void PollStatus();

		/// <summary>
		///  <para>Closes the process associated Win32 handles and removes events handlers.</para>
		///  <para>Expected to be called when the process exits/terminates or from this class' destructor.</para>
		/// </summary>
		void ReleaseResources();

		#pragma endregion


		#pragma region Private Static Functions
		/// <summary>Called by Windows on another thread when the process has exited/terminated.</summary>
		/// <param name="contextPointer">
		///  Expected to be set to a "Process" pointer that registered this callback into Windows.
		/// </param>
		/// <param name="hasTimedOut">Always set to true. Not relevant to callbacks.</param>
		static void CALLBACK OnAsyncExited(PVOID contextPointer, BOOLEAN hasTimedOut);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Manages the "Exited" event.</summary>
		ExitedEvent fExitedEvent;

		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised.</summary>
		Interop::UI::UIComponent::ReceivedMessageEvent::MethodHandler<Process> fReceivedMessageEventHandler;

		/// <summary>The running process' EXE file name and path.</summary>
		std::wstring fFilePath;

		/// <summary>The command line arguments used to launch the process stored as a single string.</summary>
		std::wstring fCommandLineArguments;

		/// <summary>
		///  <para>Win32 struct storing the launched process' handle and ID assigned to it by Windows.</para>
		///  <para>All fields will be zeroed out when the process exits.</para>
		/// </summary>
		PROCESS_INFORMATION fProcessInfo;

		/// <summary>
		///  <para>Stores a Win32 handle to an anonymous pipe used to write data to the process' stdin.</para>
		///  <para>Set to null if stdin was not redirected upon launch or if the process has exited (pipe was closed).</para>
		/// </summary>
		HANDLE fStdInHandle;

		/// <summary>
		///  <para>Pointer to a reader used to read the process's stdout asynchronously.</para>
		///  <para>Set to null if the process' stdout was not redirected upon launch or if not available.</para>
		/// </summary>
		AsyncPipeReader* fStdOutReaderPointer;

		/// <summary>
		///  <para>Pointer to a reader used to read the process's stderr asynchronously.</para>
		///  <para>Set to null if the process' stderr was not redirected upon launch or if not available.</para>
		/// </summary>
		AsyncPipeReader* fStdErrReaderPointer;

		/// <summary>Set true if the process has exited. Set false if currently running.</summary>
		bool fHasExited;

		/// <summary>
		///  <para>Stores the process' exit code after it has terminated.</para>
		///  <para>Only applicable if member variable "fHasExited" is set true.</para>
		/// </summary>
		int32_t fExitCode;

		/// <summary>
		///  <para>
		///   Stores the handle provided by the Win32 RegisterWaitForSingleObject() function used to notify
		///   this object when the process has exited.
		///  </para>
		///  <para>Set to null if not registered yet or if unregistered after the process exits.</para>
		/// </summary>
		HANDLE fRegisterWaitHandle;

		/// <summary>
		///  Pointer to a Win32 message-only window used to notify this object on its main thread when
		///  the reference application has exited.
		/// </summary>
		Interop::UI::MessageOnlyWindow* fMessageOnlyWindowPointer;

		/// <summary>
		///  <para>Unique Windows message ID reserved by this object's Win32 message-only window.</para>
		///  <para>Used to notify this object when its referenced application process has been exited/terminated.</para>
		/// </summary>
		UINT fCustomExitedMessageId;

		#pragma endregion
};

} }	// namespace Interop::Ipc
