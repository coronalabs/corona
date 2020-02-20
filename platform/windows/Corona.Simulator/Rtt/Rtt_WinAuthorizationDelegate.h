//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_WinAuthorizationDelegate_H__
#define _Rtt_WinAuthorizationDelegate_H__

#include "Rtt_MAuthorizationDelegate.h"
#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

class CLoginDlg;

namespace Rtt
{

// ----------------------------------------------------------------------------

class WinAuthorizationDelegate : public MAuthorizationDelegate
{
	public:
		WinAuthorizationDelegate();
		~WinAuthorizationDelegate();

	protected:
		int Authorize( Authorization *authorizer, const char *user, const char *pass, void *parent ) const;

	public:
		bool Reauthorize( Authorization *authorizer ) const;
		bool Login( Authorization *authorizer ) const;
		bool ShowReauthorizationRequiredWarning( const Authorization& sender ) const;

	public:
		virtual bool ShowLicense( const Authorization& sender );
		virtual bool ShowTrial( const Authorization& sender, const AuthorizationTicket& ticket );
		virtual bool ShowTrialUpsell( const Authorization& sender, const AuthorizationTicket& ticket, const char *message );
		virtual void ShowRenewal( const Authorization& sender, const AuthorizationTicket& ticket );
		virtual bool TicketNotInstalled( const Authorization& sender );

		virtual const char* PasswordInvalid( const Authorization& sender, const char *usr );

		virtual bool PreVerification( const Authorization& sender ) const;
		virtual bool AuthorizationDidFail( const Authorization& sender ) const;
		virtual bool VerificationDidFail( const Authorization& sender ) const;

		virtual bool ShowDeviceBuildUpsell( const Authorization& sender, const AuthorizationTicket& ticket, TargetDevice::Platform platform );
		virtual bool ShowTrialBuild( const Authorization& sender, const AuthorizationTicket& ticket );

	private:
		CLoginDlg *pLoginDlg;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WinAuthorizationDelegate_H__
