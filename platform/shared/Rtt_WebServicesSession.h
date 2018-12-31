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

#ifndef _Rtt_WebServicesSession_H__
#define _Rtt_WebServicesSession_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

class AppPackagerParams;

struct KeyValuePair;
class MPlatformServices;
class PlatformConnection;
class PlatformDictionaryWrapper;

// ----------------------------------------------------------------------------

class WebServicesSession
{
	public:
		// TODO: Move this to a separate file you lazy bum!
		enum
		{
			kServicesUrlType = 0,
			kBaseUrlType,

			kNumUrlTypes
		};
		static const char* CoronaServerUrl( const MPlatformServices& services, int urlType = kServicesUrlType );

#if !defined( Rtt_PROJECTOR )
	public:
		enum
		{
			kNoError = 0,
			kLoginError,			// = 1
			kApiKeyError,			// = 2
			kCriticalError,			// = 3
			kAgreementError,		// = 4
			kBuildError,			// = 5
			kBuildVersionMismatchError,	// = 6
			kExpiredError,			// = 7
			kUnverifiedUserError,	// = 8
			kTokenExpiredError,		// = 9
            kAlreadyLoggedInError,	// = 10
			kConnectionError,		// = 11
			kLocalPackagingError,	// = 12

			kNumErrors
		};

	public:
		// Performs call without requiring login
		static PlatformDictionaryWrapper* Call(
			const MPlatformServices& services,
			const char *url,
			const char *method,
			KeyValuePair *pairs,
			size_t numPairs );

	public:
		WebServicesSession( const MPlatformServices& services );

	protected:
		WebServicesSession( const MPlatformServices& services, const char *url );

	public:
		~WebServicesSession();

	public:
		// Assumes you have already called Login()
		PlatformDictionaryWrapper* Call( const char* method, KeyValuePair* pairs, size_t numPairs );

	protected:
		void SetSessionId( const char *sessid );
		void SetUid( const char *uid );

	public:
		int LoginWithEncryptedPassword( const char* url, const char* username, const char* encryptedPassword );
		void Logout();
		void CloseConnection();

		int BeginBuild( AppPackagerParams * params, const char* inputFile, const char* outputFile );

        const char *ErrorMessage() { return fErrorMessage; }

	private:
		const MPlatformServices& fServices;
		PlatformConnection* fConnection;
		char* fSessionId;
		S64 fSessionServerTimestamp;
		S64 fSessionClientTimestamp;
		S32 fUid;
		char *fErrorMessage;
#endif // Rtt_PROJECTOR
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WebServicesSession_H__
