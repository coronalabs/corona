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

#include "Rtt_Preference.h"
#include "Rtt_OperationResult.h"


namespace Rtt
{
	class PreferenceCollection;
}


namespace Interop { namespace Storage {

/// <summary>
///  <para>Interface to an object used to read, write, and delete an application's preferences to storage.</para>
///  <para>Preferences are key-value pairs used to easily store simple data/settings.</para>
/// </summary>
/// <remarks>
///  This is an abstract class. A derived class is expected to implement how preferences are stored,
///  such as to a SQLite database or to the Windows registys. This allows us to easily swap out the storage
///  implementation that the Corona "RuntimeEnvironment" class will use.
/// </remarks>
class MStoredPreferences
{
	public:
		/// <summary>Destroys this object.</summary>
		virtual ~MStoredPreferences() {}

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
		virtual Rtt::Preference::ReadValueResult Fetch(const char* keyName) = 0;

		/// <summary>Writes the given preference key-value pair to storage.</summary>
		/// <param name="preference">The preference key name and value.</param>
		/// <returns>
		///  <para>Returns a success result if the given preference was written to storage.</para>
		///  <para>
		///   Returns a failure result if failed to write the given preference to storage.
		///   The result object's GetUtf8Message() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult UpdateWith(const Rtt::Preference& preference) = 0;

		/// <summary>Writes the given collection of preference key-value pairs to storage.</summary>
		/// <param name="preferences">Collection of preferences to write to storage. Can be empty.</param>
		/// <returns>
		///  <para>Returns a success result if all of the given preferences were written to storage.</para>
		///  <para>
		///   Returns a failure result if at least 1 preference has failed to write to storage.
		///   The result object's GetUtf8Message() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult UpdateWith(const Rtt::PreferenceCollection& preferences) = 0;

		/// <summary>Delete a preference from storage by its unique string key.</summary>
		/// <param name="keyName">The unique name of the preference to delete in UTF-8 form.</param>
		/// <returns>
		///  <para>Returns a success result if the preference was deleted from storage.</para>
		///  <para>
		///   Returns a failure result if failed to access storage or unable to delete the preference.
		///   The result object's GetUtf8Message() method will return a string detailing why it failed.
		///  </para>
		/// </returns>
		virtual Rtt::OperationResult Delete(const char* keyName) = 0;

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
		virtual Rtt::OperationResult Delete(const char* keyNameArray[], int count) = 0;
};

} }	// namespace Interop::Storage
