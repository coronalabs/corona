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

#include "Core\Rtt_Macros.h"
#include "Rtt_ProjectSettings.h"


namespace Rtt
{

/// <summary>
///  Stores a Corona project's "build.settings" and "config.lua" information.
///  Also reads and stores Win32 desktop app specific information from these files.
/// </summary>
class WinProjectSettings : public ProjectSettings
{
	Rtt_CLASS_NO_COPIES(WinProjectSettings)

	public:
		/// <summary>Indicates where preferences should be stored to.</summary>
		enum class PreferenceStorageType
		{
			/// <summary>Indicates that preferences should be stored to a SQLite database file.</summary>
			kSQLite,

			/// <summary>Indicates that preferences should be stored to the Windows registry.</summary>
			kRegistry
		};


		/// <summary>Creates a new project settings initialized to its defaults.</summary>
		WinProjectSettings();

		/// <summary>Destroys this object and its owned resources.</summary>
		virtual ~WinProjectSettings();

		/// <summary>Resets this object's "build.settings" related information back to their defaults.</summary>
		virtual void ResetBuildSettings();

		/// <summary>Determines where preferences should be stored for a Win32 desktop application.</summary>
		/// <returns>Returns the storage type such as kSQLite or kRegistry.</returns>
		WinProjectSettings::PreferenceStorageType GetWin32PreferenceStorageType() const;

		/// <summary>
		///  Determines if only 1 application window isntance is allowed at a time on the desktpo
		///  or multiple application windows are allowed.
		/// </summary>
		/// <returns>
		///  <para>Returns true if only 1 application window instance can exist at a time on the desktop.</para>
		///  <para>Returns false if multiple application windows are allowed to be made.</para>
		/// </returns>
		bool IsWin32SingleInstanceWindowEnabled() const;

	protected:
		/// <summary>
		///  Called after the LoadFromDirectory() method has successfully loaded information from the
		///  the "build.settings" or "config.lua" files. Will not be called if neither file could be loaded.
		/// </summary>
		/// <param name="luaStatePointer">
		///  Pointer to the Lua state that the "build.settings" and "config.lua" global tables have been pushed to.
		/// </param>
		virtual void OnLoadedFrom(lua_State* luaStatePointer);

	private:
		/// <summary>Indicates if a Win32 app should store preferences to the registry or SQLite.</summary>
		PreferenceStorageType fPreferenceStorageType;

		/// <summary>Set true if only 1 instance of the app window is allowed. Set false for multiple instances.</summary>
		bool fIsSingleInstanceWindowEnabled;
};

}	// namespace Rtt
