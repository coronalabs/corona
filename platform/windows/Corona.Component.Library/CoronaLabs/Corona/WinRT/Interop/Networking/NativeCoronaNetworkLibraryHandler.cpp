// ----------------------------------------------------------------------------
// 
// NativeCoronaNetworkLibraryHandler.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "NativeCoronaNetworkLibraryHandler.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeEnvironment.h"
#include "CoronaLabs\Corona\WinRT\Interop\InteropServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\Logging\LoggingServices.h"
#include "CoronaLabs\WinRT\ImmutableByteBuffer.h"
#include "CoronaLabs\WinRT\IOperationResult.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#include "CoronaHttpRequestOperation.h"
#include "HttpRequestSettings.h"
#include "INetworkServices.h"
#include "NetworkConnectionStatus.h"
#include "Rtt_WinRTPlatform.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "CoronaLibrary.h"
#	include "CoronaLua.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_LuaLibSystem.h"
#	include "Rtt_MPlatform.h"
#	include "Rtt_Runtime.h"
#	include <limits>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


extern "C"
{
	extern int CoronaPluginLuaLoad_network(lua_State *L);
}


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

#pragma region Constants
const char NativeCoronaNetworkLibraryHandler::kLuaFinalizerMetatableName[] = "network.finalizer";

#pragma endregion


#pragma region Consructors/Destructors
NativeCoronaNetworkLibraryHandler::NativeCoronaNetworkLibraryHandler(CoronaRuntimeEnvironment^ environment)
{
	// Validate.
	if (nullptr == environment)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Initialize member variables.
	fEnvironment = environment;
	fNextRequestId = 1;
	fHttpRequestOperations = ref new Platform::Collections::Map<int, CoronaHttpRequestOperation^>();
}

NativeCoronaNetworkLibraryHandler::~NativeCoronaNetworkLibraryHandler()
{
	// Abort all active HTTP request operations.
	// Note: We must clone the "fHttpRequestOperations" first, because calling Abort() on each operation will
	//       cause this class' event handler to immediately remove that operation from "fHttpRequestOperations",
	//       which would invalidate its iterator in a foreach loop.
	auto httpRequestOperationList = ref new Platform::Collections::Vector<CoronaHttpRequestOperation^>();
	for (auto&& pair : fHttpRequestOperations)
	{
		httpRequestOperationList->Append(pair->Value);
	}
	for (auto&& operation : httpRequestOperationList)
	{
		operation->Abort();
	}
}

#pragma endregion


#pragma region Public Static Functions
bool NativeCoronaNetworkLibraryHandler::RegisterTo(CoronaRuntimeEnvironment^ environment)
{
	// Validate.
	if ((nullptr == environment) ||
	    (nullptr == environment->NativePlatformPointer) ||
	    (nullptr == environment->NativeRuntimePointer) ||
	    (nullptr == environment->InteropServices->NetworkServices))
	{
		return false;
	}

	// Fetch the Lua state.
	auto luaStatePointer = environment->NativeRuntimePointer->VMContext().L();
	if (nullptr == luaStatePointer)
	{
		return false;
	}

	// Do not continue if this library has already been registered into the given runtime's Lua state.
	luaL_getmetatable(luaStatePointer, kLuaFinalizerMetatableName);
	bool wasRegistered = lua_istable(luaStatePointer, -1) ? true : false;
	lua_pop(luaStatePointer, 1);
	if (wasRegistered)
	{
		return true;
	}
	
	// Create the library binding object.
	Rtt_Allocator& allocator = environment->NativePlatformPointer->GetAllocator();
	auto libraryHandlerPointer = Rtt_NEW(&allocator, NativeCoronaNetworkLibraryHandler(environment));
	if (nullptr == libraryHandlerPointer)
	{
		return false;
	}

	// Push the above object's pointer into the Lua registry as userdata.
	// Have Lua's finalizer delete this object from memory upon termination.
	::CoronaLuaInitializeGCMetatable(
			luaStatePointer, kLuaFinalizerMetatableName, NativeCoronaNetworkLibraryHandler::OnFinalizing);
	Rtt::Lua::PushUserdata(luaStatePointer, libraryHandlerPointer, kLuaFinalizerMetatableName);
	luaL_ref(luaStatePointer, LUA_REGISTRYINDEX);

	// Load the Lua "network" library and push it to the top of the stack.
	const luaL_Reg kVTable[] =
	{
		{ "getConnectionStatus", NativeCoronaNetworkLibraryHandler::OnGetConnectionStatus },
		{ "request", NativeCoronaNetworkLibraryHandler::OnRequest },
		{ "request_native", NativeCoronaNetworkLibraryHandler::OnRequest },
		{ "cancel", NativeCoronaNetworkLibraryHandler::OnCancel },
		{ NULL, NULL }
	};
	lua_CFunction factory = ::Corona::Lua::Open<CoronaPluginLuaLoad_network>;
	int pushedItemsCount = ::CoronaLibraryNewWithFactory(luaStatePointer, factory, kVTable, libraryHandlerPointer);
	if (pushedItemsCount <= 0)
	{
		return false;
	}
	if (lua_istable(luaStatePointer, 1) == 0)
	{
		lua_pop(luaStatePointer, pushedItemsCount);
		return false;
	}
	int libraryIndex = lua_gettop(luaStatePointer);

	// Add the library to Lua's "package.loaded" table so that it can be returned by Lua's require() function.
	wasRegistered = false;
	lua_getfield(luaStatePointer, libraryIndex, "name");
	auto libraryName = lua_tostring(luaStatePointer, -1);
	if (libraryName)
	{
		lua_getfield(luaStatePointer, LUA_GLOBALSINDEX, "package");
		lua_getfield(luaStatePointer, -1, "loaded");
		int loadedTableIndex = lua_gettop(luaStatePointer);
		lua_pushvalue(luaStatePointer, libraryIndex);
		lua_setfield(luaStatePointer, loadedTableIndex, libraryName);
		lua_pop(luaStatePointer, 2);
		wasRegistered = true;
	}
	lua_pop(luaStatePointer, 1);

	// Pop the library from the stack.
	lua_pop(luaStatePointer, pushedItemsCount);

	// Returns true if the library was successfully loaded/registered.
	return wasRegistered;
}

#pragma endregion


#pragma region Private Static Functions
int NativeCoronaNetworkLibraryHandler::OnGetConnectionStatus(lua_State *luaStatePointer)
{
	// Validate.
	if (nullptr == luaStatePointer)
	{
		return 0;
	}

	// Fetch the library binding object.
	auto libraryHandlerPointer = (NativeCoronaNetworkLibraryHandler*)lua_touserdata(luaStatePointer, lua_upvalueindex(1));
	if (nullptr == libraryHandlerPointer)
	{
		lua_pushnil(luaStatePointer);
		return 1;
	}

	// Fetch the network connection status.
	auto connectionStatus = libraryHandlerPointer->fEnvironment->InteropServices->NetworkServices->FetchConnectionStatus();

	// Push the connection status to a table.
	lua_createtable(luaStatePointer, 0, 2);
	lua_pushboolean(luaStatePointer, connectionStatus->IsConnected ? 1 : 0);
	lua_setfield(luaStatePointer, -2, "isConnected");
	lua_pushboolean(luaStatePointer, connectionStatus->IsUsingCelluarService ? 1 : 0);
	lua_setfield(luaStatePointer, -2, "isMobile");

	// Return the above Lua table.
	return 1;
}

int NativeCoronaNetworkLibraryHandler::OnRequest(lua_State *luaStatePointer)
{
	// Validate.
	if (nullptr == luaStatePointer)
	{
		return 0;
	}

	// Throw a Lua error if the 1st argument (ie: the URL argument) is not a string.
	if (lua_type(luaStatePointer, 1) != LUA_TSTRING)
	{
		luaL_error(luaStatePointer, "First argument to network.request() must be a URL string.");
	}

	// Fetch the library binding object.
	auto libraryHandlerPointer = (NativeCoronaNetworkLibraryHandler*)lua_touserdata(luaStatePointer, lua_upvalueindex(1));
	if (nullptr == libraryHandlerPointer)
	{
		lua_pushnil(luaStatePointer);
		return 1;
	}

	// Acquire a unique integer ID for the HTTP operation to be created below.
	int requestId = libraryHandlerPointer->fNextRequestId;
	for (; (0 == requestId) || libraryHandlerPointer->fHttpRequestOperations->HasKey(requestId); requestId++);
	libraryHandlerPointer->fNextRequestId = requestId + 1;

	// Create a new HTTP request operation.
	auto httpRequestOperation = ref new CoronaHttpRequestOperation(requestId, libraryHandlerPointer->fEnvironment);
	libraryHandlerPointer->fHttpRequestOperations->Insert(requestId, httpRequestOperation);
	httpRequestOperation->EndedEventHandler.Set(OnHttpRequestEnded, libraryHandlerPointer);
	auto settings = httpRequestOperation->Settings;

	// Fetch and validate the URL argument.
	int nextLuaArgumentIndex = 1;
	auto utf8Url = lua_tostring(luaStatePointer, nextLuaArgumentIndex);
	if (Rtt_StringIsEmpty(utf8Url))
	{
		CoronaLuaError(luaStatePointer, "Network API was given a nil or empty string.", utf8Url);
		lua_pushnil(luaStatePointer);
		return 1;
	}
	try
	{
		settings->Uri = ref new Windows::Foundation::Uri(CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8Url));
	}
	catch (...)
	{
		CoronaLuaError(luaStatePointer, "Network API was given an invalid URL of: %s", utf8Url);
		lua_pushnil(luaStatePointer);
		return 1;
	}
	nextLuaArgumentIndex++;

	// Fetch the optional HTTP method argument. (The default is "GET" if not assigned.)
	if (lua_type(luaStatePointer, nextLuaArgumentIndex) == LUA_TSTRING)
	{
		auto utf8HttpMethod = lua_tostring(luaStatePointer, nextLuaArgumentIndex);
		auto utf16HttpMethod = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8HttpMethod);
		if (utf16HttpMethod->IsEmpty() == false)
		{
			settings->MethodName = utf16HttpMethod;
		}
		nextLuaArgumentIndex++;
	}

	// Fetch the optional Lua listener argument.
	if (CoronaLuaIsListener(luaStatePointer, nextLuaArgumentIndex, Rtt::NetworkRequestEvent::kName))
	{
		httpRequestOperation->LuaListener = CoronaLuaNewRef(luaStatePointer, nextLuaArgumentIndex);
		nextLuaArgumentIndex++;
	}

	// Fetch the optional parameters table argument.
	bool wasBodyTypeProvided = false;
	bool isBodyTypeText = false;
	if (lua_istable(luaStatePointer, nextLuaArgumentIndex))
	{
		int luaParametersTableIndex = nextLuaArgumentIndex;

		// Fetch the HTTP header settings.
		lua_getfield(luaStatePointer, luaParametersTableIndex, "headers");
		if (lua_istable(luaStatePointer, -1))
		{
			int luaHeaderTableIndex = lua_gettop(luaStatePointer);
			for (lua_pushnil(luaStatePointer); lua_next(luaStatePointer, luaHeaderTableIndex); lua_pop(luaStatePointer, 1))
			{
				// Fetch the header name. (Must be of type string and non-empty.)
				if (lua_type(luaStatePointer, -2) != LUA_TSTRING)
				{
					continue;
				}
				auto utf8HeaderName = lua_tostring(luaStatePointer, -2);
				auto utf16HeaderName = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8HeaderName);
				if (utf16HeaderName->IsEmpty())
				{
					continue;
				}

				// Fetch the header value.
				Platform::String^ utf16HeaderValue = nullptr;
				auto luaValueType = lua_type(luaStatePointer, -1);
				switch (luaValueType)
				{
					case LUA_TSTRING:
					{
						auto utf8HeaderValue = lua_tostring(luaStatePointer, -1);
						utf16HeaderValue = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8HeaderValue);
						break;
					}
					case LUA_TNUMBER:
					{
						wchar_t utf16Buffer[64];
						double decimalValue = lua_tonumber(luaStatePointer, -1);
						double roundedDecimalValue =
									(double)(int64)((decimalValue >= 0.0) ? (decimalValue + 0.5) : (decimalValue - 0.5));
						if (std::abs(roundedDecimalValue - decimalValue) > std::numeric_limits<double>::epsilon())
						{
							swprintf_s(utf16Buffer, L"%lf", decimalValue);
						}
						else
						{
							_i64tow_s((int64)decimalValue, utf16Buffer, sizeof(utf16Buffer), 10);
						}
						utf16HeaderValue = ref new Platform::String(utf16Buffer);
						break;
					}
					case LUA_TBOOLEAN:
					{
						utf16HeaderValue = lua_toboolean(luaStatePointer, -1) ? L"true" : L"false";
						break;
					}
				}

				// Add the HTTP header entry to the configuration.
				settings->Headers->Insert(utf16HeaderName, utf16HeaderValue);
			}
		}
		lua_pop(luaStatePointer, 1);

		// Fetch the HTTP body type setting.
		lua_getfield(luaStatePointer, luaParametersTableIndex, "bodyType");
		if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
		{
			auto utf8BodyTypeName = lua_tostring(luaStatePointer, -1);
			if (utf8BodyTypeName)
			{
				if (_stricmp(utf8BodyTypeName, "binary") == 0)
				{
					isBodyTypeText = false;
					wasBodyTypeProvided = true;
				}
				else if (_stricmp(utf8BodyTypeName, "text") == 0)
				{
					isBodyTypeText = true;
					wasBodyTypeProvided = true;
				}
			}
		}
		lua_pop(luaStatePointer, 1);

		// Fetch the HTTP body content to upload.
		lua_getfield(luaStatePointer, luaParametersTableIndex, "body");
		if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
		{
			// Use a Lua string as the HTTP body content. Fetch any null characters that may be embedded in it too.
			size_t bufferLength = 0;
			auto buffer = lua_tolstring(luaStatePointer, -1, &bufferLength);
			if (buffer && (bufferLength > 0))
			{
				if (false == wasBodyTypeProvided)
				{
					isBodyTypeText = true;
				}
				if (isBodyTypeText)
				{
					// Send all bytes in the string up to the first null character found.
					settings->BytesToUpload = CoronaLabs::WinRT::ImmutableByteBuffer::From(buffer);
				}
				else
				{
					// Send all bytes in the Lua string, including null characters.
					// Note: In this case, the Lua string may be used as a binary array and is unprintable.
					settings->BytesToUpload = CoronaLabs::WinRT::ImmutableByteBuffer::From(buffer, bufferLength);
				}
			}
		}
		else if (lua_istable(luaStatePointer, -1))
		{
			// Fetch the name of the file whose content will be uploaded.
			const char* utf8FileName = nullptr;
			lua_getfield(luaStatePointer, -1, "filename");
			if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
			{
				utf8FileName = lua_tostring(luaStatePointer, -1);
			}
			lua_pop(luaStatePointer, 1);

			// Fetch the above file's relative base directory.
			lua_getfield(luaStatePointer, -1, "baseDirectory");
			auto baseDirectory = Rtt::LuaLibSystem::ToDirectory(luaStatePointer, -1, Rtt::MPlatform::kResourceDir);
			lua_pop(luaStatePointer, 1);

			// Create an absolute path to the above file, if it exists.
			// Note: The "filePath" variable will be empty if the file was not found or if "filename" was null/empty.
			Rtt::String filePath(&(libraryHandlerPointer->fEnvironment->NativePlatformPointer->GetAllocator()));
			libraryHandlerPointer->fEnvironment->NativePlatformPointer->PathForFile(
					utf8FileName, baseDirectory, Rtt::MPlatform::kTestFileExists, filePath);

			// Set up the HTTP request to upload the referenced file, if it exists.
			if (filePath.IsEmpty() == false)
			{
				if (false == wasBodyTypeProvided)
				{
					isBodyTypeText = false;
				}
				settings->UploadFilePath = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(filePath.GetString());
				settings->UploadFileEnabled = true;
			}
		}
		lua_pop(luaStatePointer, 1);

		// Fetch the HTTP response handling settings.
		lua_getfield(luaStatePointer, luaParametersTableIndex, "response");
		if (lua_istable(luaStatePointer, -1))
		{
			// Fetch the name of the file to download to.
			const char* utf8FileName = nullptr;
			lua_getfield(luaStatePointer, -1, "filename");
			if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
			{
				utf8FileName = lua_tostring(luaStatePointer, -1);
			}
			lua_pop(luaStatePointer, 1);

			// Fetch the above file's relative base directory.
			lua_getfield(luaStatePointer, -1, "baseDirectory");
			auto baseDirectory = Rtt::LuaLibSystem::ToDirectory(luaStatePointer, -1, Rtt::MPlatform::kDocumentsDir);
			if ((Rtt::MPlatform::kResourceDir == baseDirectory) || (Rtt::MPlatform::kSystemResourceDir == baseDirectory))
			{
				baseDirectory = Rtt::MPlatform::kDocumentsDir;
			}
			lua_pop(luaStatePointer, 1);

			// Create an absolute path to the above file, if provided.
			Rtt::String utf8FilePath(&(libraryHandlerPointer->fEnvironment->NativePlatformPointer->GetAllocator()));
			if (utf8FileName)
			{
				libraryHandlerPointer->fEnvironment->NativePlatformPointer->PathForFile(
						utf8FileName, baseDirectory, 0, utf8FilePath);
			}

			// Set up the download path to save the HTTP response to, if provided.
			if (utf8FilePath.IsEmpty() == false)
			{
				settings->DownloadFilePath = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8FilePath.GetString());
			}
		}
		lua_pop(luaStatePointer, 1);

		// Fetch the progress setting.
		lua_getfield(luaStatePointer, luaParametersTableIndex, "progress");
		if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
		{
			auto directionName = lua_tostring(luaStatePointer, -1);
			if (directionName)
			{
				if (_stricmp(directionName, "upload") == 0)
				{
					httpRequestOperation->ProgressEventsEnabled = true;
					httpRequestOperation->ProgressEventsDirection = CoronaHttpRequestOperation::TransmitDirection::kUpload;
				}
				else if (_stricmp(directionName, "download") == 0)
				{
					httpRequestOperation->ProgressEventsEnabled = true;
					httpRequestOperation->ProgressEventsDirection = CoronaHttpRequestOperation::TransmitDirection::kDownload;
				}
			}
		}
		lua_pop(luaStatePointer, 1);

		// Fetch the custom timeout setting.
		lua_getfield(luaStatePointer, luaParametersTableIndex, "timeout");
		if (lua_type(luaStatePointer, -1) == LUA_TNUMBER)
		{
			auto timeoutInSeconds = lua_tointeger(luaStatePointer, -1);
			if (timeoutInSeconds < 1)
			{
				timeoutInSeconds = 1;
			}
			Windows::Foundation::TimeSpan timeSpan;
			timeSpan.Duration = timeoutInSeconds * 10000000LL;
			settings->CustomTimeout = timeSpan;
			settings->DefaultTimeoutEnabled = false;
		}
		lua_pop(luaStatePointer, 1);

		// Fetch the redirect handling setting.
		lua_getfield(luaStatePointer, luaParametersTableIndex, "handleRedirects");
		if (lua_type(luaStatePointer, -1) == LUA_TBOOLEAN)
		{
			settings->AutoRedirectEnabled = lua_toboolean(luaStatePointer, -1) ? true : false;
		}
		lua_pop(luaStatePointer, 1);
	}

	// Check if an HTTP "Content-Type" header was provided by the caller.
	bool hasContentTypeHeader = false;
	for (auto&& headerEntry : settings->Headers)
	{
		if (_wcsicmp(headerEntry->Key->Data(), L"Content-Type") == 0)
		{
			hasContentTypeHeader = true;
			break;
		}
	}
	if (false == hasContentTypeHeader)
	{
		// An HTTP "Content-Type" was not assigned. Default it to one of the below if applicable.
		if (_wcsicmp(settings->MethodName->Data(), L"POST") == 0)
		{
			settings->Headers->Insert(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
		}
		else if (settings->BytesToUpload->IsNotEmpty || settings->UploadFileEnabled)
		{
			if (isBodyTypeText)
			{
				settings->Headers->Insert(L"Content-Type", L"text/plain; charset=UTF-8");
			}
			else
			{
				settings->Headers->Insert(L"Content-Type", L"application/octet-stream");
			}
		}
	}

	// Send the HTTP request.
	auto result = httpRequestOperation->Execute();
	if (result->HasSucceeded)
	{
		// The send has started successfully.
		// Push the request's unique integer ID to Lua as a return value.
		lua_pushlightuserdata(luaStatePointer, (void*)(httpRequestOperation->IntegerId));
	}
	else
	{
		// The send has failed. Push nil to Lua as a return value and log the error message.
		lua_pushnil(luaStatePointer);
		Logging::LoggingServices::Log(result->Message + L"\r\n");
	}

	// Returning 1 indicates that this Lua function is returning 1 value.
	return 1;
}

int NativeCoronaNetworkLibraryHandler::OnCancel(lua_State *luaStatePointer)
{
	// Validate.
	if (nullptr == luaStatePointer)
	{
		return 0;
	}

	// Fetch the library binding object.
	auto libraryHandlerPointer = (NativeCoronaNetworkLibraryHandler*)lua_touserdata(luaStatePointer, lua_upvalueindex(1));
	if (nullptr == libraryHandlerPointer)
	{
		return 0;
	}

	// Fetch the network request ID argument from Lua.
	if (lua_type(luaStatePointer, 1) != LUA_TLIGHTUSERDATA)
	{
		return 0;
	}
	int requestId = (int)lua_touserdata(luaStatePointer, 1);

	// Fetch the HTTP request operation assigned the given ID.
	CoronaHttpRequestOperation^ httpRequestOperation = nullptr;
	if (libraryHandlerPointer->fHttpRequestOperations->HasKey(requestId))
	{
		httpRequestOperation = libraryHandlerPointer->fHttpRequestOperations->Lookup(requestId);
	}
	if (nullptr == httpRequestOperation)
	{
		return 0;
	}

	// Abort the operation.
	// Note: This will cause OnHttpRequestEnded() to be invoked if the operation hasn't ended already.
	httpRequestOperation->Abort();
	return 0;
}

int NativeCoronaNetworkLibraryHandler::OnFinalizing(lua_State *luaStatePointer)
{
	// The Lua state is being destroyed. Delete this library binding's C++ object.
	void* libraryHandlerPointer = Rtt::Lua::CheckUserdata(luaStatePointer, 1, kLuaFinalizerMetatableName);
	if (libraryHandlerPointer)
	{
		Rtt_DELETE((NativeCoronaNetworkLibraryHandler*)libraryHandlerPointer);
	}
	return 0;
}

void NativeCoronaNetworkLibraryHandler::OnHttpRequestEnded(CoronaHttpRequestOperation ^operation, void *contextPointer)
{
	// Validate.
	if (!operation || !contextPointer)
	{
		return;
	}

	// Fetch the library binding pointer.
	auto libraryHandlerPointer = (NativeCoronaNetworkLibraryHandler*)contextPointer;

	// Fetch the runtime's Lua state.
	lua_State *luaStatePointer = nullptr;
	if (libraryHandlerPointer->fEnvironment->NativeRuntimePointer)
	{
		luaStatePointer = libraryHandlerPointer->fEnvironment->NativeRuntimePointer->VMContext().L();
	}

	// Delete the operation's Lua istener, if it has one.
	auto luaListener = operation->LuaListener;
	if (luaListener && luaStatePointer)
	{
		CoronaLuaDeleteRef(luaStatePointer, luaListener);
		operation->LuaListener = nullptr;
	}

	// Unsubscribe from this HTTP operation's events.
	operation->EndedEventHandler.Clear();

	// Remove the operation from the library's collection.
	// This will free the operation's request ID for future operations.
	if (libraryHandlerPointer->fHttpRequestOperations->HasKey(operation->IntegerId))
	{
		libraryHandlerPointer->fHttpRequestOperations->Remove(operation->IntegerId);
	}
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
