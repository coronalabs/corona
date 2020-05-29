//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_TargetAndroidAppStore.h"
#include <list>


namespace Rtt
{

/// Android build which does not target an app store. Intended for generic Android builds.
const TargetAndroidAppStore TargetAndroidAppStore::kNone("none", "-None-", TargetDevice::kAndroidPlatform);

/// Android build targeting the Google Play app store.
const TargetAndroidAppStore TargetAndroidAppStore::kGoogle("google", "Google Play", TargetDevice::kAndroidPlatform);

/// Android build targeting the Amazon app store.
const TargetAndroidAppStore TargetAndroidAppStore::kAmazon("amazon", "Amazon", TargetDevice::kKindlePlatform);

/// Android build targeting the Ouya app store.
const TargetAndroidAppStore TargetAndroidAppStore::kOuya("ouya", "Ouya", TargetDevice::kAndroidPlatform);

/// Android build targeting the GameStick app store.
const TargetAndroidAppStore TargetAndroidAppStore::kGameStick("gameStick", "GameStick", TargetDevice::kAndroidPlatform);


/// Android app store collection type.
typedef std::list<TargetAndroidAppStore*> TargetAndroidAppStoreList;

/// Gets a static collection which stores all predefined app store targets up above.
/// This collection is populated by this class' private constructor.
/// @return Returns a collection of all supports Android app stores.
static TargetAndroidAppStoreList& GetCollection()
{
	static TargetAndroidAppStoreList sAppStoreCollection;
	return sAppStoreCollection;
}

/// Callback to be passed into the TargetAndroidAppStoreList.sort() function.
/// Will sort store names alphabetically, but always put store name "None" at the end.
static bool OnCompareCollectionItems(
	TargetAndroidAppStore *itemPointer1, TargetAndroidAppStore *itemPointer2)
{
	// If item 1 is NULL, then put it before item 2.
	if (!itemPointer1)
	{
		return true;
	}

	// If item 2 is NULL, then put it before item 1.
	if (!itemPointer2)
	{
		return false;
	}
	
	// Always put app store target "None" at the end of the collection.
	if (&TargetAndroidAppStore::kNone == itemPointer1)
	{
		return false;
	}
	if (&TargetAndroidAppStore::kNone == itemPointer2)
	{
		return true;
	}

	// Compare items by app store name.
	return (strcmp(itemPointer1->GetName(), itemPointer2->GetName()) <= 0);
}


/// Creates a new Android app store build target.
/// @param storeStringId The unique string ID of the app store to be used by the build process.
/// @param storeName The name of the app store to be displayed to the end-user.
/// @param platform The platform the app will be built for.
TargetAndroidAppStore::TargetAndroidAppStore(
	const char *storeStringId, const char *storeName, TargetDevice::Platform platform)
{
	// Store given app store settings.
	fStoreStringId.Set(storeStringId);
	fStoreName.Set(storeName);
	fPlatform = platform;
	
	// Add this app store to the collection.
	GetCollection().push_back(this);
	GetCollection().sort(OnCompareCollectionItems);
}

/// Destroys the app store target.
TargetAndroidAppStore::~TargetAndroidAppStore()
{
}

/// Gets the unique string ID of the app store being targeted.
/// @return Returns the name of the store being targeted such as "apple", "google", "amazon", etc.
const char* TargetAndroidAppStore::GetStringId() const
{
	return fStoreStringId.GetString();
}

/// Gets the name of the app store to be displayed to the end-user.
/// @return Returns a human readable name of the app store.
const char* TargetAndroidAppStore::GetName() const
{
	return fStoreName.GetString();
}

/// Gets the platform the app will be built for.
/// @return Returns the a platform such as kIPhonePlatform, kAndroidPlatform, etc.
TargetDevice::Platform TargetAndroidAppStore::GetPlatform() const
{
	return fPlatform;
}

/// Gets the total number of Android app store targets that Corona supports.
/// To be used in conjunction with the TargetAndroidAppStore::GetByIndex() function
/// so that you can for loop through all of the available Android app stores.
/// @return Returns the number of supported Android app stores.
int TargetAndroidAppStore::GetCount()
{
	return (int) GetCollection().size();
}

/// Fetches an Android app store build target by its zero based index.
/// Intended to be used in conjunction with the TargetAndroidAppStore::GetCont() function
/// to for loop through all of the Android app stores that Corona supports.
/// The indexed stores are in alphabetical order, except for "None" which is always at the end.
/// @return Returns a pointer to the indexed Android app store.
///         Returns NULL if the given index is out of range.
TargetAndroidAppStore* TargetAndroidAppStore::GetByIndex(int index)
{
	TargetAndroidAppStoreList::iterator iter;

	// Validate.
	if ((index < 0) || (index >= GetCount()))
	{
		return NULL;
	}
	
	// Fetch the indexed store.
	TargetAndroidAppStoreList &collection = GetCollection();
	for (iter = collection.begin(); iter != collection.end(); iter++)
	{
		if (index <= 0)
		{
			return *iter;
		}
		index--;
	}

	// Store not found.
	return NULL;
}

/// Fetches an Android app store build target by its unique string ID.
/// @param stringId Unique ID of the store such as "google", "amazon", "nook", etc.
/// @return Returns a pointer to the specified store.
///         Returns NULL if a store having the given ID was not found.
TargetAndroidAppStore* TargetAndroidAppStore::GetByStringId(const char *stringId)
{
	TargetAndroidAppStoreList::iterator iter;
	TargetAndroidAppStore *storePointer;

	// Validate.
	if (!stringId)
	{
		return NULL;
	}
	
	// Fetch the store by ID.
	TargetAndroidAppStoreList &collection = GetCollection();
	for (iter = collection.begin(); iter != collection.end(); iter++)
	{
		storePointer = *iter;
		if (storePointer && storePointer->GetStringId())
		{
			if ((storePointer->GetStringId() == stringId) ||
			    (strcmp(storePointer->GetStringId(), stringId) == 0))
			{
				// Store found. Return a pointer to it.
				return storePointer;
			}
		}
	}
	
	// Failed to find a store matching the given string ID.
	return NULL;
}

} // namespace Rtt
