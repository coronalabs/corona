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

#include "Rtt_WebServicesSession.h"

#include "Core/Rtt_VersionTimestamp.h"
#include "Core/Rtt_Time.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_MPlatformServices.h"
//#include "Rtt_MacAppPackager.h"  unused
#include "Rtt_AndroidAppPackager.h"
#include "Rtt_WebAppPackager.h"
#include "Rtt_PlatformConnection.h"
#include "Rtt_PlatformDictionaryWrapper.h"
#include "Rtt_FileSystem.h"
#include "Rtt_Authorization.h"

#include "Rtt_DeviceBuildData.h"

#include "hmac_sha2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#if defined(Rtt_WIN_ENV) && !defined(Rtt_LINUX_ENV)
	#include "WinFile.h"  // memory mapped file
#elif defined(Rtt_LINUX_ENV)
// 
#else 
	#include <sys/fcntl.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <unistd.h>
#endif

#include <string>
#include <vector>

#define kDefaultNumBytes 128

// ----------------------------------------------------------------------------

namespace Rtt
{


/*
Modified version of http://www.adp-gmbh.ch/cpp/common/base64.html

Copyright (C) 2004-2008 René Nyffenegger

This source code is provided 'as-is', without any express or implied
warranty. In no event will the author be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this source code must not be misrepresented; you must not
claim that you wrote the original source code. If you use this source code
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original source code.

3. This notice may not be removed or altered from any source distribution.

René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/
	
static const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";


static std::string base64_encode(U8 const* buf, size_t bufLen)
{
	std::string ret;
	int i = 0;
	int j = 0;
	U8 char_array_3[3];
	U8 char_array_4[4];
	
	while (bufLen--)
	{
		char_array_3[i++] = *(buf++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			
			for(i = 0; (i <4) ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}
	
	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';
		
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
		
		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];
		
		while((i++ < 3))
			ret += '=';
	}
	
	return ret;
}

#ifdef NOT_USED
static inline bool is_base64(U8 c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::vector<U8> base64_decode(std::string const& encoded_string)
{
	size_t in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	U8 char_array_4[4], char_array_3[3];
	std::vector<U8> ret;
	ret.reserve(encoded_string.length());
	
	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
	{
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4)
        {
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);
			
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
			
			for (i = 0; (i < 3); i++)
				ret.push_back(char_array_3[i]);
			i = 0;
		}
	}
	
	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;
		
		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);
		
		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
		
		for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
	}
	
	return ret;
}
#endif // NOT_USED
	
// ----------------------------------------------------------------------------

#define Rtt_USE_TEST_SERVER 0

const char*
WebServicesSession::CoronaServerUrl( const MPlatformServices& services, int urlType )
{
	#if Rtt_USE_TEST_SERVER
		#warning USING TEST SERVER URL
		static const char kBaseUrl[] = "http://192.168.192.51";
		static const char kServicesUrl[] = "http://192.168.192.51/services/xmlrpc";
	#else
		static const char kBaseUrl[] = "%s://developer.coronalabs.com";
		static const char kServicesUrl[] = "%s://developer.coronalabs.com/services/xmlrpc";
	#endif

    static char result[BUFSIZ];
    const char *protocol = "https";

    String prefVal;
    services.GetPreference("httpWebServices", &prefVal);
    if (! prefVal.IsEmpty() && strtol(prefVal.GetString(), (char **)NULL, 10) > 0)
    {
        Rtt_LogException("WebServices: using HTTP protocol");
        protocol = "http";
    }

    snprintf(result, BUFSIZ, kServicesUrl, protocol);

	switch( urlType )
	{
		case kBaseUrlType:
            snprintf(result, BUFSIZ, kBaseUrl, protocol);
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------
#if !defined( Rtt_PROJECTOR )
// ----------------------------------------------------------------------------

WebServicesSession::WebServicesSession( const MPlatformServices& services )
:	fServices( services ),
	fConnection( NULL ),
	fSessionId( NULL ),
	fSessionServerTimestamp( 0 ),
	fSessionClientTimestamp( 0 ),
	fUid( 0 ),
    fErrorMessage( NULL )
{
}

WebServicesSession::WebServicesSession( const MPlatformServices& services, const char *url )
:	fServices( services ),
	fConnection( services.CreateConnection( url ) ),
	fSessionId( NULL ),
	fSessionServerTimestamp( 0 ),
	fSessionClientTimestamp( 0 ),
	fUid( 0 ),
    fErrorMessage( NULL )
{
}

WebServicesSession::~WebServicesSession()
{
	Logout();
	free(fErrorMessage);
}

static char
RandomChar()
{
	const int kRange = 52; // Upper case/lower case letters

	int i = rand() % kRange; // clamp i in range [0,51]
	char base;
	if ( i < 26 )
	{
		base = 'a';
	}
	else
	{
		base = 'A';
		i -= 26;
	}

	return base + i;
}

// WARNING: Allocation of s must be (length + 1) to accomodate NULL-termination.
static char*
GetNonce( char* s, int length )
{
	char* result = s;
	// for xmlrpc nonce
	// Is this going to be an overflow problem for us?
	unsigned int seed = (unsigned int)Rtt_AbsoluteToMilliseconds( Rtt_GetAbsoluteTime() );
	srand( seed );
	for ( int i = 0; i < length; i++, s++ )
	{
		*s = RandomChar();
	}
	result[length] = '\0';
	return result;
}

static char*
GetHexBytes( char* dst, int dstLen, const unsigned char* dataBuffer, size_t len )
{
	char* result = dst;
	for ( size_t i = 0, iMax = len; i < iMax; ++i )
	{
		dst += snprintf( dst, dstLen, "%02x", (unsigned int)dataBuffer[ i ] );
        dstLen -= 2;
	}

	return result;
}

// This is one version of this interface, see below for another
    
PlatformDictionaryWrapper*
WebServicesSession::Call( const char* method, KeyValuePair* params, size_t numParams )
{
#if !defined(Rtt_LINUX_ENV)
	Rtt_ASSERT( fSessionId );
	Rtt_ASSERT( fConnection );

	// This domain is what Drupal thinks the domain string that's used in formulating the web API call.
	// Both live and staging server are configured to have this set to be the same.
	const char *domain = "developer.anscamobile.com";
	unsigned char key[] = "8e3ed4869262703dedf5c55aea6632b4";

	// Calculate the current time on the server.
	// If server timestamp was not received, then fallback to local machine's current time.
	// A current timestamp is needed so that the server won't timeout the session.
	S64 currentServerTime = (S64)time(NULL);
	if (fSessionServerTimestamp > 0)
	{
		currentServerTime = (currentServerTime - fSessionClientTimestamp) + fSessionServerTimestamp;
	}
	char timestamp[12];
	int __n = snprintf( timestamp, 12, "%lld", currentServerTime );
	Rtt_ASSERT( (size_t)__n < sizeof( timestamp ) );
	Rtt_UNUSED( __n );

	char nonce[12]; GetNonce( nonce, 10 );
	char hash[128]; 
	int hashLen = snprintf( hash, 128, "%s;%s;%s;%s", timestamp, domain, nonce, method );
	// int hashLen = snprintf( hash, 128, "%s;%s;%s;user.login", timestamp, domain, nonce );

	const size_t kHmacLen = 32; Rtt_STATIC_ASSERT( sizeof( hash ) > ( kHmacLen * 2 ) );
	unsigned char hmac[kHmacLen + 1]; hmac[kHmacLen] = '\0';
	hmac_sha256( key, sizeof(key), (unsigned char*)hash, hashLen, hmac, kHmacLen );
	Rtt_ASSERT( '\0' == hmac[kHmacLen] );

	GetHexBytes( hash, 128, hmac, kHmacLen );

	const KeyValuePair requiredParams[] =
	{
		{ "hash", hash, kStringValueType },
		{ "domain_name", domain, kStringValueType },
		{ "domain_time_stamp", timestamp, kStringValueType },
		{ "nonce", nonce, kStringValueType },
		{ "sessid", fSessionId, kStringValueType }
	};

	const size_t numRequired = sizeof(requiredParams) / sizeof( requiredParams[0] );
	const size_t numArgs = numParams + numRequired;
	KeyValuePair* args = (KeyValuePair*)malloc( sizeof( KeyValuePair ) * numArgs );

	for ( size_t i = 0; i < numRequired; i++ )
	{
		args[i] = requiredParams[i];
	}

	for ( size_t i = 0, j = numRequired; i < numParams; i++, j++ )
	{
		Rtt_ASSERT( j < numArgs );
		args[j] = params[i];
	}

	PlatformDictionaryWrapper* response = NULL;

	if (fConnection != NULL)
	{
		response = fConnection->Call( method, args, (int) numArgs );

		free( args );

		if (response == NULL)
		{
			const char *errorMsg = NULL;
			if (fConnection->Error() != NULL)
			{
				errorMsg = fConnection->Error();
			}
			else
			{
				errorMsg = "empty response";
			}

			if (fConnection->Error() != NULL)
			{
				fErrorMessage = strdup(fConnection->Error());
			}
		}
	}

	return response;
#else
	return NULL;
#endif
}

// Static helper

PlatformDictionaryWrapper*
WebServicesSession::Call(
                         const MPlatformServices& services,
                         const char *url,
                         const char *method,
                         KeyValuePair *pairs,
                         size_t numPairs )
{
    WebServicesSession session( services, url );
    PlatformDictionaryWrapper* response = session.fConnection->Call( "system.connect", NULL, 0 );

    if ( response )
    {
        session.SetSessionId( response->ValueForKey( "sessid" ) );
        Rtt_DELETE( response );
        
        response = session.Call( method, pairs, numPairs );
        session.SetSessionId( NULL );
    }

    if (response == NULL)
    {
        // Because this is a static member we can't save the error info
		const char *errorMsg = NULL;
		if (session.fConnection->Error() != NULL)
		{
			errorMsg = session.fConnection->Error();
		}
		else
		{
			errorMsg = "empty response";
		}
    }

    return response;
}

void
WebServicesSession::SetSessionId( const char* sessid )
{
	if ( ! sessid || fSessionId )
	{
		free( fSessionId );
	}

	if ( sessid )
	{
		size_t numElements = 1 + strlen( sessid );
		fSessionId = (char*)malloc( numElements*sizeof(char) );
		memcpy( fSessionId, sessid, numElements*sizeof(char) );	
	}
	else
	{
		fSessionId = NULL;
	}
}

void
WebServicesSession::SetUid( const char *uid )
{
	if ( uid )
	{
		long value = atol( uid );
		Rtt_ASSERT( uid != 0 );

		fUid = (S32) value;
	}
}

int
WebServicesSession::LoginWithEncryptedPassword( const char* url, const char* username, const char* encryptedPassword )
{
#if !defined(Rtt_LINUX_ENV)
	// In case multiple logins are called, logout previous session
	Logout();

	PlatformConnection* connection = fServices.CreateConnection( url );

	fConnection = connection; // TODO: this is required to allow CloseConnection() to work - figure out why fConnection isn't just used everywhere (this is historically a real mess; see below)
	PlatformDictionaryWrapper* response = connection->Call("system.connect", NULL, 0);
	fConnection = NULL;

	int result = kConnectionError;

	if ( response )
	{
		// Store received session ID.
		SetSessionId( response->ValueForKey( "sessid" ) );
		Rtt_DELETE( response );
		
		// Fetch current time from server.
		// Will be used by the Call() function to timestamp subsequent server requests.
		response = connection->Call("corona.getTime", NULL, 0);
		fSessionServerTimestamp = 0;
		fSessionClientTimestamp = 0;
		if (response)
		{
			const char *stringValue = response->ValueForKey( "timestamp" );
			if (stringValue)
			{
				// Note: atoi() function returns zero if it fails to parse.
				fSessionServerTimestamp = ::atoi(stringValue);
				if (fSessionServerTimestamp > 0)
				{
					fSessionClientTimestamp = (S64)time(NULL);
				}
			}
			Rtt_DELETE( response );
		}

		// Get the timestamp of the current build.
		char timestamp[32];
		snprintf( timestamp, 32, "%x", (unsigned int) Rtt_VersionTimestamp() );

		//Rtt_TRACE( ( "WebServicesSession::Login() encrypted pwd: %s\n", encryptedPassword ) );

		KeyValuePair loginParams[] =
		{
			{ "username", username, kStringValueType },
			{ "encryptedPassword", encryptedPassword, kStringValueType },
			{ "timestamp", timestamp, kStringValueType }
		};

		result = kLoginError;
		fConnection = connection;
		response = Call( "user.login2", loginParams, sizeof( loginParams ) / sizeof( loginParams[0] ) );
		fConnection = NULL;

		if ( response )
		{
			const char* sessid = response->ValueForKey( "sessid" );
			if ( sessid != NULL )
			{
				SetSessionId( sessid );

				fConnection = connection;
				connection = NULL;

				result = kNoError;
			}

			SetUid( response->ValueForKeyPath( "user.uid" ) );
		}
		else
		{
			const char *e = connection->Error();
			if ( e )
			{
				if ( strcmp( "Invalid API key.", e ) == 0 )
				{
					result = kApiKeyError;
				}
				else if ( strcmp( "Token has expired.", e ) == 0 )
				{
					result = kTokenExpiredError;
				}
// Sonia added this for windows 
// If there is an error, try to force a logout, otherwise the server thinks we're logged in and won't let us back in.
				else if ( strncmp( "Already logged in as ", e, 21) == 0 )
				{
                    // Mostly an artifact of interrupting debugging sessions
                    // Logout() doesn't work because it tests for a valid SessionId
					result = kAlreadyLoggedInError;  
					fConnection = connection;
					PlatformDictionaryWrapper* logout_response = Call( "user.logout", NULL, 0 );
					Rtt_DELETE( logout_response );
					connection = NULL;
				}
			}

			if ( kLoginError == result )
			{
				Rtt_DELETE( response );

				// Check if they need to sign the latest agreement
				Rtt::String encryptedUsername;
				Authorization::Encrypt( username, &encryptedUsername );

				KeyValuePair params[] =
				{
					{ "timestamp", timestamp, kStringValueType },
					{ "encryptedUsername", encryptedUsername.GetString(), kStringValueType },
				};
				
				fConnection = connection;
				response = Call( "corona.agreement", params, sizeof( params ) / sizeof( params[0] ) );
				fConnection = NULL;
				
				if ( response )
				{
					const char *r = response->ValueForKey( "result" );
					if ( r )
					{
						if ( 0 == strcmp( "kCoronaDidNotAgree", r ) )
						{
							result = kAgreementError;
						}
						else if ( 0 == strcmp( "kCoronaNonexistentUser", r ) )
						{
							result = kLoginError;
						}
						else if ( 0 == strcmp( "kCoronaUnverifiedUser", r ) )
						{
							result = kUnverifiedUserError;
						}
					}
				}
			}

			SetSessionId( NULL );
			fSessionServerTimestamp = 0;
			fSessionClientTimestamp = 0;
		}
	}
	else
	{
		if (connection->Error() != NULL)
		{
			fErrorMessage = strdup(connection->Error());
		}
	}

	Rtt_DELETE( response );
	Rtt_DELETE( connection );
	return result;
#else
	return 0;
#endif
}

void
WebServicesSession::Logout()
{
	if ( fSessionId )
	{
		PlatformDictionaryWrapper* response = Call( "user.logout", NULL, 0 );
		Rtt_DELETE( response );

		SetSessionId( NULL );
	}
	fSessionServerTimestamp = 0;
	fSessionClientTimestamp = 0;

	Rtt_DELETE( fConnection );
	fConnection = NULL;
}

void
WebServicesSession::CloseConnection()
{
	if (fConnection != NULL)
	{
		fConnection->CloseConnection();
	}
}

int
WebServicesSession::BeginBuild( AppPackagerParams * params, const char* inputFile, const char* outputFile )
{
#if !defined(Rtt_LINUX_ENV)
	int result = kBuildError;

	const char *deviceId = fServices.Platform().GetDevice().GetUniqueIdentifier( MPlatformDevice::kDeviceIdentifier );
	if ( !deviceId )
	{
		String tmpString;

		tmpString.Set("Rtt::WebServicesSession::BeginBuild - Invalid device ID\n");

		Rtt_TRACE( ( "%s", tmpString.GetString() ) );
		params->SetBuildMessage(tmpString.GetString());

		return result;
	}

	// Base64 encode inputFile

#ifdef Rtt_WIN_ENV
	WinFile file;
    file.Open( inputFile );

	if ( file.IsOpen() )
	{
        size_t numBytes = file.GetFileSize();
        const void *p = file.GetContents();
#else
	int fd = open( inputFile, O_RDONLY );
	if ( Rtt_VERIFY( fd >= 0 ) )
	{
		struct stat statbuf;
		int statResult = fstat( fd, & statbuf ); Rtt_UNUSED( statResult ); Rtt_ASSERT( statResult >= 0 );

		size_t numBytes = statbuf.st_size;
		void* p = mmap( NULL, numBytes, PROT_READ, MAP_SHARED, fd, 0 );
#endif
		const void* src = p;

		if ( Rtt_VERIFY( src ) )
		{
            std::string str64 = base64_encode((U8 const*)src, numBytes);

			char timestamp[32];
			snprintf( timestamp, 32, "%x", (unsigned int) Rtt_VersionTimestamp() );
			const char* appName = params->GetAppName();
			const char * appPackage = params->GetAppPackage();
			intptr_t targetPlatform = params->GetTargetPlatform();
			intptr_t targetVersion = params->GetTargetVersion();
			intptr_t targetDevice = params->GetTargetDevice();

			String escapedAppName;
			if ( targetPlatform == TargetDevice::kAndroidPlatform ||
			     targetPlatform == TargetDevice::kKindlePlatform ||
			     targetPlatform == TargetDevice::kNookPlatform )
			{
				PlatformAppPackager::EscapeStringForAndroid( appName, escapedAppName );
				appName = escapedAppName.GetString();
			}
			else
			{
                // To avoid sending raw Unicode to the build server we'll escape the appname for iOS
                // the same way we do for Android.  Since iOS doesn't understand the \u Unicode escapes
                // used, this scheme only works out because iPhonePackageApp fixes the name later
                PlatformAppPackager::EscapeFileName( appName, escapedAppName );
                appName = escapedAppName.GetString();
			}

			const char * customBuildId = params->GetCustomBuildId();
			const char * productId = params->GetProductId();
			const char *platformName = fServices.Platform().GetDevice().GetPlatformName();
			const char *certType = params->GetCertType();
			const char *appVersion = params->GetVersion();
			bool isLaunchPad = false; // TODO: Remove server dependency then remove this

			DeviceBuildData& deviceBuildData = params->GetDeviceBuildData( fServices.Platform(), fServices );
			String json( & fServices.Platform().GetAllocator() );
			deviceBuildData.GetJSON( json );

			KeyValuePair callParams[] =
			{
				{ "deviceId", deviceId, kStringValueType },
				{ "timestamp", timestamp, kStringValueType },
				{ "appname", appName, kStringValueType },
				{ "file", str64.c_str(), kStringValueType },
				{ "device", (void*)targetDevice, kIntegerValueType },
				{ "platform", (void*)targetPlatform, kIntegerValueType },
				{ "version", (void*)targetVersion, kIntegerValueType },
				{ "appPackage", (void*)appPackage, kStringValueType },				
				{ "customBuildId", (void*)customBuildId, kStringValueType },				
				{ "productId", (void*)productId, kStringValueType },
				{ "clientPlatform", (void*)platformName, kStringValueType },
				{ "certType", (void*)certType, kStringValueType },
				{ "appVersion", (void*)appVersion, kStringValueType },
				{ "launchPad", (void*)isLaunchPad, kIntegerValueType },
				{ "json", (void*)json.GetString(), kStringValueType },
				{ "downloadResult", (void*)1, kIntegerValueType },
			};

			size_t kMaxNumParams = sizeof(callParams)/sizeof(callParams[0]);
			size_t numParams = kMaxNumParams;

			const char *buildMethod = "corona.build.7";

			String buildMethodValue;
			fServices.GetPreference( "CoronaBuildMethod", & buildMethodValue );
			if ( buildMethodValue.GetString() )
			{
				buildMethod = buildMethodValue.GetString();
				Rtt_TRACE_SIM( ( "CoronaBuildMethod overridden. Using: %s.\n", buildMethod ) );
			}

			String numParamsValue;
			fServices.GetPreference( "CoronaBuildMethodNumParams", & numParamsValue );
			if ( numParamsValue.GetString() )
			{
				size_t val = (size_t)strtoul( numParamsValue.GetString(), NULL, 10 );
				if ( val > numParams && val <= kMaxNumParams )
				{
					numParams = val;
					Rtt_TRACE_SIM( ( "CoronaBuildMethodNumParams overriden. Using: %ld.\n", numParams ) );
				}
			}

			// .2 includes android and customBuildId and productId
			PlatformDictionaryWrapper* response = Call( buildMethod, callParams, numParams );

			if ( Rtt_VERIFY( response ) )
			{
				const char* buildSessionId = response->ValueForKey( "result" );
				Rtt_TRACE_SIM( ( "BuildID: %s\n", buildSessionId ) );

				if (buildSessionId == NULL)
				{
					const char *message = response->ValueForKey( "faultString" );;
					Rtt_TRACE_SIM( ( "ERROR: An error occurred during build (%s). The server returned the following message:\n---\n%s\n---\n", buildMethod, message ) );
					params->SetBuildMessage( message );
				}
				else if ( NULL != strstr( buildSessionId, "kCoronaBuild [" ) )
				{
					const char *message = buildSessionId + strlen( "kCoronaBuild [" );
					Rtt_TRACE_SIM( ( "ERROR: An error occurred during build (%s). The server returned the following message:\n---\n%s\n---\n", buildMethod, message ) );
					params->SetBuildMessage( message );
				}
				else
				{
					KeyValuePair params2[] = { "buildsessionid", buildSessionId, kStringValueType };
					PlatformDictionaryWrapper* response2 = Call( "corona.getUrl", params2, sizeof(params2)/sizeof(params2[0]) );
					
					if ( response2 == NULL )
					{
						const char *message = "ERROR: Unexpected empty response from build server (corona.getUrl)";
						
						Rtt_TRACE_SIM( ( "%s", message ) );
						
						params->SetBuildMessage( message );
					}
					
					if ( Rtt_VERIFY( response2 ) )
					{
						const char* url = response2->ValueForKey( "result" );

						if (fConnection->Download(url, outputFile))
						{
							result = kNoError;
						}
						else
						{
							String tmpString;

							tmpString.Set("ERROR: failed to download build: ");
							tmpString.Append(url);

							if (fConnection->Error() != NULL)
							{
								tmpString.Append("\n\n");
								tmpString.Append(fConnection->Error());
							}

							Rtt_TRACE_SIM( ( "%s", tmpString.GetString() ) );

							params->SetBuildMessage( tmpString.GetString() );
						}

						Rtt_DELETE( response2 );
					}

					Rtt_DELETE( response );
				}
			}
            else
            {
                // A NULL response means a network failure (no internet, bad proxy, etc)
				String tmpString;

				tmpString.Set("ERROR: build failed\n\n");

				if (fConnection->Error() != NULL)
				{
					tmpString.Append(fConnection->Error());
				}
				else
				{
					// Invent a reason
					tmpString.Append("There was a problem completing the build on the server");
				}

				Rtt_TRACE_SIM( ( "%s", tmpString.GetString() ) );

				params->SetBuildMessage( tmpString.GetString() );
                result = kConnectionError;
            }
		}

#ifdef Rtt_WIN_ENV
        file.Close();
#else
		munmap( p, numBytes );
		close( fd );
#endif
	}
	else  // file handle was invalid
	{
#ifdef Rtt_WIN_ENV
		// these values can be used as parameters to the constructor to reduce init time
		// tableStart, hashCharCount, fFirstInterestingChar
				
		PCHAR pBuffer;    
        DWORD lError = GetLastError();
		FormatMessageA ( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    lError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPSTR)&pBuffer,
                    0,
                    NULL );

		String tmpString;
					
		tmpString.Set("Rtt::WebServicesSession::BeginBuild - Couldn't open input file\n\n");
		tmpString.Append(inputFile);
		tmpString.Append("\n");
		tmpString.Append(pBuffer);
		tmpString.Append("\n");

		Rtt_TRACE( ( tmpString.GetString() ) );
		params->SetBuildMessage(tmpString.GetString());

		LocalFree ( pBuffer );
#endif  // Rtt_WIN_ENV
	}

	return result;
#else

	// build output.zip
	
	// build3_output_android.sh Clock ~/template ~/tmp ~/developerkey.cert ~/input.zip little com.coronalabs.vitaly.Clock 00000000 _YES_
	char cmd[1024]; 
	const char* coronaAppID = "00000000";
	const char* addSplashScreen = "_YES_";

	const char* resourcesDir = params->GetSdkRoot();
	std::string	templateDir = resourcesDir;
	templateDir += "/template";
	
	// tmp dir for build3_output_android.sh
	char tmpDirTemplate[] = "/tmp/CoronaLabs/BBXXXXXX";
	const char *tmpDirName = Rtt_MakeTempDirectory(tmpDirTemplate);
	if (tmpDirName == NULL)
	{
		Rtt_TRACE(("Failed to create %s", tmpDirTemplate));
		return kBuildError;
	}
	
	// copy android template to tmp folder
	snprintf(cmd, sizeof(cmd), "cp -R %s %s/", templateDir.c_str(), tmpDirName );
	int rc = system(cmd);
	if (rc != 0)
	{
		Rtt_TRACE(("Failed to run %s", cmd));
		return kBuildError;
	}
	
	snprintf(cmd, sizeof(cmd), "cd %s && ./build3_output_android.sh \"%s\" %s/template %s %s/developerkey.cert %s little %s %s %s",
	resourcesDir, params->GetAppName(), tmpDirName, tmpDirName, resourcesDir, inputFile, params->GetAppPackage(), coronaAppID, addSplashScreen);
	
	Rtt_TRACE((cmd));
	rc = system(cmd);
	if (rc != 0)
	{
		Rtt_TRACE(("Failed to run %s", cmd));
		return kBuildError;
	}
	
	// check if output.zip exist
	std::string output = tmpDirName;
	output += "/output.zip";
	if (Rtt_FileExists(output.c_str()))
	{
		snprintf(cmd, sizeof(cmd), "cp %s %s", output.c_str(), outputFile);
		rc = system(cmd);
		if (rc != 0)
		{
			Rtt_TRACE(("Failed to run %s", cmd));
			return kBuildError;
		}
	}
	
	snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpDirName);
	rc = system(cmd);
	if (rc != 0)
	{
		Rtt_TRACE(("Failed to run %s", cmd));
		return kBuildError;
	}
	
	return 0;

#endif
}

// ----------------------------------------------------------------------------
#endif // Rtt_PROJECTOR
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

