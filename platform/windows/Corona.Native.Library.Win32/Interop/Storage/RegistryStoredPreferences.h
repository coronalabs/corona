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
#include "MStoredPreferences.h"
#include "StoredPreferencesChain.h"
#include "WinString.h"
#include <memory>
#include <string>
#include <Windows.h>


namespace Interop { namespace Storage {

/// <summary>
///  <para>Reads, writes, and deletes an application's preferences to/from the Windows registry.</para>
///  <para>Preferences are key-value pairs used to easily store simple data/settings.</para>
/// </summary>
class RegistryStoredPreferences : public MStoredPreferences
{
	Rtt_CLASS_NO_COPIES(RegistryStoredPreferences)

	public:
		/// <summary>Provides the "Ansca Corona" registry key name used by the Corona Simulator.</summary>
		static const wchar_t* kAnscaCoronaKeyName;

		/// <summary>Provides the "Corona Simulator" registry key name used by the Corona Simulator.</summary>
		static const wchar_t* kCoronaSimulatorKeyName;

		/// <summary>
		///  Provides the "Corona Labs" registry key name used by the Corona install program
		///  and Corona Console application.
		/// </summary>
		static const wchar_t* kCoronaLabsKeyName;

		/// <summary>
		///  Provides the "Corona" registry key name used by the Corona install program
		///  and Corona Console application.
		/// </summary>
		static const wchar_t* kCoronaSdkKeyName;

		/// <summary>
		///  <para>Provides the MFC profile name to be used by the Corona Simulator.</para>
		///  <para>Is used by MFC to set the registry key name under "HKCU\Software".</para>
		/// </summary>
		static const wchar_t* kSimulatorProfileName;

		/// <summary>Provides the "Preferences" registry key name used by the Corona Simulator.</summary>
		static const wchar_t* kSimulatorPreferencesKeyName;

		/// <summary>
		///  <para>Indicates if the 32-bit or 64-bit sections of the registry on a 64-bit machine.</para>
		///  <para>Only applicable to 64-bit machine registry access. This type is ignored on 32-bit machines.</para>
		///  <para>Note: WoW64 stands for "Windows 32-bit on Windows 64-bit".</para>
		/// </summary>
		enum class Wow64ViewType
		{
			/// <summary>
			///  <para>Indicates that registry access will use Microsoft Windows' default handling.</para>
			///  <para>Will access the 32-bit section of the registry if the application is 32-bit.</para>
			///  <para>Will access the 64-bit section of the registry if the application is 64-bit.</para>
			/// </summary>
			kDefault,

			/// <summary>
			///  Indicates that the 32-bit section of the registry will be accessed on a 64-bit machine.
			/// </summary>
			k32Bit,

			/// <summary>
			///  Indicates that the 64-bit section of the registry will be accessed on a 64-bit machine.
			/// </summary>
			k64Bit
		};

		/// <summary>Provides settings to be passed into the RegistryStoredPreferences class' constructor.</summary>
		struct CreationSettings
		{
			/// <summary>
			///  <para>
			///   Set to a base registry path that all prefere key names will be appended to when reading, writing,
			///   and deleting values to/from the registry. For example, "HKCU\\Software\\Corona Labs".
			///  </para>
			///  <para>
			///   Set to null or empty string to not use a base registry path. This mean that preference key name must
			///   use fully qualified registry paths starting with a registry hive name such as "HKCU", "HKLM", etc.
			///  </para>
			/// </summary>
			const wchar_t* BaseRegistryPath;

			/// <summary>
			///  Indicates if the 32-bit or 64-bit sections of the registry should be accessed a 64-bit machine.
			///  Only applicable to 64-bit machine registry access. This type is ignored on 32-bit machines.
			/// </summary>
			RegistryStoredPreferences::Wow64ViewType Wow64ViewType;

			/// <summary>
			///  <para>
			///   Set true if '/' characters should be interpretted as '\' characters when accessing the registry,
			///   which is like a directory separator in the registry.
			///   This applies to the "BaseRegistryPath" and preference key names.
			///   Intended to make key access portable with other platforms such as Mac, iOS, Android, etc.
			///  </para>
			///  <para>Set false to accept '/' characters in registry key and value names.</para>
			/// </summary>
			bool IsUsingForwardSlashAsPathSeparator;
		};


		/// <summary>Creates a preference accessing manager using the Windows registry to store preferences.</summary>
		/// <param name="settings"></param>
		RegistryStoredPreferences(const RegistryStoredPreferences::CreationSettings& settings);

		/// <summary>Destroys this object.</summary>
		virtual ~RegistryStoredPreferences();

		/// <summary>
		///  <para>Gets the base registry path this object uses, if assigned one upon object construction.</para>
		///  <para>Key names will be appended to this base path when reading, writing, or deleting preferences.</para>
		/// </summary>
		/// <retuns>
		///  <para>Returns the base registry path this object uses.</para>
		///  <para>
		///   Returns an empty string if this object was not assigned a base path upon creation.
		///   In this case, key name must use fully qualified registry paths, starting with the registry hive name
		///   such as "HKCU", "HKLM", etc.
		///  </para>
		/// </returns>
		const wchar_t* GetBaseRegistryPath() const;

		/// <summary>Determines how the registry will be accessed on a 64-bit machine.</summary>
		/// <returns>Returns how the registry will be accessed on a 64-bit machine.</returns>
		RegistryStoredPreferences::Wow64ViewType GetWow64ViewType() const;

		/// <summary>
		///  Determines if '/' characters will be treated like '\' path separators when accessing the registry.
		/// </summary>
		/// <returns>
		///  <para>Returns true if '/' characters will be treated like registry path separators.</para>
		///  <para>Returns false if '/' characters will be accepted as valid registry key and value names.</para>
		/// </returns>
		bool IsUsingForwardSlashAsPathSeparator() const;

		/// <summary>Fetches a preference's value by its unique key name.</summary>
		/// <param name="keyName">The unique name of the preference.</param>
		/// <returns>
		///  <para>
		///   Returns a success result if the preference was found.
		///   The result object's GetValue() method will return the fetched preference's value.
		///  </para>
		///  <para>
		///   Returns a failure result if the preference key was not found or if there was a problem accessing storage.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::Preference::ReadValueResult Fetch(const char* keyName) override;

		/// <summary>Writes the given preference key-value pair to storage.</summary>
		/// <param name="preference">The preference key name and value.</param>
		/// <returns>
		///  <para>Returns a success result if the given preference was written to storage.</para>
		///  <para>
		///   Returns a failure result if failed to write the given preference to storage.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult UpdateWith(const Rtt::Preference& preference) override;

		/// <summary>Writes the given collection of preference key-value pairs to storage.</summary>
		/// <param name="preferences">Collection of preferences to write to storage. Can be empty.</param>
		/// <returns>
		///  <para>Returns a success result if all of the given preferences were written to storage.</para>
		///  <para>
		///   Returns a failure result if at least 1 preference has failed to write to storage.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult UpdateWith(const Rtt::PreferenceCollection& preferences) override;

		/// <summary>Delete a preference from storage by its unique string key.</summary>
		/// <param name="keyName">The unique name of the preference to delete.</param>
		/// <returns>
		///  <para>Returns a success result if the preference was deleted from storage.</para>
		///  <para>
		///   Returns a failure result if failed to access storage or unable to delete the preference.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult Delete(const char* keyName) override;

		/// <summary>Delete multiple preferences from storage by their unique string keys.</summary>
		/// <param name="keyNameArray">Array of unique preference names to delete from storage.</param>
		/// <param name="count">The number of keys/elements in the "keyNameArray" argument.</param>
		/// <returns>
		///  <para>Returns a success result if all of the given preferences have been deleted from storage.</para>
		///  <para>
		///   Returns a failure result if failed to access storage, if unable to delete at least 1 preference,
		///   or if "keyNameArray" is null.
		///   The result object's GetMessage() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult Delete(const char* keyNameArray[], int count) override;

		/// <summary>
		///  Gets a preconfigured RegistryStoredPreferences object that is used to provide read/write access to the
		///  Corona Simulator's "Preferences" registry values under the current user's HKCU registry hive.
		/// </summary>
		/// <returns>Returns an objecct used to access the Corona Simulator's preferences for the current user.</returns>
		static std::shared_ptr<RegistryStoredPreferences> ForSimulatorPreferencesUserKey();

		/// <summary>
		///  <para>
		///   Gets a preconfigured RegistryStoredPreferences object that is used to provide read/write access to the
		///   Corona Simulator's "Preferences" registry values under the local machine's HKLM registry hive.
		///  </para>
		///  <para>Note: This section of the registry is only writable if the logged in user has admin permissions.</para>
		/// </summary>
		/// <returns>Returns an objecct used to access the Corona Simulator's preferences for the machine.</returns>
		static std::shared_ptr<RegistryStoredPreferences> ForSimulatorPreferencesMachineKey();

		/// <summary>
		///  <para>
		///   Gets a preconfigured RegistryStoredPreferences object that is used to provide access to the
		///   Corona Simulator's "Preferences" registry values under the HKCU and HKLM registry hives.
		///   This object will first attempt to access the HKCU for preferences, and if not found, will fallback
		///   to accessing the HKLM for preferences.
		///  </para>
		///  <para>Access to the HKLM registry hive is read-only. Even if the logged in user has admin access.</para>
		///  <para>The returned object is intended to mimic Apple's NSUserDefaults behavior on Mac.</para>
		/// </summary>
		/// <returns>
		///  Returns an object used to access the Corona Simulator's preferences under the HKCU and HKLM, in that order.
		/// </returns>
		static std::shared_ptr<StoredPreferencesChain> ForSimulatorPreferences();

//TODO: Implement this for sandboxed Corona Simulator projects that use the registry.
#if 0
		static std::shared_ptr<RegistryStoredPreferences> ForSimulatorProjectPath(const wchar_t* directoryPath);
#endif

	private:
		/// <summary>Provides registry component information extracted from a fully qualified registry path.</summary>
		struct ExtractComponentsResult
		{
			/// <summary>Handle to the registry hive such as HKCU, HKLM, etc.</summary>
			HKEY HiveKeyHandle;

			/// <summary>Path to the registry key to be accessed, excluding the hive name and value name.</summary>
			std::shared_ptr<std::wstring> KeyPath;

			/// <summary>
			///  <para>Name of the registry value to be accessed under the registry key specified by "KeyPath".</para>
			///  <para>Set to empty string if accessing the key's default value.</para>
			/// </summary>
			std::shared_ptr<std::wstring> ValueName;
		};

		/// <summary>Extracts the registry components from a fully qualified registry path.</summary>
		/// <param name="path">The fully qualified registry path to be parsed.</param>
		/// <returns>
		///  <para>
		///   Returns a success result if the given registry path was valid and this function was able to extract
		///   its registry components. The result object's GetValue() method will return the extracted components.
		///  </para>
		///  <para>
		///   Returns a failure result if the given path was invalid.
		///   The result object's GetUtf8Message() method will provide an error message explaining what was wrong.
		///  </para>
		/// </returns>
		static Rtt::ValueResult<ExtractComponentsResult> ExtractComponentsFromRegistryPath(const wchar_t* path);


		/// <summary>
		///  <para>Stores the base registry path that preference key names will be appended to.</para>
		///  <para>Set to empty string if a base path was not set. Key names must be fully qualified in this case.</para>
		/// </summary>
		WinString fBaseRegistryPath;

		/// <summary>Determines how the registry should be accessed on a 64-bit machine.</summary>
		RegistryStoredPreferences::Wow64ViewType fWow64ViewType;

		/// <summary>Set true to interpret '/' characters as registry path separators.</summary>
		bool fIsUsingForwardSlashAsPathSeparator;
};

} }	// namespace Interop::Storage
