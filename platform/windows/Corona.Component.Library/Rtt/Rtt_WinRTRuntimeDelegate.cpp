// ----------------------------------------------------------------------------
// 
// Rtt_WinRTRuntimeDelegate.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTRuntimeDelegate.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeEnvironment.h"
#include "CoronaLabs\Corona\WinRT\Interop\Networking\NativeCoronaNetworkLibraryHandler.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Core/Rtt_String.h"
#	include "Rtt_CKWorkflow.h"
#	include "Rtt_DependencyLoader.h"
#	include "Rtt_Lua.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_Runtime.h"
#	include "Rtt_WinRTPlatform.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Stubbed Out Lua Functions
/// <summary>Called by Lua when it invokes a function that should always return false.</summary>
/// <remarks>This is a temporary function intended to stub out implementation for a Lua library's function.</remarks>
/// <param name="L">Pointer to a Lua state that invoked this function.</param>
/// <returns>Returns 1, indicating that a boolean return value was pushed back into Lua.</returns>
static int LuaFunctionStubReturningFalse(lua_State *L)
{
	lua_pushboolean(L, 0);
	return 1;
}

/// <summary>Called by Lua when it invokes a function with no return value.</summary>
/// <remarks>This is a temporary function intended to stub out implementation for a Lua library's function.</remarks>
/// <param name="L">Pointer to a Lua state that invoked this function.</param>
/// <returns>Returns 0 indicating that nothing was pushed back into Lua as a return value.</returns>
static int LuaFunctionStub(lua_State *L)
{
	return 0;
}

#pragma endregion


#pragma region Constructors/Destructors
WinRTRuntimeDelegate::WinRTRuntimeDelegate(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment)
:	fEnvironment(environment),
	RuntimeDelegatePlayer()
{
	if (!environment)
	{
		throw ref new Platform::NullReferenceException("environment");
	}
}

WinRTRuntimeDelegate::~WinRTRuntimeDelegate()
{
}

#pragma endregion


#pragma region Public Member Functions
void WinRTRuntimeDelegate::DidInitLuaLibraries(const Runtime& sender) const
{
	// If the runtime is using the CoronaCards feature, then add Corona's resource directory path to Lua's package path.
	lua_State *L = sender.VMContext().L();
	if (sender.IsProperty(Rtt::Runtime::kIsCoronaKit))
	{
		LuaContext::InitializeLuaPath(L, sender.Platform());
	}

	// Push in Lua libraries that are normally provided as separate plugins, such as the "network" library.
	// These are core libraries that are normally provided anyways. It is more convenient to make them a part of this library.
	CoronaLabs::Corona::WinRT::Interop::Networking::NativeCoronaNetworkLibraryHandler::RegisterTo(fEnvironment);

	// Push in Corona's other Lua libraries such as "socket", "mime", and "ltn12".
	// Note: These libraries are not typically provided by CoronaCards, but we offer them on Windows Phone.
	RuntimeDelegatePlayer::DidInitLuaLibraries(sender);

//TODO: Add plugin path to Lua::InsertPackageLoader() function.
}

bool WinRTRuntimeDelegate::HasDependencies(const Runtime& sender) const
{
#if 1
	// CoronaCards for WP8 is now free and should no longer require a license file.
	return true;
#else
	// Always return true if not using CoronaCards/Kit.
	// Note: Authorization is typically done via the signature attached to the "resource.car" in this case.
	bool isNotUsingCoronaKit = !sender.IsProperty(Rtt::Runtime::kIsCoronaKit);
	if (isNotUsingCoronaKit)
	{
		return true;
	}

	// *** The runtime is set up for CoronaCards. Handle its licensing below. ***

	// Check if the CoronaCards license file exists.
	bool wasLicenseFileFound = false;
	{
		FILE *fileHandle = nullptr;
		auto utf16LicenseFilePath = fEnvironment->ResourceDirectoryPath + L"\\license.ccdata";
		::_wfopen_s(&fileHandle, utf16LicenseFilePath->Data(), L"r");
		if (fileHandle)
		{
			::fclose(fileHandle);
			wasLicenseFileFound = true;
		}
	}

	// Display a trial watermark if the license was not found.
	// Note: Android and iOS display a native alert message instead, which exits the app when a button has been pressed.
	//       We're lowerig the barrier on this platform so that the Corona project created by our Visual Studio extension
	//       will "just work", which would hopefully provide a better developer experience when trying out our SDK.
	if (false == wasLicenseFileFound)
	{
		// Flag the Corona runtime to display a trial watermark.
		sender.SetShowingTrialMessage(true);

		// Log a message indicating where the developer can obtain a valid license file.
		Rtt::CKWorkflow workflow;
		auto alertSettings = workflow.CreateAlertSettingsFor(Rtt::CKWorkflow::kMissingLicense, "coronacards");
		Rtt_LogException(
				"[CoronaCards Trial Notice]\r\n"
				"   Corona is currently in trial mode. To obtain a valid license, please go here:\r\n"
				"   %s\r\n",
				alertSettings.ActionButtonUrl.c_str());
		return true;
	}

	// Load and validate the CoronaCards license.
	return Corona::DependencyLoader::CCDependencyCheck(sender);
#endif
}

void WinRTRuntimeDelegate::WillLoadMain(const Runtime& sender) const
{
	lua_State *L = sender.VMContext().L();

	// Raise a "Loaded" event on the WinRT side of Corona.
	fLoadedCallback.Invoke();

//TODO: Push launch arguments to Lua.
	if (Rtt_VERIFY(const_cast<Runtime&>(sender).PushLaunchArgs(true) > 0))
	{
//		NativeToJavaBridge::GetInstance()->PushLaunchArgumentsToLuaTable(L);
		lua_pop(L, 1);
	}
}

void WinRTRuntimeDelegate::DidLoadMain(const Runtime& sender) const
{
	// Raise a "Started" event on the WinRT side of Corona.
	fStartedCallback.Invoke();
}

void WinRTRuntimeDelegate::WillLoadConfig(const Runtime& sender, lua_State *L) const
{
}

void WinRTRuntimeDelegate::DidLoadConfig(const Runtime& sender, lua_State *L) const
{
}

void WinRTRuntimeDelegate::InitializeConfig(const Runtime& sender, lua_State *L) const
{
	static const char kApplication[] = "application";
	static const char kMetadata[] = "metadata";
	static const char kAppId[] = "appId";

	Rtt_LUA_STACK_GUARD(L);

	// Query for an "application" table in Lua. (Normally created via "config.lua", if provided.)
	lua_getglobal(L, kApplication);
	{
		// If an "application" table does not exist, then create one.
		if (!lua_istable(L, -1))
		{
			// Pop nil off of the stack.
			lua_pop(L, 1);

			// Push a new "application" table to the top of the stack and to the global table.
			lua_createtable(L, 0, 1);
			lua_setglobal(L, kApplication);
		}
		int applicationLuaTableIndex = lua_gettop(L);

		// Query for the "metadata" field, which is used by CoronaCards.
		lua_getfield(L, applicationLuaTableIndex, kMetadata);
		{
			// If a "metadata" table was not found, then create one.
			// Note: The CoronaCards licensing system requires this to exist.
			if (!lua_istable(L, -1))
			{
				// Pop nil off of the stack.
				lua_pop(L, 1);

				// Create a mock "metadata" table and add it to the global "application" table.
				lua_createtable(L, 0, 1);
				lua_pushvalue(L, -1);	// Leave "metadata" on stack. So, push extra.
				lua_setfield(L, applicationLuaTableIndex, kMetadata);
			}
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
}

void WinRTRuntimeDelegate::SetLoadedCallback(const WinRTMethodCallback &callback)
{
	fLoadedCallback = callback;
}

void WinRTRuntimeDelegate::SetStartedCallback(const WinRTMethodCallback &callback)
{
	fStartedCallback = callback;
}

#pragma endregion

} // namespace Rtt
