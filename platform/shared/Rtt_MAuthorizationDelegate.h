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
