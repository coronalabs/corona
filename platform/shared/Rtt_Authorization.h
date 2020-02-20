//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Authorization_H__
#define _Rtt_Authorization_H__

#include <time.h>
#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Authorization;
class AuthorizationTicket;
class MAuthorizationDelegate;
class MPlatformServices;

// ----------------------------------------------------------------------------

class Authorization
{
	public:
		static const char kUrlHome[];
		static const char kUrlPurchase[];
		static const char kUrlUpgradeToPro[];
		static const char kUrlRenew[];
		static const char kUrlRegister[];
		static const char kUrlDownload[];
		static const char kUrlAgreement[];

	public:
		static const char kTicketKey[];
		static const char kSuppressFeedbackKey[];
		static const char kVersionKey[];
		static const char kUsernameKey[];
		static const char kRenewalReminderKey[];

	public:
		Authorization( const MPlatformServices& services, MAuthorizationDelegate& delegate );
		virtual ~Authorization();

	public:
		bool Initialize(bool initiateLogin = true) const;
		void ClearTicket() const;

	protected:
		bool InitializeTicket() const;
		bool LoadTicket() const;
		AuthorizationTicket* CreateTicketFrom(const char *encryptedTicketData) const;

	public:
		// Encrypts using public key
		static void Encrypt( const char *plaintext, String * result );

	public:
//		void SetDelegate( const MAuthorizationDelegate* delegate ) { fDelegate = delegate; }
		//const MAuthorizationDelegate& GetDelegate() const { return fDelegate; }

	public:
		int Reauthorize() const;

		enum AuthorizationResultCode
		{
			// Following are returned by the authorize web api
			kAuthorizationDeviceBlacklisted = -1,
			kAuthorizationTrialExpired = -2,
			kAuthorizationBasicExpired = -3,
			kAuthorizationQuotaLimitReached = -4,
			kAuthorizationPlatformDoesNotMatch = -5,
			kAuthorizationDatabaseError = -6,
			kAuthorizationApiKeyInvalid = -7,
			kAuthorizationDeauthorizeSuccess = -8,
			kAuthorizationDeauthorizeTryLater = -9,
			kAuthorizationUnverifiedUserError = -10,
			kAuthorizationTokenExpired = -11,
			kAuthorizationConnectionError = -12,

			// Following are client-side only.
			kAuthorizationReceivedTicket = 0, // NOTE: This ticket still needs to be verified
			kAuthorizationError,
			kAuthorizationLoginFailed,
			kAuthorizationPassword,
			kAuthorizationTrialOnly,
			kAuthorizationAgreementError,

			kNumAuthorizationResultCodes
		};
		int Authorize( const char *email, const char *encryptedPassword, bool storeLoginInfo, bool verifyTicket = true ) const;
		int Deauthorize( const char *email, const char *encryptedPassword, const char *ticket ) const;

	public:
		bool IsFirstRun() const;
		// bool VerifyLogin() const; // used for device builds
		bool VerifyTicket() const;
		void SetLoginMessage(const char *mesg);
		char *GetLoginMessage() const { return (fLoginMessage == NULL ? (char *)"" : fLoginMessage); }

	protected:
		bool VerifyTicketOnce() const;

	public:
		const AuthorizationTicket* GetTicket() const { return fTicket; }
		const MPlatformServices& GetServices() const { return fServices; }

	private:
		const MPlatformServices& fServices;
		MAuthorizationDelegate& fDelegate;
		mutable AuthorizationTicket *fTicket;
		time_t fEulaTimestamp;
		bool fUseLibraryPreferences;
		char *fLoginMessage;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Authorization_H__
