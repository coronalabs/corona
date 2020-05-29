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
