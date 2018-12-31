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


#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Rtt_AndroidStoreProvider.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "NativeToJavaBridge.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Creates a new object used to provide an appropriate store accessing object for the current platform.
/// @param handle The Lua state wrapper needed to create objects in the runtime and raise event in Lua.
AndroidStoreProvider::AndroidStoreProvider( const ResourceHandle<lua_State>& handle, NativeToJavaBridge *ntjb )
:	PlatformStoreProvider( handle ),
	fActiveStore( NULL ),
	fTargetedStoreName( NULL ),
	fNativeToJavaBridge(ntjb)
{
	// Fetch all available stores on this device and store their names to the base class' array.
	ntjb->GetAvailableStoreNames(GetAvailableStores());
}

/// Destructor. Cleans up allocated resources.
AndroidStoreProvider::~AndroidStoreProvider()
{
	Rtt_DELETE( fActiveStore );
	Rtt_DELETE( fTargetedStoreName );
}

/// Assign a store to the provider to be returned by the GetActiveStore() function.
/// @param storeName The unique name of the store to provide access to.
///                  Set to NULL or empty string to have the platform automatically select a default store, if possible.
void
AndroidStoreProvider::SetActiveStore( const char *storeName )
{
	// Create the store accessing object, if not done already.
	// The Android store object created here has access to all store types on the Java side.
	if (!fActiveStore)
	{
		Runtime *runtimePointer = LuaContext::GetRuntime(GetLuaHandle().Dereference());
		if (runtimePointer)
		{
			fActiveStore = Rtt_NEW(runtimePointer->Allocator(), AndroidStore(GetLuaHandle(), fNativeToJavaBridge));
		}
		if (!fActiveStore)
		{
			// Failed to create the store accessing object. Give up here.
			return;
		}
	}
	
	// Tell the Java side of Corona to use the given store.
	fActiveStore->UseStore(storeName);
}

/// Gets the store accessing object that was assigned to this provider via the SetActiveStore() function.
/// @return Returns a pointer to a store accessing object.
///         Returns NULL if a store was not assigned to this provider or if a store is not available on this platform.
PlatformStore*
AndroidStoreProvider::GetActiveStore()
{
	return fActiveStore;
}

/// Gets the unique name of the store this application is targeting.
/// @return Returns the unique name of the targetd store such as "apple", "google", "amazon", etc.
///         Returns NULL if this application is not targeting a store.
const char*
AndroidStoreProvider::GetTargetedStoreName()
{
	// Create the string object if not done already.
	if (!fTargetedStoreName)
	{
		Runtime *runtimePointer = LuaContext::GetRuntime(GetLuaHandle().Dereference());
		if (runtimePointer)
		{
			fTargetedStoreName = Rtt_NEW(runtimePointer->Allocator(), String(runtimePointer->Allocator()));
		}
		if (!fTargetedStoreName)
		{
			return NULL;
		}
	}

	// Return the targeted store name.
	fNativeToJavaBridge->GetTargetedStoreName(fTargetedStoreName);
	return fTargetedStoreName->GetString();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
