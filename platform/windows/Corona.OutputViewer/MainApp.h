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

#ifndef __AFXWIN_H__
#	error "include 'stdafx.h' before including this file for PCH"
#endif

#include "Interop\Event.h"
#include "Interop\EventArgs.h"
#include <string>
class VisualTheme;


/// <summary>
///  <para>The main application class where the program starts.</para>
///  <para>Only 1 instance of this class is expected to be created for the lifetime of the application.</para>
/// </summary>
class MainApp : public CWinAppEx
{
	public:
		/// <summary>
		///  Defines the "VisualThemeChanged" event type which is raised when the UI's visual theme has been changed
		///  via the SetVisualTheme() method.
		/// </summary>
		typedef Interop::Event<MainApp&, const Interop::EventArgs&> VisualThemeChangedEvent;

		/// <summary>Name of the "Settings" registry key used to store application settings.</summary>
		static const TCHAR kSettingsRegistrySectionName[];

		/// <summary>Name of the registry entry used to store the string ID of the last selected visual theme.</summary>
		static const TCHAR kVisualThemeRegistryEntryName[];

		/// <summary>Creates a new application object. Only 1 is allowed to exist at a time.</summary>
		MainApp();

		/// <summary>Called by MFC upon application start. The MFC equivalent of a main() function.</summary>
		/// <returns>Returns true if the application was initialized successfully. Returns false if failed.</returns>
		virtual BOOL InitInstance();

		/// <summary>Called by MFC upon program termination.</summary>
		/// <returns>
		///  <para>Returns an exit code value.</para>
		///  <para>A value of zero indicates the program was ran successfully.</para>
		///  <para>A non-zero value indicates that an error has occurred.</para>
		/// </returns>
		virtual int ExitInstance();

		/// <summary>Gets an object used to add or remove an event handler for the "VisualThemeChanged" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		VisualThemeChangedEvent::HandlerManager& GetVisualThemeChangedEventHandlers();

		/// <summary>Determines if this application is in exclusive stdin mode.</summary>
		/// <returns>
		///  <para>Returns true if this app is set up to receive its parent app's output via this app's stdin pipe.</para>
		///  <para>
		///   Returns false if this app is set up to launch and monitor other app's output via their stdout/stderr pipes.
		///  </para>
		/// </returns>
		bool IsStdInExclusiveModeEnabled() const;

		/// <summary>
		///  <para>Determines if this application's main window should have its close button disabled.</para>
		///  <para>This is the case when this app is launched by the Corona Simualtor in stdin exclusive mode.</para>
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns true if the window's close button should be disabled while monitoring the output of another app.
		///  </para>
		///  <para>Returns false if the end-user should be allowed to close this application at any time.</para>
		/// </returns>
		bool IsDisableCloseWhileReadingEnabled() const;

		/// <summary>Gets the currently selected theme applied to this application such as kSystem, kDark, etc.</summary>
		/// <returns>Returns the currently selected visual theme such as kSystem, kDark, etc.</returns>
		const VisualTheme& GetVisualTheme() const;

		/// <summary>
		///  <para>Sets the visual theme to be applied to this application's UI.</para>
		///  <para>Will raise a "VisualThemeChanged" event if theme has changed.</para>
		/// </summary>
		/// <param name="theme">The visual theme to be applied such as kSystem, kDark, etc.</param>
		void SetVisualTheme(const VisualTheme& theme);

		/// <summary>
		///  <para>Converts the given relative file or directory path to an absolute path.</para>
		///  <para>Replaces the given string's contents (which is passed by reference) with the new absolute path.</para>
		///  <para>The given string will be unchanged if it's already an absolute path or if it's an invalid path.</para>
		/// </summary>
		/// <param name="path">Relative file/directory path string to be converted to an absolute path.</param>
		static void ConvertToAbsolutPath(std::wstring& path);

	private:
		/// <summary>Manages the "VisualThemeChanged" event.</summary>
		VisualThemeChangedEvent fVisualThemeChangedEvent;

		/// <summary>
		///  <para>Set true if we're receiving a parent app's output via this app's stdin.</para>
		///  <para>Set false to set up this app to launch other apps and monitor their stdout/stderr.</para>
		/// </summary>
		bool fIsStdInExclusiveModeEnabled;

		/// <summary>Set true if a command line argument was received to disable this app window's close button.</summary>
		bool fIsDisableCloseWhileReadingEnabled;

		/// <summary>Pointer to the currently selected theme such as kSystem, kDark, or kBlue.</summary>
		const VisualTheme* fVisualThemePointer;
};

/// <summary>The main application instance as defined by the MFC project template.</summary>
extern MainApp theApp;
