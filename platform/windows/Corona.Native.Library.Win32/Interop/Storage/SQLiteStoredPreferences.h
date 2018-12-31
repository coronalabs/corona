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
#include "Rtt_PreferenceCollection.h"
#include "WinString.h"
extern "C"
{
	struct sqlite3;
	typedef struct sqlite3 sqlite3;
}


namespace Interop { namespace Storage {

/// <summary>
///  <para>Reads, writes, and deletes an application's preferences to a SQLite database file.</para>
///  <para>Preferences are key-value pairs used to easily store simple data/settings.</para>
/// </summary>
class SQLiteStoredPreferences : public MStoredPreferences
{
	Rtt_CLASS_NO_COPIES(SQLiteStoredPreferences)

	public:
		/// <summary>Creates a preference accessing manager using a SQLite database file to store preferences.</summary>
		/// <param name="filePath">
		///  <para>Path to a SQLite database file used to store preferences.</para>
		///  <para>If the given file does not exist, then this object will automatically create the file.</para>
		/// </param>
		SQLiteStoredPreferences(const wchar_t* filePath);

		/// <summary>Destroys this object.</summary>
		virtual ~SQLiteStoredPreferences();

		/// <summary>Gets the file name and path of the SQLite database that preferences are stored to.</summary>
		/// <returns>Returns the file name and path of the SQLite database.</returns>
		const wchar_t* GetFilePath() const;

		/// <summary>Fetches a preference's value by its unique key name.</summary>
		/// <param name="keyName">The unique name of the preference in UTF-8 form.</param>
		/// <returns>
		///  <para>
		///   Returns a success result if the preference was found.
		///   The result object's GetValue() method will return the fetched preference's value.
		///  </para>
		///  <para>
		///   Returns a failure result if the preference key was not found or if there was a problem accessing storage.
		///   The result object's GetUtf8Message() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::Preference::ReadValueResult Fetch(const char* keyName) override;

		/// <summary>Writes the given preference key-value pair to storage.</summary>
		/// <param name="preference">The preference key name and value.</param>
		/// <returns>
		///  <para>Returns a success result if the given preference was written to storage.</para>
		///  <para>
		///   Returns a failure result if failed to write the given preference to storage.
		///   The result object's GetUtf8Message() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult UpdateWith(const Rtt::Preference& preference) override;

		/// <summary>Writes the given collection of preference key-value pairs to storage.</summary>
		/// <param name="preferences">Collection of preferences to write to storage. Can be empty.</param>
		/// <returns>
		///  <para>Returns a success result if all of the given preferences were written to storage.</para>
		///  <para>
		///   Returns a failure result if at least 1 preference has failed to write to storage.
		///   The result object's GetUtf8Message() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult UpdateWith(const Rtt::PreferenceCollection& preferences) override;

		/// <summary>Delete a preference from storage by its unique string key.</summary>
		/// <param name="keyName">The unique name of the preference to delete in UTF-8 form.</param>
		/// <returns>
		///  <para>Returns a success result if the preference was deleted from storage.</para>
		///  <para>
		///   Returns a failure result if failed to access storage or unable to delete the preference.
		///   The result object's GetUtf8Message() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult Delete(const char* keyName) override;

		/// <summary>Delete multiple preferences from storage by their unique string keys.</summary>
		/// <param name="keyNameArray">Array of unique preference names (in UTF-8 form) to delete from storage.</param>
		/// <param name="count">The number of keys/elements in the "keyNameArray" argument.</param>
		/// <returns>
		///  <para>Returns a success result if all of the given preferences have been deleted from storage.</para>
		///  <para>
		///   Returns a failure result if failed to access storage, if unable to delete at least 1 preference,
		///   or if "keyNameArray" is null.
		///   The result object's GetUtf8Message() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult Delete(const char* keyNameArray[], int count) override;

	private:
		class DatabaseConnection
		{
			Rtt_CLASS_NO_COPIES(DatabaseConnection)

			public:
				DatabaseConnection(SQLiteStoredPreferences& storedPreferences);
				virtual ~DatabaseConnection();

				bool IsOpen() const;
				Rtt::OperationResult OpenForReading();
				Rtt::OperationResult OpenForWriting();
				void Close();
				Rtt::OperationResult UpdateWith(const Rtt::PreferenceCollection& preferences);
				Rtt::OperationResult Delete(const char* keyNameArray[], int count);

			private:
				Rtt::OperationResult Open(bool isForWriting);

				SQLiteStoredPreferences& fStoredPreferences;
				sqlite3* fDatabasePointer;
		};


		/// <summary>The file name and path of the SQLite database file.</summary>
		WinString fFilePath;

		/// <summary>
		///  <para>Stores the last modified timestamp assigned to the SQLite database file.</para>
		///  <para>Used to determine if the database was modified externally by another application.</para>
		/// </summary>
		FILETIME fLastFileUpdateTime;

		/// <summary>Stores a copy of all preference key-value pairs fetched from the database for fast access.</summary>
		Rtt::PreferenceCollection fCachedPreferences;
};

} }	// namespace Interop::Storage
