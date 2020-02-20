//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Rtt_WinRTStoreProvider.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Core/Rtt_String.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_Runtime.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constants
/// Unique name assigned to Microsoft's Windows store.
static const char* kWindowsStoreName = "windows";

#pragma endregion


#pragma region Constructors/Destructors
/// Creates a new object used to provide an appropriate store accessing object for the current platform.
/// @param handle The Lua state wrapper needed to create objects in the runtime and raise event in Lua.
WinRTStoreProvider::WinRTStoreProvider(const ResourceHandle<lua_State>& handle)
	: PlatformStoreProvider(handle),
	fActiveStore(NULL)
{
	Runtime *runtime = LuaContext::GetRuntime(GetLuaHandle().Dereference());
	if (runtime)
	{
		GetAvailableStores().Append(Rtt_NEW(runtime->Allocator(), String(runtime->Allocator(), kWindowsStoreName)));
	}
}

/// Destructor. Cleans up allocated resources.
WinRTStoreProvider::~WinRTStoreProvider()
{
	Rtt_DELETE(fActiveStore);
}

#pragma endregion


#pragma region Public Member Functions
/// Assign a store to the provider to be returned by the GetActiveStore() function.
/// @param storeName The unique name of the store to provide access to.
///                  Set to NULL or empty string to have the platform automatically select a default store, if possible.
void WinRTStoreProvider::SetActiveStore(const char *storeName)
{
	// Do not allow the store to be changed once one has been activated.
	if (fActiveStore)
	{
		return;
	}

	// Automatically select the Apple store if a name was not provided.
	if (!storeName || (strlen(storeName) < 1))
	{
		storeName = kWindowsStoreName;
	}

	// Delete access to the last store, if assigned.
	Rtt_DELETE(fActiveStore);
	fActiveStore = NULL;

	// Create the specified store accessing object if available on this system/device.
	if (IsStoreAvailable(storeName))
	{
		Runtime *runtime = LuaContext::GetRuntime(GetLuaHandle().Dereference());
		if (runtime)
		{
			fActiveStore = Rtt_NEW(runtime->Allocator(), WinRTStore(GetLuaHandle()));
		}
	}
}

/// Gets the store accessing object that was assigned to this provider via the SetActiveStore() function.
/// @return Returns a pointer to a store accessing object.
///         Returns NULL if a store was not assigned to this provider or if a store is not available on this platform.
PlatformStore* WinRTStoreProvider::GetActiveStore()
{
	return fActiveStore;
}

/// Gets the unique name of the store this application is targeting.
/// @return Returns the unique name of the targetd store such as "apple", "google", "amazon", etc.
///         Returns NULL if this application is not targeting a store.
const char* WinRTStoreProvider::GetTargetedStoreName()
{
	return kWindowsStoreName;
}

#pragma endregion

} // namespace Rtt
