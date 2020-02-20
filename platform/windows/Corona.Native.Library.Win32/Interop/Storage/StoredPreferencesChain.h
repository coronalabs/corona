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
#include "MStoredPreferences.h"
#include <functional>
#include <list>
#include <memory>


namespace Interop { namespace Storage {

/// <summary>
///  <para>Stores an ordered collection of MStoredPreferences derived objects.</para>
///  <para>
///   When requesting a StoredPreferencesChain to read, write, and delete preferences, the chain will attempt
///   to perform that operation with all of the MStoredPreferences objects in the collection in order, stopping
///   at the first MStoredPreferences to succeed at that operation.
///  </para>
///  <para>
///   The intent of this class is to simulate the Apple NSUserDefaults behavior where application preferences are
///   access first, and if not found, it falls-back to accessing the global machine preferences. On Windows,
///   this class is expected to be used to access the current user's registry entries first (HKCU) before falling
///   back to the machine's (HKLM) registry settings.
///  </para>
/// </summary>
class StoredPreferencesChain : public MStoredPreferences
{
	Rtt_CLASS_NO_COPIES(StoredPreferencesChain)

	public:
		/// <summary>
		///  Collection of stored preference objects to be passed into the StoredPreferencesChain class' constructor.
		/// </summary>
		typedef std::list<std::shared_ptr<MStoredPreferences>> ListType;

		/// <summary>
		///  Creates a new stored preferences chain object which will use the given collection of
		///  stored preference accessing objects.
		/// </summary>
		/// <param name="collection">
		///  <para>Collection of stored preference accessing objects that the StoredPreferencesChain will use.</para>
		///  <para>Order matters. The first object in this collection will always be invoked first.</para>
		/// </param>
		StoredPreferencesChain(const StoredPreferencesChain::ListType& collection);

		/// <summary>Destroys this object.</summary>
		virtual ~StoredPreferencesChain();

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
		template<class TReturnValue>
		/// <summary>
		///  Invokes the given callback for each stored preference object in the collection in order.
		///  Will return immediately upon the first success result returned by the given callback.
		/// </summary>
		/// <param name="lambda">
		///  Callback to be invoked, passing the next stored preference object in the chain.
		/// </param>
		/// <returns>Returns a success or failure result from a stored preference object in the chain.</returns>
		TReturnValue InvokeChain(const std::function<TReturnValue(MStoredPreferences&)>& lambda);


		/// <summary>Ordered collection of objects used to access stored preferences.</summary>
		StoredPreferencesChain::ListType fCollection;
};

} }	// namespace Interop::Storage
