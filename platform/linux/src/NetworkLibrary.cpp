//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Assert.h"
#include "Corona/CoronaLibrary.h"
#include "Corona/CoronaLua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "Rtt_PlatformNotifier.h"
#include "NetworkLibrary.h"
#include "NetworkSupport.h"

int luaload_network(lua_State *L);

// This corresponds to the name of the library, e.g. [Lua] require "plugin.library"
const char NetworkLibrary::kName[] = "plugin.network";

// This corresponds to the event name, e.g. [Lua] event.name
const char NetworkCompletionEvent::kName[] = "networkLibraryEvent";

int NetworkCompletionEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( VirtualEvent::Push( L ) ) )
	{
		// Future phases might relate to "fade"
//		lua_pushstring( L, "stopped" );
//		lua_setfield( L, -2, "phase" );
	}
	return 1;
}

// curl callback
static size_t curlWriteData(void *buffer, size_t size, size_t nmemb, void *arg)
{
	membuf* response = (membuf*) arg;
	response->append(buffer, size * nmemb);
	return size * nmemb;
}

void NetworkNotifierTask::operator()( Scheduler & sender )
{
	// sanity check
	if (fRequestParams != NULL && fRequestState != NULL)
	{
		int still_running = 0;
		long status = 0;
		CURLM* cm = fRequestParams->getMultiCURL();
		CURL* curl = fRequestParams->getCURL();
		if (cm && curl) // not cancelled ?
		{
			curl_multi_perform(cm, &still_running);
			if (still_running > 0)
			{
				return;
			}


			CURLcode rc = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

			// It is worth noting that browsers report a status of 0 in case of XMLHttpRequest errors too.
			if (status != 200)
			{
				UTF8String *message = new  UTF8String(curl_easy_strerror(rc));
				fRequestState->setError(message);
			}

			// Set responseHeaders
			fRequestState->setResponseHeaders(fRequestParams->fResponseHeaders.c_str());

			CoronaFileSpec *responseFile = fRequestParams->getResponseFile();
			if (responseFile != NULL)
			{
				if (status == 200)
				{
					fRequestState->fResponseBody.bodyType = TYPE_FILE;
					fRequestState->fResponseBody.bodyFile = new CoronaFileSpec(responseFile);
					UTF8String responseFileFullPath = responseFile->getFullPath();

					FILE* f = fopen(responseFileFullPath.c_str(), "wb");
					if (f)
					{
						fwrite(fRequestParams->fResponse.data(), 1, fRequestParams->fResponse.size(), f);
						fclose(f);
						fRequestState->setBytesEstimated(fRequestParams->fResponse.size());
						fRequestState->setBytesTransferred(fRequestParams->fResponse.size());
					}
				}
			}
			else if (fRequestParams->fResponse.size() > 0)
			{
				fRequestState->fResponseBody.bodyType = TYPE_BYTES;
				const uint8_t* buf = (const uint8_t*) fRequestParams->fResponse.data();
				fRequestState->fResponseBody.bodyBytes = new ByteVector(buf, buf + fRequestParams->fResponse.size());
				fRequestState->setBytesEstimated(fRequestParams->fResponse.size());
				fRequestState->setBytesTransferred(fRequestParams->fResponse.size());
			}

			fRequestState->setStatus(status);

			LuaCallback* func = fRequestParams->getLuaCallback();
			func->callWithNetworkRequestState(fRequestState);
		}

		// delete from queue
		fNetworkLibrary->fRequests.erase(fRequestParams->getID());

		// delete request
		fRequestParams = NULL;
		fRequestState = NULL;

		// delete task fron chain
		setKeepAlive(false);
	}
}

NetworkLibrary::NetworkLibrary()
{
	fSystemEventListener = LUA_REFNIL;
}

NetworkLibrary::~NetworkLibrary()
{
}

// CoronaRuntimeListener
void NetworkLibrary::onStarted(lua_State *L)
{
}

// CoronaRuntimeListener
void NetworkLibrary::onSuspended(lua_State *L)
{
}

// CoronaRuntimeListener
void NetworkLibrary::onResumed(lua_State *L)
{
}

// CoronaRuntimeListener
void NetworkLibrary::onExiting(lua_State *L)
{
	fSystemEventListener = NetworkLibrary::RemoveSystemEventListener(L, fSystemEventListener);
}

int NetworkLibrary::Open(lua_State *L)
{
	curl_version_info_data* vinfo = curl_version_info(CURLVERSION_NOW);
	if (vinfo->features && CURL_VERSION_SSL == 0)
	{
		Rtt_LogException("Network plugin: SSL not enabled\n");
	}

	// Register __gc callback
	const char kMetatableName[] = __FILE__; // Globally unique string to prevent collision
	CoronaLuaInitializeGCMetatable(L, kMetatableName, Finalizer);

	// Register the RequestCanceller Lua "class" (metatable)
	//
	//		RequestCanceller::registerClassWithLuaState( L );

	// Functions in library
	const luaL_Reg kVTable[] =
	{
		{"request_native", request},
		{"cancel", cancel},
		{"getConnectionStatus", getConnectionStatus},

		{NULL, NULL}
	};

	// Set library as upvalue for each library function
	Self *library = new Self;

	library->fSystemEventListener = NetworkLibrary::AddSystemEventListener(L, library);

	// Store the library singleton in the registry so it persists
	// using kMetatableName as the unique key.
	CoronaLuaPushUserdata(L, library, kMetatableName);
	lua_pushstring(L, kMetatableName);
	lua_settable(L, LUA_REGISTRYINDEX);

	// Leave "library" on top of stack
	// Set library as upvalue for each library function
	lua_CFunction factory = Corona::Lua::Open < Rtt::luaload_network > ;
	return CoronaLibraryNewWithFactory(L, factory, kVTable, library);
}

int
NetworkLibrary::Finalizer(lua_State *L)
{
	Self *library = (Self *)CoronaLuaToUserdata(L, 1);
	delete library;
	return 0;
}

NetworkLibrary* NetworkLibrary::ToLibrary(lua_State *L)
{
	// library is pushed as part of the closure
	Self *library = (Self *)lua_touserdata(L, lua_upvalueindex(1));
	return library;
}

int	NetworkLibrary::ValueForKey(lua_State *L)
{
	int result = 0;

	const char *key = lua_tostring(L, 2);
	if (key)
	{
		// if ( 0 == strcmp( key, "propertyName" ) )
	}

	return result;
}

// static [Lua] network.request( )
int	NetworkLibrary::request(lua_State *L)
{
	Self* thiz = NetworkLibrary::ToLibrary(L);
	return thiz->sendRequest(L);
}

size_t curlHeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata) {
	size_t totalSize = size * nitems;

	UTF8String *headers = static_cast<UTF8String *>(userdata);
	headers->append(buffer, totalSize);

	return totalSize;
}

int	NetworkLibrary::sendRequest(lua_State *L)
{
	smart_ptr<NetworkRequestParameters> requestParams = new NetworkRequestParameters(L);
	if (requestParams->isValid())
	{
		const std::string& url = requestParams->getRequestUrl();

		CURLM* curlMulti = curl_multi_init();
		CURL* curl = curl_easy_init();
		curl_multi_add_handle(curlMulti, curl);

		requestParams->setCURL(curlMulti, curl);
		// const std::string& method = requestParams->getRequestMethod();

		CURLcode rc;
		const Body* body = requestParams->getRequestBody();
		std::string fileSize = "";
		switch (body->bodyType)
		{
			case TYPE_STRING:
			{
				Rtt_ASSERT(body->bodyString);
				UTF8String postParams = *body->bodyString;
				const char* buf = (const char*)body->bodyString->c_str();
				rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);
				break;
			}
			case TYPE_BYTES:
			{
				Rtt_ASSERT(body->bodyBytes);
				unsigned char* buf = &body->bodyBytes->operator[](0);
				long buflen = body->bodyBytes->size();
				rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);
				rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, buflen);
				break;
			}
			case TYPE_FILE:
			{
				Rtt_ASSERT(body->bodyFile);
				FILE *uploadFile = fopen(body->bodyFile->getFullPath().c_str(), "rb");
				if (uploadFile == NULL) {
					break;
				}
				// Record the file handle, to be cleaned up together with curl.
				requestParams->setFileHandle(uploadFile);
				rc = curl_easy_setopt(curl, CURLOPT_READDATA, uploadFile);
				// Get and set the file size.
				long size = fseek(uploadFile, 0, SEEK_END) ? -1 : ftell(uploadFile);
				fseek(uploadFile, 0, SEEK_SET);
				rc = curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)size);
				fileSize = "Content-Length: " + std::to_string(size);
				// Set the POST request.
				rc = curl_easy_setopt(curl, CURLOPT_POST, 1L);
				break;
			}
			case TYPE_NONE:
				break;

			default:
				Rtt_ASSERT(0 && "todo");
				break;
		}

		StringMap* hd = requestParams->getRequestHeaders();
		if (hd && hd->size() > 0)
		{
			struct curl_slist* headers = NULL; // init to NULL is important
			StringMap::iterator iter;
			for (iter = hd->begin(); iter != hd->end(); iter++)
			{
				UTF8String key = (*iter).first;
				UTF8String value = (*iter).second;
				UTF8String requestHeaders = key + ": " + value;
				headers = curl_slist_append(headers, requestHeaders.c_str());
			}
			// Set the size of the file to be uploaded.
			if (!fileSize.empty()) {
				curl_slist_append(headers, fileSize.c_str());
			}
			rc = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		} else {
			// Set the size of the file to be uploaded.
			if (!fileSize.empty()) {
				rc = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, fileSize.c_str()));
			}
		}

		rc = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// tell libcurl to follow redirection
		rc = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		rc = curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

		// Set the callback function for response headers.
		rc = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curlHeaderCallback);
		rc = curl_easy_setopt(curl, CURLOPT_HEADERDATA, &requestParams->fResponseHeaders);

		rc = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &requestParams->fResponse);
		rc = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteData);
		rc = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		rc = curl_easy_setopt(curl, CURLOPT_TIMEOUT, requestParams->getTimeout());

		long status = 0;
		smart_ptr<NetworkRequestState> requestState = new NetworkRequestState();
		requestState->fResponseBody.bodyType = TYPE_NONE;
		requestState->fResponseBody.bodyBytes = NULL;
		requestState->setURL(requestParams->getRequestUrl());
		requestState->setStatus(status);
		requestState->setPhase("ended");
		requestState->setBytesEstimated(requestParams->fResponse.size());
		requestState->setBytesTransferred(requestParams->fResponse.size());
		int still_running = 0;
		CURLM* cm = requestParams->getMultiCURL();
		curl_multi_perform(cm, &still_running);

		// onEnterFrame
		Runtime* runtime = LuaContext::GetRuntime(L);
		NetworkNotifierTask* task = Rtt_NEW(runtime->Allocator(), NetworkNotifierTask(requestParams, requestState, this));
		runtime->GetScheduler().Append( task );

		fRequests[requestParams->getID()] = requestParams;
		lua_pushnumber(L, requestParams->getID());

		return 1; // pushed values
	}
	return 0;
}

// [Lua] network.cancel( )
int	NetworkLibrary::cancel(lua_State *L)
{
	Self* thiz = NetworkLibrary::ToLibrary(L);
	bool rc = false;
	unsigned int requestID = lua_tonumber(L, 1);
	auto it = thiz->fRequests.find(requestID);
	if (it != thiz->fRequests.end())
	{
		NetworkRequestParameters* requestParams = it->second;
		rc = true;
		requestParams->cancel();
		thiz->fRequests.erase(it);
	}
	lua_pushboolean(L, rc);
	return 1;
}

// [Lua] network.getConnectionStatus( )
int NetworkLibrary::getConnectionStatus(lua_State *L)
{
	//		DWORD connectionFlags = WinInetConnectivity::getConnectedState();
	bool isMobile = false; // fixme, (connectionFlags & INET_CONNECTION_MODEM) != 0;
	bool isConnected = true; // fixme, isMobile || ((connectionFlags & INET_CONNECTION_LAN) != 0);

	lua_createtable(L, 0, 2);
	int luaTableStackIndex = lua_gettop(L);

	lua_pushboolean(L, isConnected);
	lua_setfield(L, luaTableStackIndex, "isConnected");

	lua_pushboolean(L, isMobile);
	lua_setfield(L, luaTableStackIndex, "isMobile");
	return 1;
}

// This static method receives "system" event messages from Corona, at which point it determines the instance
// that registered the listener and dispatches the system events to that instance.
//
int NetworkLibrary::ProcessSystemEvent(lua_State *luaState)
{
	void *ud = lua_touserdata(luaState, lua_upvalueindex(1));
	NetworkLibrary *networkLibrary = (NetworkLibrary *)ud;

	lua_getfield(luaState, 1, "type");
	const char *eventType = lua_tostring(luaState, -1);
	lua_pop(luaState, 1);

	if (0 == strcmp("applicationStart", eventType))
	{
		networkLibrary->onStarted(luaState);
	}
	else if (0 == strcmp("applicationSuspend", eventType))
	{
		networkLibrary->onSuspended(luaState);
	}
	else if (0 == strcmp("applicationResume", eventType))
	{
		networkLibrary->onResumed(luaState);
	}
	else if (0 == strcmp("applicationExit", eventType))
	{
		networkLibrary->onExiting(luaState);
	}
	return 0;
}

int NetworkLibrary::AddSystemEventListener(lua_State *L, NetworkLibrary *networkLibrary)
{
	int ref = LUA_REFNIL;

	// Does the equivalent of the following Lua code:
	//   Runtime:addEventListener( "system", ProcessSystemEvent )
	// which is equivalent to:
	//   local f = Runtime.addEventListener
	//   f( Runtime, "system", ProcessSystemEvent )
	CoronaLuaPushRuntime(L); // push 'Runtime'

	if (lua_type(L, -1) == LUA_TTABLE)
	{
		lua_getfield(L, -1, "addEventListener"); // push 'f', i.e. Runtime.addEventListener
		lua_insert(L, -2); // swap so 'f' is below 'Runtime'
		lua_pushstring(L, "system");

		// Push ProcessSystemEvent as closure so it has access to 'networkLibrary'
		lua_pushlightuserdata(L, networkLibrary); // Assumes 'networkLibrary' lives for lifetime of plugin
		lua_pushcclosure(L, &NetworkLibrary::ProcessSystemEvent, 1);

		// Register reference to C closure so we can use later when we need to remove the listener
		ref = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		// Lua stack order (from lowest index to highest):
		// f
		// Runtime
		// "system"
		// ProcessSystemEvent (closure)
		CoronaLuaDoCall(L, 3, 0);
	}
	else
	{
		lua_pop(L, 1); // pop nil
	}
	return ref;
}

int NetworkLibrary::RemoveSystemEventListener(lua_State *L, int systemEventListenerRef)
{
	// Does the equivalent of the following Lua code:
	//   Runtime:removeEventListener( "system", ProcessSystemEvent )
	// which is equivalent to:
	//   local f = Runtime.removeEventListener
	//   f( Runtime, "system", ProcessSystemEvent )
	CoronaLuaPushRuntime(L); // push 'Runtime'

	if (lua_type(L, -1) == LUA_TTABLE)
	{
		lua_getfield(L, -1, "removeEventListener"); // push 'f', i.e. Runtime.removeEventListener
		lua_insert(L, -2); // swap so 'f' is below 'Runtime'
		lua_pushstring(L, "system");

		// Push reference to the C closure that was used in "addEventListener"
		lua_rawgeti(L, LUA_REGISTRYINDEX, systemEventListenerRef);

		// Lua stack order (from lowest index to highest):
		// f
		// Runtime
		// "system"
		// ProcessSystemEvent (closure)
		CoronaLuaDoCall(L, 3, 0);

		luaL_unref(L, LUA_REGISTRYINDEX, systemEventListenerRef);
	}
	else
	{
		lua_pop(L, 1); // pop nil
	}
	return LUA_REFNIL;
}

// ----------------------------------------------------------------------------

}// namespace Corona

// ----------------------------------------------------------------------------

CORONA_EXPORT int luaopen_network(lua_State *L)
{
	return Rtt::NetworkLibrary::Open(L);
}
