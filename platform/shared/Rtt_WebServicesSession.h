//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
