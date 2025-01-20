//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StoredPreferencesChain.h"
#include "Rtt_PreferenceCollection.h"


namespace Interop { namespace Storage {

#pragma region Constructors/Destructors
StoredPreferencesChain::StoredPreferencesChain(const StoredPreferencesChain::ListType& collection)
:	fCollection(collection)
{
}

StoredPreferencesChain::~StoredPreferencesChain()
{
}

#pragma endregion


#pragma region Private Template Methods
template<class TReturnValue>
TReturnValue StoredPreferencesChain::InvokeChain(const std::function<TReturnValue(MStoredPreferences&)>& lambda)
{
	// Traverse all stored preferences in the chain in order.
	bool hasReceivedFirstResult = false;
	auto result = TReturnValue::FailedWith("The stored preferences chain is empty.");
	for (auto&& storedPreferencesPointer : fCollection)
	{
		// Validate.
		if (!storedPreferencesPointer)
		{
			continue;
		}

		// Invoke the given callback for the next stored preferences object in the chain.
		auto nextResult = lambda(*storedPreferencesPointer);

		// If the above callback succeeded, then we're down. Return its result.
		if (nextResult.HasSucceeded())
		{
			result = nextResult;
			break;
		}

		// The invoked callback failed.
		// Store the first failure result. We'll return it if all other objects in the chain too.
		if (!hasReceivedFirstResult)
		{
			result = nextResult;
			hasReceivedFirstResult = true;
		}
	}
	return result;
}

#pragma endregion


#pragma region Public Methods
Rtt::Preference::ReadValueResult StoredPreferencesChain::Fetch(const char* keyName)
{
	auto lambda = [keyName](MStoredPreferences& storedPreferences)->Rtt::Preference::ReadValueResult
	{
		return storedPreferences.Fetch(keyName);
	};
	return InvokeChain<Rtt::Preference::ReadValueResult>(lambda);
}

Rtt::OperationResult StoredPreferencesChain::UpdateWith(const Rtt::Preference& preference)
{
	auto lambda = [preference](MStoredPreferences& storedPreferences)->Rtt::OperationResult
	{
		return storedPreferences.UpdateWith(preference);
	};
	return InvokeChain<Rtt::OperationResult>(lambda);
}

Rtt::OperationResult StoredPreferencesChain::UpdateWith(const Rtt::PreferenceCollection& preferences)
{
	auto lambda = [preferences](MStoredPreferences& storedPreferences)->Rtt::OperationResult
	{
		return storedPreferences.UpdateWith(preferences);
	};
	return InvokeChain<Rtt::OperationResult>(lambda);
}

Rtt::OperationResult StoredPreferencesChain::Delete(const char* keyName)
{
	auto lambda = [keyName](MStoredPreferences& storedPreferences)->Rtt::OperationResult
	{
		return storedPreferences.Delete(keyName);
	};
	return InvokeChain<Rtt::OperationResult>(lambda);
}

Rtt::OperationResult StoredPreferencesChain::Delete(const char* keyNameArray[], int count)
{
	auto lambda = [keyNameArray, count](MStoredPreferences& storedPreferences)->Rtt::OperationResult
	{
		return storedPreferences.Delete(keyNameArray, count);
	};
	return InvokeChain<Rtt::OperationResult>(lambda);
}

#pragma endregion

} }	// namespace Interop::Storage
