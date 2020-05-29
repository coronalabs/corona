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


namespace CoronaLabs { namespace Corona { namespace WinRT {

ref class CoronaRuntime;

/// <summary>
///  Indicates the current state of a <see cref="CoronaRuntime">CoronaRuntime</see> object such as Starting, Running, etc.
/// </summary>
public enum class CoronaRuntimeState
{
	/// <summary>Indicates that the Corona runtime has not started a project yet.</summary>
	NotStarted,

	/// <summary>
	///  <para>Indicates that the Corona runtime's Run() method has been called, but hasn't been started yet.</para>
	///  <para>During this phase, the Corona runtime is initializing itself and attempting to access its rendering surface.</para>
	/// </summary>
	Starting,

	/// <summary>
	///  Indicates that the Corona runtime has been started/resumed and is actively executing its Lua scripts,
	///  rendering to the screen, playing audio, etc.
	/// </summary>
	Running,

	/// <summary>
	///  <para>
	///   Indicates that the Corona runtime's Suspend() method has been called and is in the middle of suspending itself.
	///  </para>
	///  <para>During this phase, system event "applicationSuspend" is raised in Lua.</para>
	/// </summary>
	Suspending,

	/// <summary>
	///  <para>Indicates that the Corona runtime has been suspended.</para>
	///  <para>
	///   This means features such as rendering, audio, timers, and other Corona related operations have been currently paused.
	///  </para>
	/// </summary>
	Suspended,

	/// <summary>
	///  <para>
	///   Indicates that the Corona runtime's Resume() method has been called and is in the middle of resuming itself.
	///  </para>
	///  <para>During this phase, system event "applicationResume" event is raised in Lua.</para>
	/// </summary>
	Resuming,

	/// <summary>
	///  <para>Indicates that the Corona runtime is being terminated.</para>
	///  <para>During this phase, system event "applicationExit" is raised in Lua.</para>
	/// </summary>
	Terminating,

	/// <summary>Indicates that the Corona runtime has been completely terminated.</summary>
	Terminated
};

} } }	// namespace CoronaLabs::Corona::WinRT
