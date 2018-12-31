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

#ifndef _Rtt_PreferenceCollection_H__
#define _Rtt_PreferenceCollection_H__

/*
  Only define the following if C++11 std::unordered_map<> is supported. Provides fast O(1) hash table support.
  If not defined, the below class will use the C++ std::map<> type instead, which is O(log(n)).
  Note: Corona's Win32 implementation depends on this for fast access to cached SQLite stored preferences.
 */
#if defined(Rtt_WIN_ENV)
#	define Rtt_PREFERENCE_COLLECTION_USES_UNORDERED_MAP
#endif


#include "Core/Rtt_SharedStringPtr.h"
#include "Rtt_Preference.h"
#include "Rtt_PreferenceValue.h"
#include <vector>
#ifdef Rtt_PREFERENCE_COLLECTION_USES_UNORDERED_MAP
#	include <unordered_map>
#else
#	include <map>
#endif


namespace Rtt
{

/** Stores a collection of "Preference" key-value pair objects. */
class PreferenceCollection
{
	public:
		/** Creates an empty preference collection. */
		PreferenceCollection();

		/** Creates a collection containing a deep copy of the given preference collection. */
		PreferenceCollection(const PreferenceCollection& collection);

		/** Destroys this collection and its preference objects. */
		virtual ~PreferenceCollection();

		/**
		  Adds a new preference to the collection with the given name and value.

		  Note: This method call will fail if the given preference name already exists in the collection.

		  @param keyName
		  The unique name of the preference to be added in UTF-8 form.
		  If given a null pointer, then the preference name will be assigned an empty string.

		  @param value
		  The value to be assigned to the new preference.

		  @return
		  Returns true if successfully added to the collection.

		  Returns false if the key name already exists in the collection.
		 */
		bool Add(const char* keyName, const PreferenceValue& value);

		/**
		  Adds a new preference to the collection with the given name and value.

		  Note: This method call will fail if the given preference name already exists in the collection.

		  @param keyName
		  The unique name of the preference to be added in UTF-8 form.
		  If given a null pointer, then the preference name will be assigned an empty string.

		  @param value
		  The value to be assigned to the new preference.

		  @return
		  Returns true if successfully added to the collection.

		  Returns false if the key name already exists in the collection.
		 */
		bool Add(const Rtt::SharedConstStdStringPtr& keyName, const PreferenceValue& value);

		/**
		  Adds a copy of the given preference to the collection.

		  Note: This method call will fail if the given preference's name already exists in the collection.

		  @param preference The preference to be copied into the collection.

		  @return
		  Returns true if successfully added to the collection.

		  Returns false if the given preference's name already exists in the collection.
		 */
		bool Add(const Preference& preference);

		/**
		  Removes a preference from the collection by its zero based index.
		  @param index Zero based index to the preference to be removed.
		  @return
		  Returns true if the indexed preference has been removed.

		  Returns false if given an invalid index.
		 */
		bool RemoveByIndex(int index);

		/**
		  Removes a preference from the collection by its unique name.
		  @param keyName Unique name of the preference to be removed in UTF-8 form.
		  @return
		  Returns true if the preference was removed.

		  Returns false if the given preference name was not found.
		 */
		bool RemoveByKey(const char* keyName);

		/** Removes all preferences from the collection. */
		void Clear();

		/**
		  Gets the number of preferences stored in this collection.
		  @return
		  Returns the number of preferences stored in this collection.

		  Returns zero if the collection is empty.
		 */
		int GetCount() const;

		/**
		  Fetches a preference from the collection by its zero based index.
		  @param index Zero based index of the preference to fetch.
		  @return
		  Returns a pointer to the indexed preference.

		  Returns null if given an invalid index.
		 */
		Preference* GetByIndex(int index) const;

		/**
		  Fetches a preference from the collection by its unique name.
		  @param keyName Unique name of the preference to fetch in UTF-8 form.
		  @return
		  Returns a pointer to the referenced preference.

		  Returns null if the given preference name was not found.
		 */
		Preference* GetByKey(const char* keyName) const;

		/**
		  Determines if the given preference name exists in the collection.
		  @param keyName Unique name of the preference to search for in UTF-8 form.
		  @return
		  Returns true if the given preference name was found in the collection.

		  Returns false if the given name was not found.
		 */
		bool ContainsKey(const char* keyName) const;

		/**
		  Copies the preferences from the given collection to this collection.

		  Preferences from the given collection will overwrite the values of this collection's matching preferences.
		  Any given preferences that don't exist in this collection will be added.
		  @param collection The collection of preference to copy from.
		 */
		void CopyFrom(const PreferenceCollection& collection);

		/** Clears this collection and copies over the preferences from the given collection. */
		void operator=(const PreferenceCollection& collection);

	private:
		/* The below defines the map type and its fast custom string hasher/comparer to be used by this class. */
#ifdef Rtt_PREFERENCE_COLLECTION_USES_UNORDERED_MAP
		struct StlStringPointerHashProvider : public std::hash<const char*>
		{
			size_t operator()(const char* stringPointer) const;
		};
		struct StlStringPointerEqualityComparer : public std::equal_to<const char*>
		{
			bool operator()(const char* string1, const char* string2) const;
		};

		/** The type used by member variable "fMap". */
		typedef std::unordered_map<const char*, Rtt::Preference*, StlStringPointerHashProvider, StlStringPointerEqualityComparer> MapType;
#else
		struct StlMapStringPointerIsLessThanComparer : public std::map<const char*, Rtt::Preference*>::key_compare
		{
			bool operator()(const char* string1, const char* string2) const;
		};

		/** The type used by member variable "fMap". */
		typedef std::map<const char*, Rtt::Preference*, StlMapStringPointerIsLessThanComparer> MapType;
#endif

		/** The type used by member variable "fList". */
		typedef std::vector<Preference*> ListType;


		/** Hash table providing quick access to preference via their unique names. */
		PreferenceCollection::MapType fMap;

		/** Indexable list of preferences stored by this collection. */
		PreferenceCollection::ListType fList;
};

}	// namespace Rtt

#endif	// _Rtt_PreferenceCollection_H__
