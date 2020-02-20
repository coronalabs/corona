//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MAuthorizationDelegate_H__
#define _Rtt_MAuthorizationDelegate_H__

#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Authorization;
class AuthorizationTicket;

// ----------------------------------------------------------------------------

class MAuthorizationDelegate
{
	public:
		// Return true if user accepted EULA; false otherwise
		virtual bool ShowLicense( const Authorization& sender ) = 0;

		// Return true if user accepted EULA; false otherwise
		virtual bool ShowTrial( const Authorization& sender, const AuthorizationTicket& ticket ) = 0;

		virtual bool ShowDeviceBuildUpsell( const Authorization& sender, const AuthorizationTicket& ticket, TargetDevice::Platform platform ) = 0;

		virtual bool ShowTrialUpsell( const Authorization& sender, const AuthorizationTicket& ticket, const char *message ) = 0;

		virtual void ShowRenewal( const Authorization& sender, const AuthorizationTicket& ticket ) = 0;

		// When ticket is not installed, it corresponds to first launch
		// Return true if caller should reattempt to find ticket; false otherwise
		virtual bool TicketNotInstalled( const Authorization& sender ) = 0;

		// Shows dialog asking user for password or NULL if user cancelled
		// Caller must verify if password is correct
		virtual const char* PasswordInvalid( const Authorization& sender, const char *usr ) = 0;

		// Performs platform specific verification to occur before the authorization object
		// does its default verification on the ticket.
		// Returns true if verification has passed. Returns false if failed, which prevents the authorization
		// object from performing its default verification.
		virtual bool PreVerification( const Authorization& sender ) const { return true; }

		// Ticket was invalid
		// virtual void TicketInvalid( const Authorization& sender ) const = 0;

		// Return true if caller should reattempt to authorize with server; false otherwise
		virtual bool AuthorizationDidFail( const Authorization& sender ) const = 0;

		// Return true if caller should reattempt to verify ticket; false otherwise
		virtual bool VerificationDidFail( const Authorization& sender ) const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MAuthorizationDelegate_H__
