//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StoredPreferencesProxy.h"
#include "Rtt_PreferenceCollection.h"


namespace Interop { namespace Storage {

#pragma region Private Constants
static const char kProxyNotAssignedMessage[] = "Proxy has not been assigned an MStoredPreferences object.";

static const char kProxyIsReadOnlyMessage[] = "Access to stored preferences is read-only.";

#pragma endregion


#pragma region Constructors/Destructors
StoredPreferencesProxy::StoredPreferencesProxy()
:	StoredPreferencesProxy(nullptr)
{
}

StoredPreferencesProxy::StoredPreferencesProxy(
	const std::shared_ptr<MStoredPreferences>& storedPreferencesPointer, bool isReadOnly /*= false*/)
:	fStoredPreferencesPointer(storedPreferencesPointer),
	fIsReadOnly(isReadOnly)
{
}

StoredPreferencesProxy::~StoredPreferencesProxy()
{
}

#pragma endregion


#pragma region Public Methods
Rtt::Preference::ReadValueResult StoredPreferencesProxy::Fetch(const char* keyName)
{
	if (!fStoredPreferencesPointer)
	{
		return Rtt::Preference::ReadValueResult::FailedWith(kProxyNotAssignedMessage);
	}
	return fStoredPreferencesPointer->Fetch(keyName);
}

Rtt::OperationResult StoredPreferencesProxy::UpdateWith(const Rtt::Preference& preference)
{
	if (!fStoredPreferencesPointer)
	{
		return Rtt::OperationResult::FailedWith(kProxyNotAssignedMessage);
	}
	if (fIsReadOnly)
	{
		return Rtt::OperationResult::FailedWith(kProxyIsReadOnlyMessage);
	}
	return fStoredPreferencesPointer->UpdateWith(preference);
}

Rtt::OperationResult StoredPreferencesProxy::UpdateWith(const Rtt::PreferenceCollection& preferences)
{
	if (!fStoredPreferencesPointer)
	{
		return Rtt::OperationResult::FailedWith(kProxyNotAssignedMessage);
	}
	if (fIsReadOnly)
	{
		return Rtt::OperationResult::FailedWith(kProxyIsReadOnlyMessage);
	}
	return fStoredPreferencesPointer->UpdateWith(preferences);
}

Rtt::OperationResult StoredPreferencesProxy::Delete(const char* keyName)
{
	if (!fStoredPreferencesPointer)
	{
		return Rtt::OperationResult::FailedWith(kProxyNotAssignedMessage);
	}
	if (fIsReadOnly)
	{
		return Rtt::OperationResult::FailedWith(kProxyIsReadOnlyMessage);
	}
	return fStoredPreferencesPointer->Delete(keyName);
}

Rtt::OperationResult StoredPreferencesProxy::Delete(const char* keyNameArray[], int count)
{
	if (!fStoredPreferencesPointer)
	{
		return Rtt::OperationResult::FailedWith(kProxyNotAssignedMessage);
	}
	if (fIsReadOnly)
	{
		return Rtt::OperationResult::FailedWith(kProxyIsReadOnlyMessage);
	}
	return fStoredPreferencesPointer->Delete(keyNameArray, count);
}

bool StoredPreferencesProxy::IsReadOnly() const
{
	return fIsReadOnly;
}

#pragma endregion


#pragma region Public Static Functions
std::shared_ptr<StoredPreferencesProxy> StoredPreferencesProxy::CreateReadOnlyUsing(
	const std::shared_ptr<MStoredPreferences>& storedPreferencesPointer)
{
	bool isReadOnly = true;
	return std::make_shared<StoredPreferencesProxy>(storedPreferencesPointer, isReadOnly);
}

#pragma endregion

} }	// namespace Interop::Storage
