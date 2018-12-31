// ----------------------------------------------------------------------------
// 
// CoronaRuntime.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
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
#include "CoronaRuntimeEnvironment.h"
#include "CoronaRuntimeEventArgs.h"
#include "CoronaRuntimeState.h"


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	namespace Interop
	{
		ref class CoronaInteropSettings;
	}
	ref class CoronaRuntimeLaunchSettings;
} } }
namespace CoronaLabs { namespace WinRT {
	ref class EmptyEventArgs;
} }

#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>
///  <para>
///   Runtime used to run a Corona project and manage its lifetime via its suspend, resume, and terminate methods and events.
///  </para>
///  <para>
///   You would not normally create a CoronaRuntime instance yourself. Instead, you would use a Corona runtime object
///   provided by a "CoronaLabs.Corona.WinRT.Phone.CoronaPanel" Xaml control.
///  </para>
/// </summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class CoronaRuntime sealed
{
	private:
		/// <summary>Terminates the runtime if still running and destroys this object's resources.</summary>
		~CoronaRuntime();

	public:
		#pragma region Events
		/// <summary>
		///  <para>Raised after loading the "config.lua" file and just but before executing the "main.lua" file.</para>
		///  <para>This is the application's opportunity to register custom APIs into Lua.</para>
		/// </summary>
		event Windows::Foundation::EventHandler<CoronaRuntimeEventArgs^>^ Loaded;

		/// <summary>Raised just after the "main.lua" file has been executed by the Corona runtime.</summary>
		event Windows::Foundation::EventHandler<CoronaRuntimeEventArgs^>^ Started;

		/// <summary>
		///  <para>
		///   Raised when the Corona runtime has been suspended which pauses all rendering, audio, timers
		///   and other Corona related operations.
		///  </para>
		///  <para>
		///   This is typically raised when the end-user navigates to another app or when the power button has been pressed.
		///  </para>
		/// </summary>
		event Windows::Foundation::EventHandler<CoronaRuntimeEventArgs^>^ Suspended;

		/// <summary>Raised when the Corona runtime has been resumed after a suspend.</summary>
		event Windows::Foundation::EventHandler<CoronaRuntimeEventArgs^>^ Resumed;

		/// <summary>
		///  <para>Raised just before the Corona runtime environment is about to be terminated.</para>
		///  <para>
		///   This typically happens when the end-user backs out of the app, the Corona XAML control has been unloaded
		///   from the page, or when the runtime's Terminate() method has been called.
		///  </para>
		/// </summary>
		event Windows::Foundation::EventHandler<CoronaRuntimeEventArgs^>^ Terminating;

		#pragma endregion


		#pragma region Constructors
		/// <summary>Creates a new Corona runtime with the given interop settings.</summary>
		/// <param name="settings">
		///  <para>Provides cross-platform and cross-language feature implementations to the C/C++ side of Corona.</para>
		///  <para>Also provides an optional reference to the Corona control that the runtime will be rendering to.</para>
		///  <para>All other settings in the given object must be set or else an exception will be thrown</para>
		/// </param>
		CoronaRuntime(Interop::CoronaInteropSettings^ settings);

		#pragma endregion


		#pragma region Public Methods/Properties
		/// <summary>Indicates if the Corona runtime is currently running, suspended, terminated, etc.</summary>
		/// <value>The current state of the Corona runtime.</value>
		property CoronaRuntimeState State { CoronaRuntimeState get(); }

		/// <summary>
		///  <para>
		///   Starts executing the "resource.car" or "main.lua" file found under the default resource
		///   directory specified by the <see cref="CoronaRuntimeEnvironment::DefaultResourceDirectoryPath"/> property.
		///  </para>
		///  <para>
		///   If the runtime is currently executing a Corona project, then it will be terminated before running a new one.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns a success result if the Corona runtime was able to start up.</para>
		///  <para>
		///   Returns a failure result if unable to start. In this case, the result object's "Message" property
		///   will provide an error message indicating why the failure occurred.
		///  </para>
		/// </returns>
		CoronaLabs::WinRT::IOperationResult^ Run();
		
		/// <summary>
		///  <para>Starts the Corona runtime by executing the given *.lua or *.car file.</para>
		///  <para>
		///   If the runtime is currently executing a Corona project, then it will be terminated before running a new one.
		///  </para>
		/// </summary>
		/// <param name="filePath">
		///  <para>Absolute path to a *.lua or *.car file to be executed.</para>
		///  <para>This is normally set to a "main.lua" or "resource.car" file in the resource directory.</para>
		///  <para>
		///   This file does not have to reside in the resource directory, but it is recommended since the
		///   Lua require() function defaults to finding Lua files in the resource directory.
		///  </para>
		///  <para>Cannot be null or empty string or else an exception will be thrown.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a success result if the Corona runtime was able to start up.</para>
		///  <para>
		///   Returns a failure result if unable to start. In this case, the result object's "Message" property
		///   will provide an error message indicating why the failure occurred.
		///  </para>
		/// </returns>
		CoronaLabs::WinRT::IOperationResult^ Run(Platform::String^ filePath);

		/// <summary>
		///  <para>
		///   Starts the Corona runtime by using the given path as the resource directory and executing
		///   the "resource.car" or "main.lua" file found it it.
		///  </para>
		///  <para>
		///   If the runtime is currently executing a Corona project, then it will be terminated before running a new one.
		///  </para>
		/// </summary>
		/// <param name="resourceDirectoryPath">
		///  <para>Absolute path to the directory that Corona should use as the resource directory.</para>
		///  <para>Cannot be null or empty string or else an exception will be thrown.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a success result if the Corona runtime was able to start up.</para>
		///  <para>
		///   Returns a failure result if unable to start. In this case, the result object's "Message" property
		///   will provide an error message indicating why the failure occurred.
		///  </para>
		/// </returns>
		CoronaLabs::WinRT::IOperationResult^ RunInDirectory(Platform::String^ resourceDirectoryPath);

		/// <summary>
		///  <para>Starts the Corona runtime using the given launch settings.</para>
		///  <para>
		///   If the runtime is currently executing a Corona project, then it will be terminated before running a new one.
		///  </para>
		/// </summary>
		/// <param name="settings">
		///  <para>
		///   Settings which allow you to customize the Corona runtime environment such as directory paths,
		///   which file to launch on startup.
		///  </para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a success result if the Corona runtime was able to start up.</para>
		///  <para>
		///   Returns a failure result if unable to start. In this case, the result object's "Message" property
		///   will provide an error message indicating why the failure occurred.
		///  </para>
		/// </returns>
		CoronaLabs::WinRT::IOperationResult^ RunUsing(CoronaRuntimeLaunchSettings^ settings);

		/// <summary>
		///  <para>Suspends/pauses the runtime, if currently running.</para>
		///  <para>This will raise an "applicationSuspend" system event in Lua.</para>
		/// </summary>
		void Suspend();

		/// <summary>
		///  <para>Resumes the runtime, if currently suspended.</para>
		///  <para>This will raise an "applicationResume" system event in Lua.</para>
		/// </summary>
		void Resume();

		/// <summary>
		///  <para>Exits and destroys the runtime, if not done already.</para>
		///  <para>This will raise an "applicationExit" system event in Lua.</para>
		/// </summary>
		void Terminate();

		#pragma endregion

	private:
		#pragma region Private SetUpDirectoryResult Class
		/// <summary>
		///  <para>Result type returned by the CoronaRuntime's private SetUpDirectory() method.</para>
		///  <para>Indicates if that method was successful at setting up a Corona sandboxed directory.</para>
		///  <para>
		///   If the SetUpDirectory() method modified the given path, then the result object's "UpdatedPath"
		///   will provide the new path.
		///  </para>
		/// </summary>
		ref class SetUpDirectoryResult sealed : CoronaLabs::WinRT::IOperationResult
		{
			public:
				SetUpDirectoryResult(bool hasSucceeded, Platform::String^ updatedPath, Platform::String^ message);

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

				/// <summary>
				///  <para>Gets the path to the directory that was set up.</para>
				///  <para>Should only be accessed if the operation was successful.</para>
				/// </summary>
				/// <value>
				///  <para>Path to the directory that was set up.</para>
				///  <para>If the operation changed the directory path, then this property will provided the new path.</para>
				///  <para>Set to empty string if the operation failed.</para>
				/// </value>
				property Platform::String^ UpdatedPath { Platform::String^ get(); }

			private:
				bool fHasSucceeded;
				Platform::String^ fUpdatedPath;
				Platform::String^ fMessage;
		};

		#pragma endregion


		#pragma region Private Methods/Properties
		/// <summary>Ensures that the given directory path is valid and creates its directory tree.</summary>
		/// <param name="directoryPath">
		///  <para>Directory path to validate and create the directory tree for.</para>
		///  <para>Can be null or empty string, in which case the given default directory path parameter will be used.</para>
		/// </param>
		/// <param name="defaultDirectoryPath">
		///  <para>The default directory to use if the given "directoryPath" parameter is null or empty.</para>
		///  <para>Cannot be null/empty or else an exception will be thrown.</para>
		/// </param>
		/// <returns>
		///  <para>
		///   Returns a success result if the given directory path was valid and its directory tree was successfully created.
		///   You should use the returned result's "UpdatedPath" property for the directory because it will indicate which
		///   of the given directory paths were used (either "directoryPath" or "defaultDirectoryPath") and it will have
		///   the path's trailing slashes removed if applicable.
		///  </para>
		///  <para>Returns a failure result if given an invalid path or if unable to access the given directory.</para>
		/// </returns>
		SetUpDirectoryResult^ SetUpDirectory(Platform::String^ directoryPath, Platform::String^ defaultDirectoryPath);

		/// <summary>Creates a full directory tree for the given path, if it doesn't already exist.</summary>
		/// <param name="directoryPath">Path to the directory.</param>
		/// <returns>
		///  <para>Returns true if the given path's full directory tree exists.</para>
		///  <para>Returns false if unable to create a directory for the given path or if given an invalid path.</para>
		/// </returns>
		bool CreateDirectoryTreeFor(Platform::String^ directoryPath);

		/// <summary>Determines if the given path references a file that exists.</summary>
		/// <param name="filePath">Path to the file to check for.</param>
		/// <returns>
		///  <para>Returns true if the file exists.</para>
		///  <para>Returns false if the file was not found, the path was invalid, or if given a null/empty string.</para>
		/// </returns>
		bool FileExists(Platform::String^ filePath);

		/// <summary>
		///  <para>Called when a CoronaRuntimeEnvironment created by this object has raised its "Loaded" event.</para>
		///  <para>Relays this event to this runtime object's event handlers.</para>
		/// </summary>
		/// <param name="sender">Reference to the Corona runtime environment that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnLoaded(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>
		///  <para>Called when a CoronaRuntimeEnvironment created by this object has raised its "Started" event.</para>
		///  <para>Relays this event to this runtime object's event handlers.</para>
		/// </summary>
		/// <param name="sender">Reference to the Corona runtime environment that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnStarted(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>
		///  <para>Called when a CoronaRuntimeEnvironment created by this object has raised its "Suspended" event.</para>
		///  <para>Relays this event to this runtime object's event handlers.</para>
		/// </summary>
		/// <param name="sender">Reference to the Corona runtime environment that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnSuspended(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>
		///  <para>Called when a CoronaRuntimeEnvironment created by this object has raised its "Resumed" event.</para>
		///  <para>Relays this event to this runtime object's event handlers.</para>
		/// </summary>
		/// <param name="sender">Reference to the Corona runtime environment that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnResumed(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>
		///  <para>Called when a CoronaRuntimeEnvironment created by this object has raised its "Terminating" event.</para>
		///  <para>Relays this event to this runtime object's event handlers.</para>
		/// </summary>
		/// <param name="sender">Reference to the Corona runtime environment that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnTerminating(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>
		///  <para>Default interop settings provided to this runtime via its constructor.</para>
		///  <para>To be passed into a CoronaRuntimeEnvironment object created by the Run() method.</para>
		/// </summary>
		Interop::CoronaInteropSettings^ fDefaultInteropSettings;

		/// <summary>
		///  <para>Reference to the Corona runtime evironment created by the Run() method using its launch settings.</para>
		///  <para>This contains and handles the native Corona runtime and platform objects which the class controls.</para>
		///  <para>Should be set to null when terminated.</para>
		/// </summary>
		CoronaRuntimeEnvironment^ fEnvironment;

		#pragma endregion
};

} } }	// namespace CoronaLabs::Corona::WinRT
