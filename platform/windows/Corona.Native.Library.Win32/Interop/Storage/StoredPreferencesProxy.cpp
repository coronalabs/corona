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
