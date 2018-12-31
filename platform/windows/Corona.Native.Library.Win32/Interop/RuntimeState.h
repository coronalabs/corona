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


namespace Interop {

/// <summary>Indicates the current state of a Corona runtime object such as Starting, Running, etc.</summary>
enum class RuntimeState
{
	/// <summary>Indicates that the Corona runtime has not started a project yet.</summary>
	kNotStarted,

	/// <summary>
	///  <para>Indicates that the Corona runtime's Run() method has been called, but hasn't been started yet.</para>
	///  <para>
	//    During this phase, the Corona runtime is initializing itself and attempting to access its rendering surface.
	///  </para>
	/// </summary>
	kStarting,

	/// <summary>
	///  Indicates that the Corona runtime has been started/resumed and is actively executing its Lua scripts,
	///  rendering to the screen, playing audio, etc.
	/// </summary>
	kRunning,

	/// <summary>
	///  <para>
	///   Indicates that the Corona runtime's Suspend() method has been called and is in the middle of suspending itself.
	///  </para>
	///  <para>During this phase, system event "applicationSuspend" is raised in Lua.</para>
	/// </summary>
	kSuspending,

	/// <summary>
	///  <para>Indicates that the Corona runtime has been suspended.</para>
	///  <para>
	///   This means features such as rendering, audio, timers, and other Corona related operations
	///   have been currently paused.
	///  </para>
	/// </summary>
	kSuspended,

	/// <summary>
	///  <para>
	///   Indicates that the Corona runtime's Resume() method has been called and is in the middle of resuming itself.
	///  </para>
	///  <para>During this phase, system event "applicationResume" event is raised in Lua.</para>
	/// </summary>
	kResuming,

	/// <summary>
	///  <para>Indicates that the Corona runtime is being terminated.</para>
	///  <para>During this phase, system event "applicationExit" is raised in Lua.</para>
	/// </summary>
	kTerminating,

	/// <summary>Indicates that the Corona runtime has been completely terminated.</summary>
	kTerminated
};

}	// namespace Interop
