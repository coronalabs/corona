//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ConsoleAuthorizationDelegate_H__
#define _Rtt_ConsoleAuthorizationDelegate_H__

#include "Rtt_MAuthorizationDelegate.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Authorization;
class AuthorizationTicket;

// ----------------------------------------------------------------------------

class ConsoleAuthorizationDelegate : public MAuthorizationDelegate
{
	public:
		ConsoleAuthorizationDelegate();
		virtual ~ConsoleAuthorizationDelegate();

	public:
		void SetLoginCredentials( const char *usr, const char *pwd ) { fUsr = usr; fPwd = pwd; }

	public:
		bool IsPreviouslyAuthorized() const { return fIsPreviouslyAuthorized; }
		bool HasLoginCredentials() const { return fUsr && fPwd; }

	public:
		bool Login( Authorization *authorizer ) const;
		int Authorize( Authorization *authorizer, const char *user, const char *encryptedPassword ) const;

	public:
		virtual bool ShowLicense( const Authorization& sender );
		virtual bool ShowTrial( const Authorization& sender, const AuthorizationTicket& ticket );
		virtual bool ShowDeviceBuildUpsell( const Authorization& sender, const AuthorizationTicket& ticket, TargetDevice::Platform platform );
		virtual bool ShowTrialUpsell( const Authorization& sender, const AuthorizationTicket& ticket, const char *message );
		virtual void ShowRenewal( const Authorization& sender, const AuthorizationTicket& ticket );
		virtual bool TicketNotInstalled( const Authorization& sender );
		virtual const char* PasswordInvalid( const Authorization& sender, const char *usr );
		virtual bool AuthorizationDidFail( const Authorization& sender ) const;
		virtual bool VerificationDidFail( const Authorization& sender ) const;

	private:
		const char *fUsr;
		const char *fPwd;
		bool fIsPreviouslyAuthorized;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_ConsoleAuthorizationDelegate_H__
