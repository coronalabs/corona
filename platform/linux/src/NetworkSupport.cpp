//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>

#include "Corona/CoronaLog.h"
#include "NetworkSupport.h"
#include "Core/Rtt_Assert.h"

#ifdef _WIN32
#define strncasecmp _strnicmp
#define strcasecmp stricmp
#define strtok_r strtok_s
#define snprintf _snprintf
#define strdup _strdup
#else
#define strcmpi strcasecmp
#endif

bool startsWith( const char * haystack, char * needle )
{
	if (!needle || !haystack)
		return false;

	size_t lenHaystack = strlen(haystack);
	size_t lenNeedle = strlen(needle);
	if (lenNeedle > lenHaystack)
		return false;

	return strncasecmp( haystack, needle, lenNeedle ) == 0;
}

bool endsWith( const char * haystack, char * needle )
{
	if (!needle || !haystack)
		return false;

	size_t lenHaystack = strlen(haystack);
	size_t lenNeedle = strlen(needle);
	if (lenNeedle >  lenHaystack)
		return false;

	return strncasecmp( haystack + lenHaystack - lenNeedle, needle, lenNeedle) == 0;
}


char *trimWhitespace( char * str )
{
	// Trim leading space
	while(isspace(*str))
	{
		str++;
	}

	if (*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	char *end = str + strlen(str) - 1;
	while(end > str && isspace(*end))
	{
		end--;
	}

	// Write new null terminator
	*(end + 1) = 0;

	return str;
}

char * getContentTypeEncoding( const char *contentTypeHeader )
{
	char *ct = getContentType(contentTypeHeader);
	if (ct)
	{
//		printf("Parsed Content-Type: %s", ct);
	}
	delete [] ct;

	char *charset = NULL;
	if ( NULL != contentTypeHeader )
	{
		const char* charsetPrefix = "charset=";
		char *tokens = strdup(contentTypeHeader);

		char *nextToken = NULL;
		char *value = strtok_r(tokens, ";", &nextToken);
		while (value)
		{
			value = trimWhitespace(value);
			if ( startsWith(value, (char *)charsetPrefix) )
			{
				if ( strlen(value) > strlen(charsetPrefix) )
				{
					charset = strdup(value + strlen(charsetPrefix));
//					printf("Explicit charset was found in content type, was: %s", charset);
				}
				value = NULL;
			}
			else
			{
				value = strtok_r(NULL, ";", &nextToken);
			}
		}

		free(tokens);
	}

	// Caller will be responsible for freeing this...
	return charset;
}


ProgressDirection getProgressDirectionFromString( const char *progressString )
{
	if (strcmpi( "upload", progressString ) == 0)
		return Upload;
	else if (strcmpi( "download", progressString ) == 0)
		return Download;
	else if (strcmpi( "none", progressString ) == 0)
		return None;
	else
		return UNKNOWN;
}

const char* getProgressDirectionName( ProgressDirection progressDirection )
{
	if (progressDirection == Upload)
		return "Upload";
	else if (progressDirection == Download)
		return "Download";
	else if (progressDirection == None)
		return "None";
	else
		return "UNKONWN";
}

void paramValidationFailure( lua_State *luaState, const char *message, ... )
{
	// For now we're just going to log this.  We take a lua_State in case we decide at some point that
	// we want to do more (like maybe throw a Lua exception).
	//
	const char *where = "";
	if ( luaState == NULL )
	{
		// Include the location of the call from the Lua context
		luaL_where( luaState, 2 );
		{
			where = lua_tostring( luaState, -1 );
		}
		lua_pop( luaState, 1 );
	}

	if (where == NULL)
	{
		where = "";
	}

	va_list args;
	va_start(args, message);

	fprintf(stderr, "ERROR: network: %sinvalid parameter: ", where);
	vfprintf(stderr, message, args);
	fprintf(stderr, "\n");

	va_end(args);
	return;
}

// Parse the content type from a Content-Type header
//
char * getContentType( const char *contentTypeHeader )
{
	char *contentType = NULL;
	if ( NULL != contentTypeHeader )
	{
		char * tempContentType = strdup(contentTypeHeader);
		char * paramStart = strchr(tempContentType, ';');
		if (0 != paramStart)
		{
			*paramStart = 0;
		}

		char * trimmedContentType = trimWhitespace(tempContentType);

		// Since trimming might change the starting pointer, we need to make another copy of
		// the string to pass back, and delete the original copy.
		contentType = strdup(trimmedContentType);
		free(tempContentType);
	}

	// Caller will be responsible for freeing this...
	return contentType;
}


bool isudatatype(lua_State *L, int idx, const char *name)
{
	// returns true if a userdata is of a certain type
	if ( LUA_TUSERDATA != lua_type( L, idx ) )
		return 0;

	lua_getmetatable( L, idx );
	luaL_newmetatable ( L, name );
	int res = lua_equal( L, -2, -1 );
	lua_pop( L, 2 ); // pop both tables (metatables) off
	return ( 0 != res );
}

void NetworkRequestState::setBytesTransferred( long long nBytesTransferred )
{
	fBytesTransferred = nBytesTransferred;
}

NetworkRequestState::NetworkRequestState()
	: fStatus(-1)
	, fIsError(false)
	, fResponseType("text")
	, fBytesEstimated(0)
	, fBytesTransferred(0)
	, fRequestID(0)
{
}

NetworkRequestState::~NetworkRequestState( )
{
	// Clean up response body...
	//
	switch (fResponseBody.bodyType)
	{
		case TYPE_STRING:
		{
			delete fResponseBody.bodyString;
			fResponseBody.bodyString = NULL;
			fResponseBody.bodyType = TYPE_NONE;
		}
		break;

		case TYPE_BYTES:
		{
			delete fResponseBody.bodyBytes;
			fResponseBody.bodyBytes = NULL;
			fResponseBody.bodyType = TYPE_NONE;
		}
		break;

		case TYPE_FILE:
		{
			delete fResponseBody.bodyFile;
			fResponseBody.bodyFile = NULL;
			fResponseBody.bodyType = TYPE_NONE;
		}
		break;
	}
}

void NetworkRequestState::setError( UTF8String *message )
{
	fIsError = true;

	if (message)
	{
		fResponseBody.bodyType = TYPE_STRING;
		fResponseBody.bodyString = message;
	}
}

void NetworkRequestState::setResponseHeaders( const char *headers )
{
	// This is the raw header body (all headers, separated by CRLF, double CRLF at the end).
	// This first line will typically be the status line.
	//
	const char *headerDelims = "\r\n";
	char *_headers = strdup(headers);
	char *nextHeader = NULL;
	char *header = strtok_r(_headers, headerDelims, &nextHeader);
	while (header)
	{
		char *value = NULL;
		char *key = strtok_r(header, ":", &value);

		if (NULL != value)
		{
			value = trimWhitespace(value);
			if (0 == *value)
			{
				value = NULL;
			}
		}

		if (value == NULL)
		{
			value = key;
			key = (char *)"HTTP-STATUS-LINE";
		}

		// We have to concatenate multiple Set-Cookie headers because of the
		// data structure used to save the headers
		if (strcmp(key, "Set-Cookie") == 0 && ! fResponseHeaders[key].empty())
		{
			// Separate multiple cookies by a single comma (no space)
			fResponseHeaders[key] += ",";
			fResponseHeaders[key] += value;
		}
		else
		{
			fResponseHeaders[key] = value;
		}

		header = strtok_r(NULL, headerDelims, &nextHeader);
	}

	free(_headers);
}

const char* NetworkRequestState::getPhase( )
{
	return fPhase.c_str();
}

void NetworkRequestState::setPhase( const char *phase )
{
	fPhase = phase;
}

void NetworkRequestState::setBytesEstimated( long long nBytesEstimated )
{
	fBytesEstimated = nBytesEstimated;
}

int NetworkRequestState::pushToLuaState( lua_State *luaState )
{
	int luaTableStackIndex = lua_gettop( luaState );
	int nPushed = 0;

	lua_pushboolean( luaState, fIsError );
	lua_setfield( luaState, luaTableStackIndex, "isError" );
	nPushed++;

	lua_pushstring( luaState, fPhase.c_str() );
	lua_setfield( luaState, luaTableStackIndex, "phase" );
	nPushed++;

	// responseHeaders must be always in network event
	{
		lua_createtable( luaState, 0, fResponseHeaders.size() );
		int luaHeaderTableStackIndex = lua_gettop( luaState );

		StringMap::iterator iter;
		for (iter = fResponseHeaders.begin(); iter != fResponseHeaders.end(); iter++)
		{
			UTF8String key = (*iter).first;
			UTF8String value = (*iter).second;

			lua_pushstring( luaState, value.c_str() );
			lua_setfield( luaState, luaHeaderTableStackIndex, key.c_str() );
		}

		lua_setfield( luaState, luaTableStackIndex, "responseHeaders" );
		nPushed++;
	}

	if ( ( TYPE_NONE != fResponseBody.bodyType ) && ( fPhase == "ended" ) )
	{
		lua_pushstring( luaState, fResponseType.c_str() );
		lua_setfield( luaState, luaTableStackIndex, "responseType" );
		nPushed++;

		switch (fResponseBody.bodyType)
		{
			case TYPE_STRING:
			{
				lua_pushstring( luaState, fResponseBody.bodyString->c_str() );
			}
			break;

			case TYPE_BYTES:
			{
				// If we don't have any response bytes, we'll write an empty string...
				//
				if (fResponseBody.bodyBytes->size() > 0)
				{
					lua_pushlstring( luaState, (const char *)&fResponseBody.bodyBytes->front(), fResponseBody.bodyBytes->size() );
				}
				else
				{
					lua_pushstring( luaState, "" );
				}
			}
			break;

			case TYPE_FILE:
			{
				lua_createtable( luaState, 0, 3 );
				int luaResponseTableStackIndex = lua_gettop( luaState );

				lua_pushstring( luaState, fResponseBody.bodyFile->getFilename().c_str() );
				lua_setfield( luaState, luaResponseTableStackIndex, "filename" );

				lua_pushlightuserdata( luaState, fResponseBody.bodyFile->getBaseDirectory() );
				lua_setfield( luaState, luaResponseTableStackIndex, "baseDirectory" );

				lua_pushstring( luaState, fResponseBody.bodyFile->getFullPath().c_str() );
				lua_setfield( luaState, luaResponseTableStackIndex, "fullPath" );
			}
			break;
		}

		lua_setfield( luaState, luaTableStackIndex, "response" );
		nPushed++;
	}

	lua_pushinteger( luaState, fStatus );
	lua_setfield( luaState, luaTableStackIndex, "status" );
	nPushed++;

	lua_pushstring( luaState, fRequestURL.c_str() );
	lua_setfield( luaState, luaTableStackIndex, "url" );
	nPushed++;

	lua_pushnumber( luaState, (lua_Number)fBytesTransferred );
	lua_setfield( luaState, luaTableStackIndex, "bytesTransferred" );
	nPushed++;

	lua_pushnumber( luaState, (lua_Number)fBytesEstimated );
	lua_setfield( luaState, luaTableStackIndex, "bytesEstimated" );
	nPushed++;

	if ( fDebugValues.size() > 0 )
	{
		lua_createtable( luaState, 0, fDebugValues.size() );
		int luaDebugTableStackIndex = lua_gettop( luaState );

		StringMap::iterator iter;
		for (iter = fDebugValues.begin(); iter != fDebugValues.end(); iter++)
		{
			UTF8String key = (*iter).first;
			UTF8String value = (*iter).second;

			lua_pushstring( luaState, value.c_str() );
			lua_setfield( luaState, luaDebugTableStackIndex, key.c_str() );
		}

		lua_setfield( luaState, luaTableStackIndex, "debug" );
		nPushed++;
	}

	return nPushed;
}

// --------------------------------------------------------------------------------------
// LuaCallback
// --------------------------------------------------------------------------------------

LuaCallback::LuaCallback( lua_State* luaState, CoronaLuaRef luaReference )
{

	//Get the main thread state, in case we are on a
	lua_State *coronaState = luaState;

	lua_State *mainState = CoronaLuaGetCoronaThread(luaState);
	if (NULL != mainState )
	{
		coronaState = mainState;
	}


	fLuaState = coronaState;

	fLuaReference = luaReference;

	fMinNotificationIntervalMs = 1000;
	fLastNotificationTime = 0;
}

LuaCallback::~LuaCallback()
{
	if ( NULL != fLuaReference )
	{
		// ??? We can't just unreference it here, because there is no guarantee we are on the Lua thread. ???
		unregister();
	}
}

bool LuaCallback::callWithNetworkRequestState( NetworkRequestState *networkRequestState )
{
	if ( NULL == fLuaReference )
	{
//		printf("Attempt to post call to callback after it was unregistered");
		return false;
	}

	// We call the callback conditionally based on the following:
	//

	// Rule 1: We don't send notifications if the request has been cancelled.
	//
	//   Note: In practice, the request cancel is immediate and we never see this case,
	//         but we'll leave this in just in case it is possible with specific timing...
	//
//	if ( networkRequestState->getRequestCanceller()->isCancelled() )
//	{
//		printf("Attempt to post call to callback after cancelling, ignoring");
//		return false; // We did not post the callback
//	}

	// Rule 2: We don't send multiple notifications of the same type (phase) within a certain
	//         interval, in order to avoid overrunning the listener.
	//
	uint64_t currentTime = fLastNotificationTime + fMinNotificationIntervalMs; // fixme Rtt::get_ticks();
	if (networkRequestState->getPhase() == fLastNotificationPhase &&  (currentTime < fLastNotificationTime + fMinNotificationIntervalMs))
	{
		printf("Attempt to post call to callback for phase \"%s\" within notification interval, ignoring", networkRequestState->getPhase());
		return false; // We did not post the callback
	}
	else
	{
		fLastNotificationPhase = networkRequestState->getPhase();
		fLastNotificationTime = currentTime;
	}

	CoronaLuaNewEvent( fLuaState, "networkRequest" );
	networkRequestState->pushToLuaState( fLuaState );
	CoronaLuaDispatchEvent( fLuaState, fLuaReference, 0 );

	return true;
}

void LuaCallback::unregister()
{
	CoronaLuaDeleteRef( fLuaState, fLuaReference );
	fLuaReference = NULL;
}

// --------------------------------------------------------------------------------------
// NetworkRequestParameters
// --------------------------------------------------------------------------------------

NetworkRequestParameters::NetworkRequestParameters( lua_State *luaState)
{
	static unsigned int sID = 0;
	fID = sID++;

	fL = luaState;
	fIsValid = false;

	bool isInvalid = false;

	fProgressDirection = None;
	fIsBodyTypeText = true;
	fTimeout = 30;
	fIsDebug = false;
	fHandleRedirects = true;
	fRequestBody.bodyType = TYPE_NONE;
	fRequestBodySize = 0;
	fResponseFile = NULL;
	fLuaCallback = NULL;
	fCURL = NULL;
	fMultiCURL = NULL;

	int arg = 1;
	// First argument - url (required)
	//
	if ( LUA_TSTRING == lua_type( luaState, arg ) )
	{
		const char * requestUrl = lua_tostring( luaState, arg );
		fRequestUrl = requestUrl;
	}
	else
	{
		paramValidationFailure( luaState, "First argument to network.request() should be a URL string" );
		isInvalid = true;
	}

	++arg;

	// Second argument - method (required)
	//
	if (!isInvalid)
	{
		if ( LUA_TSTRING == lua_type( luaState, arg ) )
		{
			const char * method = lua_tostring( luaState, arg );

			// This is validated in the Lua class
			fMethod = method;

			++arg;
		}
		else
		{
			fMethod = "GET";
		}
	}

	// Third argument - listener (optional)
	//
	if (!isInvalid)
	{
		if ( CoronaLuaIsListener( luaState, arg, "networkRequest" ) )
		{
			CoronaLuaRef ref = CoronaLuaNewRef( luaState, arg );
			fLuaCallback = new LuaCallback( luaState, ref );

			++arg;
		}
	}

	// Fourth argument - params table (optional)
	//
	int paramsTableStackIndex = arg;

	if (!isInvalid && !lua_isnoneornil( luaState, arg ))
	{
		if ( LUA_TTABLE == lua_type( luaState, arg ) )
		{
			bool wasRequestContentTypePresent = false;

			lua_getfield( luaState, arg, "headers" );
			if (!lua_isnil( luaState, -1 ))
			{
				if ( LUA_TTABLE == lua_type( luaState, -1 ) )
				{
					for (lua_pushnil(luaState); lua_next(luaState, -2); lua_pop(luaState, 1))
					{
						// Fetch the table entry's string key.
						// An index of -2 accesses the key that was pushed into the Lua stack by luaState.next() up above.
						const char *keyName = lua_tostring(luaState, -2);
						if (keyName == NULL)
						{
							// A valid key was not found. Skip this table entry.
							continue;
						}

						if (strcmpi("Content-Length", keyName) == 0)
						{
							// You just don't worry your pretty little head about the Content-Length, we'll handle that...
							continue;
						}

						UTF8String value = UTF8String();

						// Fetch the table entry's value in string form.
						// An index of -1 accesses the entry's value that was pushed into the Lua stack by luaState.next() above.
						switch (lua_type(luaState, -1))
						{
							case LUA_TSTRING:
							{
								const char *stringValue = lua_tostring(luaState, -1);
								value.append( stringValue );
							}
							break;

							case LUA_TNUMBER:
							{
								double numericValue = lua_tonumber(luaState, -1);
								char numberBuf[32];
								if (  floor( numericValue ) == numericValue )
								{
									if ( snprintf(numberBuf, sizeof(numberBuf), "%li", (long int)numericValue) > 0 )
									{
										value.append( numberBuf  );
									}
								}
								else
								{
									if ( snprintf(numberBuf, sizeof(numberBuf), "%f", numericValue) > 0 )
									{
										value.append( numberBuf  );
									}
								}
							}
							break;

							case LUA_TBOOLEAN:
							{
								bool booleanValue = (lua_toboolean(luaState, -1) != 0);
								value.append( booleanValue ? "true" : "false" );
							}
							break;
						}

						if (!value.empty())
						{
//							printf("Header - %s: %s", keyName, value.c_str());

							if ( strcmpi( "Content-Type", keyName ) == 0 )
							{
//								printf("Processing Content-Type request header");
								wasRequestContentTypePresent = true;

								char * ctCharset = getContentTypeEncoding( value.c_str() );
								if ( NULL != ctCharset )
								{
//vv									if ( ! CharsetTranscoder::isSupportedEncoding( ctCharset ) )
//									{
//										paramValidationFailure( luaState, "'header' value for Content-Type header contained an unsupported character encoding: %s", ctCharset );
//										isInvalid = true;
//									}

									free(ctCharset);
								}
							}

							fRequestHeaders[keyName] = value;
						}
					}
				}
				else
				{
					paramValidationFailure( luaState, "'headers' value of params table, if provided, should be a table (got %s)", lua_typename(luaState, lua_type(luaState, -1)) );
					isInvalid = true;
				}
			}
			lua_pop( luaState, 1);

			//If this is a POST request and the user hasn't filled in the content-type
			//we make an assumption (to preserve existing functionality)
			if 	(fRequestHeaders.find("Content-Type") == fRequestHeaders.end() &&
			        fMethod.compare("POST") == 0 &&
			        !wasRequestContentTypePresent)
			{
				fRequestHeaders["Content-Type"] = "application/x-www-form-urlencoded; charset=UTF-8";
				wasRequestContentTypePresent = true;
			}

			fIsBodyTypeText = true;
			lua_getfield( luaState, paramsTableStackIndex, "bodyType" );
			if (!lua_isnil( luaState, -1 ))
			{
				if ( LUA_TSTRING == lua_type( luaState, -1 ) )
				{
					// If we got something, make sure it's a string
					const char *bodyTypeValue = lua_tostring( luaState, -1 );

					if ( strcmpi( "text", bodyTypeValue) == 0 )
					{
						fIsBodyTypeText = true;
					}
					else if ( strcmpi( "binary", bodyTypeValue) == 0 )
					{
						fIsBodyTypeText = false;
					}
					else
					{
						paramValidationFailure( luaState, "'bodyType' value of params table was invalid, must be either \"text\" or \"binary\", but was: \"%s\"", bodyTypeValue );
						isInvalid = true;
					}
				}
				else
				{
					paramValidationFailure( luaState, "'bodyType' value of params table, if provided, should be a string value (got %s)", lua_typename(luaState, lua_type(luaState, -1)) );
					isInvalid = true;
				}
			}
			lua_pop( luaState, 1 );

			lua_getfield( luaState, paramsTableStackIndex, "body" );
			if (!lua_isnil( luaState, -1 ))
			{
				// This can be either a Lua string containing the body, or a table with filename/baseDirectory that points to a body file.
				// If it's a string, it can either be "text" (char[]) or "binary" (byte[]), based on bodyType (above).
				//
				switch (lua_type(luaState, -1))
				{
					case LUA_TSTRING:
					{
						if (fIsBodyTypeText)
						{
//							printf("Request body from String (text)");
							const char* requestValue = lua_tostring( luaState, -1);
							fRequestBody.bodyType = TYPE_STRING;
							fRequestBody.bodyString = new UTF8String( requestValue );
							fRequestBodySize = fRequestBody.bodyString->size();

							if (!wasRequestContentTypePresent)
							{
								fRequestHeaders["Content-Type"] = "text/plain; charset=UTF-8";
								wasRequestContentTypePresent = true;
							}
						}
						else
						{
//							printf("Request body from String (binary)");
							size_t dataSize;
							const char* requestValue = lua_tolstring( luaState, -1, &dataSize );
							fRequestBody.bodyType = TYPE_BYTES;
							fRequestBody.bodyBytes = new ByteVector( dataSize );
							memcpy(&fRequestBody.bodyBytes->at(0), requestValue, dataSize);
							fRequestBodySize = dataSize;

							if (!wasRequestContentTypePresent)
							{
								fRequestHeaders["Content-Type"] = "application/octet-stream";
								wasRequestContentTypePresent = true;
							}
						}
					}
					break;

					case LUA_TTABLE:
					{
						// Body type for body from file is always binary
						//
						fIsBodyTypeText = false;

						// Extract filename/baseDirectory
						//
						lua_getfield( luaState, -1, "filename" ); // required

						if ( LUA_TSTRING == lua_type( luaState, -1 ) )
						{
							const char *filename = lua_tostring( luaState, -1 );
							lua_pop( luaState, 1 );

							void *baseDirectory = NULL;
							lua_getfield( luaState, -1, "baseDirectory"); // optional
							if (!lua_isnoneornil( luaState, 1 ))
							{
								baseDirectory = lua_touserdata( luaState, -1 );
							}
							lua_pop( luaState, 1 );

							// Prepare and call Lua function
							int	numParams = 1;
							lua_getglobal( luaState, "_network_pathForFile" );
							lua_pushstring( luaState, filename );  // Push argument #1
							if ( baseDirectory )
							{
								lua_pushlightuserdata( luaState, baseDirectory ); // Push argument #2
								numParams++;
							}

							Corona::Lua::DoCall( luaState, numParams, 2); // 1/2 arguments, 2 returns

							bool isResourceFile = ( 0 != lua_toboolean( luaState, -1 ) );
							const char *path = lua_tostring( luaState, -2 );
							lua_pop( luaState, 2 ); // Pop results

//							printf("body pathForFile from LUA: %s, isResourceFile: %s", path, isResourceFile ? "true" : "false");

							fRequestBody.bodyType = TYPE_FILE;
							fRequestBody.bodyFile = new CoronaFileSpec(filename, baseDirectory, path, isResourceFile);

							// Determine file size
							//
							Rtt_ASSERT(0 && "todo");
							//struct _stat64 buf;
							//if (_stati64(fRequestBody.bodyFile->getFullPath().c_str(), &buf) == 0)
							//	{
							//		fRequestBodySize = buf.st_size;
							//		printf("Size of body file is: %li", fRequestBodySize);
							//	}
						}
						else
						{
							paramValidationFailure( luaState, "body 'filename' value is required and must be a string value" );
							isInvalid = true;
						}
					}
					break;

					default:
					{
						paramValidationFailure( luaState, "Either body string or table specifying body file is required if 'body' is specified" );
						isInvalid = true;
					}
					break;
				}

				if ( ( TYPE_NONE != fRequestBody.bodyType ) && !wasRequestContentTypePresent )
				{
					paramValidationFailure( luaState, "Request Content-Type header is required when request 'body' is specified" );
					isInvalid = true;
				}
			}
			lua_pop( luaState, 1 );

			lua_getfield( luaState, paramsTableStackIndex, "progress" );
			if (!lua_isnil( luaState, -1 ))
			{
				if ( LUA_TSTRING == lua_type( luaState, -1 ) )
				{
					const char *progress = lua_tostring( luaState, -1 );

					fProgressDirection = getProgressDirectionFromString( progress );
					if ( UNKNOWN == fProgressDirection )
					{
						paramValidationFailure( luaState, "'progress' value of params table was invalid, if provided, must be either \"upload\" or \"download\", but was: \"%s\"", progress );
						isInvalid = true;
					}

//					printf("Progress: %s", getProgressDirectionName(fProgressDirection) );
				}
				else
				{
					paramValidationFailure( luaState, "'progress' value of params table, if provided, should be a string value (got %s)", lua_typename(luaState, lua_type(luaState, -1)) );
					isInvalid = true;
				}
			}
			lua_pop( luaState, 1 );

			lua_getfield( luaState, paramsTableStackIndex, "response" );
			if (!lua_isnil( luaState, -1 ))
			{
				if ( LUA_TTABLE == lua_type( luaState, -1 ) )
				{
					// Extract filename/baseDirectory
					//
					lua_getfield( luaState, -1, "filename" ); // required

					if ( LUA_TSTRING == lua_type( luaState, -1 ) )
					{
						const char *filename = lua_tostring( luaState, -1 );
						lua_pop( luaState, 1 );

						void *baseDirectory = NULL;
						lua_getfield( luaState, -1, "baseDirectory"); // optional
						if (!lua_isnoneornil( luaState, 1 ))
						{
							baseDirectory = lua_touserdata( luaState, -1 );
						}
						lua_pop( luaState, 1 );

						// Prepare and call Lua function
						int	numParams = 1;
						lua_getglobal( luaState, "_network_pathForFile" );
						lua_pushstring( luaState, filename );  // Push argument #1
						if ( baseDirectory )
						{
							lua_pushlightuserdata( luaState, baseDirectory ); // Push argument #2
							numParams++;
						}

						Corona::Lua::DoCall( luaState, numParams, 2 ); // 1/2 arguments, 2 returns

						bool isResourceFile = ( 0 != lua_toboolean( luaState, -1 ) );
						const char *path = lua_tostring( luaState, -2 );
						lua_pop( luaState, 2 ); // Pop results

//						printf("response pathForFile from LUA: %s, isResourceFile: %s", path, isResourceFile ? "true" : "false");

						fResponseFile = new CoronaFileSpec(filename, baseDirectory, path, isResourceFile);
					}
					else
					{
						paramValidationFailure( luaState, "response 'filename' value is required and must be a string value (got %s)", lua_typename(luaState, lua_type(luaState, -1)) );
						isInvalid = true;
					}
				}
				else
				{
					paramValidationFailure( luaState, "'response' value of params table, if provided, should be a table specifying response location values (got %s)", lua_typename(luaState, lua_type(luaState, -1)) );
					isInvalid = true;
				}
			}
			lua_pop( luaState, 1 );

			lua_getfield( luaState, paramsTableStackIndex, "timeout" );
			if (!lua_isnil( luaState, -1 ))
			{
				if ( LUA_TNUMBER == lua_type( luaState, -1 ) )
				{
					fTimeout = (int)lua_tonumber( luaState, -1 );
//					printf("Request timeout provided, was: %i", fTimeout);
				}
				else
				{
					paramValidationFailure( luaState, "'timeout' value of params table, if provided, should be a numeric value (got %s)", lua_typename(luaState, lua_type(luaState, -1)) );
					isInvalid = true;
				}
			}
			lua_pop( luaState, 1 );

			fIsDebug = false;
			lua_getfield( luaState, paramsTableStackIndex, "debug" );
			if (!lua_isnil( luaState, -1 ))
			{
				if ( LUA_TBOOLEAN == lua_type( luaState, -1 ) )
				{
					fIsDebug = ( 0 != lua_toboolean( luaState, -1 ) );
				}
			}
			lua_pop( luaState, 1 );

			fHandleRedirects = true;
			lua_getfield( luaState, paramsTableStackIndex, "handleRedirects" );
			if (!lua_isnil( luaState, -1 ))
			{
				if ( LUA_TBOOLEAN == lua_type( luaState, -1 ) )
				{
					fHandleRedirects = ( 0 != lua_toboolean( luaState, -1 ) );
				}
			}
			lua_pop( luaState, 1 );
		}
		else
		{
			paramValidationFailure( luaState, "Fourth argument to network.request(), if provided, should be a params table (got %s)", lua_typename(luaState, lua_type(luaState, -1)) );
			isInvalid = true;
		}
	}

	fIsValid = !isInvalid;
}

void NetworkRequestParameters::cancel()
{
	if (fCURL != 0 && fMultiCURL != NULL)
	{
		curl_multi_remove_handle(fMultiCURL, fCURL);
		curl_easy_cleanup(fCURL);
		curl_multi_cleanup(fMultiCURL);

		// to prevent multiple calls
		fMultiCURL = NULL;
		fCURL = NULL;
	}
}

NetworkRequestParameters::~NetworkRequestParameters()
{
	cancel();
	switch (fRequestBody.bodyType)
	{
		case TYPE_STRING:
		{
			delete fRequestBody.bodyString;
			fRequestBody.bodyString = NULL;
			fRequestBody.bodyType = TYPE_NONE;
		}
		break;

		case TYPE_BYTES:
		{
			delete fRequestBody.bodyBytes;
			fRequestBody.bodyBytes = NULL;
			fRequestBody.bodyType = TYPE_NONE;
		}
		break;

		case TYPE_FILE:
		{
			delete fRequestBody.bodyFile;
			fRequestBody.bodyFile = NULL;
			fRequestBody.bodyType = TYPE_NONE;
		}
		break;
	}

	if ( NULL != fLuaCallback )
	{
		delete fLuaCallback;
	}
	delete fResponseFile;
}

UTF8String NetworkRequestParameters::getRequestUrl( )
{
	return fRequestUrl;
}

UTF8String NetworkRequestParameters::getRequestMethod( )
{
	return fMethod;
}

ProgressDirection NetworkRequestParameters::getProgressDirection( )
{
	return fProgressDirection;
}

UTF8String NetworkRequestParameters::getRequestHeaderString( )
{
	UTF8String requestHeaders;

	StringMap::iterator iter;
	for (iter = fRequestHeaders.begin(); iter != fRequestHeaders.end(); iter++)
	{
		if (requestHeaders.size() > 0)
		{
			requestHeaders += "|"; // it was used '|' instead of "\r\n" because JS codepage differs from C++ codepage
		}

		UTF8String key = (*iter).first;
		UTF8String value = (*iter).second;
		requestHeaders += key + ": " + value;
	}
	return requestHeaders;
}

StringMap* NetworkRequestParameters::getRequestHeaders( )
{
	return &fRequestHeaders;
}

UTF8String* NetworkRequestParameters::getRequestHeaderValue( const char *headerKey )
{
	StringMap::iterator iter;
	for (iter = fRequestHeaders.begin(); iter != fRequestHeaders.end(); iter++)
	{
		UTF8String key = (*iter).first;
		if (strcmpi(key.c_str(), headerKey) == 0)
		{
			return &(*iter).second;
		}
	}

	return NULL;
}

Body* NetworkRequestParameters::getRequestBody( )
{
	return &fRequestBody;
}

long long NetworkRequestParameters::getRequestBodySize( )
{
	// For string request bodies, recompute to account for any encoding changes
	// between the original utf-8 version and any possibly re-encoded current
	// version...
	//
	if (TYPE_STRING == fRequestBody.bodyType)
	{
		fRequestBodySize = fRequestBody.bodyString->size();
	}
	return fRequestBodySize;
}

CoronaFileSpec* NetworkRequestParameters::getResponseFile( )
{
	return fResponseFile;
}

LuaCallback* NetworkRequestParameters::getLuaCallback( )
{
	return fLuaCallback;
}

//bool NetworkRequestParameters::isprintf( )
//{
//	return fIsDebug;
//}

bool NetworkRequestParameters::getHandleRedirects( )
{
	return fHandleRedirects;
}

int NetworkRequestParameters::getTimeout( )
{
	return fTimeout;
}

bool NetworkRequestParameters::isValid()
{
	return fIsValid;
}
