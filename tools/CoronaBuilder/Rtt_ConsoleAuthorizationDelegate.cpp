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

#include "Rtt_ConsoleAuthorizationDelegate.h"

#include "Rtt_Authorization.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_TargetDevice.h"

#include <stdio.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ConsoleAuthorizationDelegate::ConsoleAuthorizationDelegate()
:	fUsr( NULL ),
	fPwd( NULL ),
	fIsPreviouslyAuthorized( true )
{
}

ConsoleAuthorizationDelegate::~ConsoleAuthorizationDelegate()
{
}

bool
ConsoleAuthorizationDelegate::Login( Authorization *authorizer ) const
{
	const char *user = fUsr;
	const char *pass = fPwd;

	if ( ! user && ! pass )
	{
		return false;
	}
	
	Rtt_ASSERT( user );
	Rtt_ASSERT( pass );

	Rtt::String encryptedPassword;
	Rtt::Authorization::Encrypt( pass, &encryptedPassword );

	int code = Authorize( authorizer, user, encryptedPassword.GetString() );

	bool result = false;

	// Handle errors
	if ( Authorization::kAuthorizationReceivedTicket >= code )
	{
		const char *msg = "";

		switch( code )
		{
			case Authorization::kAuthorizationReceivedTicket:
				fprintf( stderr, "You have authorized this computer with the '%s' subscription.\n",
					AuthorizationTicket::DisplayStringForSubscription(
						authorizer->GetTicket()->GetSubscription() ) );
				result = true;
				break;
			case Authorization::kAuthorizationDeviceBlacklisted:
				msg = "This computer was already used for a trial.";
				break;
			case Authorization::kAuthorizationUnverifiedUserError:
				msg = "Your account has not been verified yet. A verification e-mail has been sent to you with further instructions on how to validate your account.";
				break;
			case Authorization::kAuthorizationTrialExpired:
				msg = "Your trial has expired. To continue using, contact Ansca to purchase a subscription with Ansca.";
				break;
			case Authorization::kAuthorizationBasicExpired:
				msg = "To use the latest updates, you must have an active subscription. Contact Ansca to renew now.";
				break;
			case Authorization::kAuthorizationQuotaLimitReached:
				msg = "You are already using this software on another computer. Please deauthorize the other computer before logging in on this computer.";
				break;
			case Authorization::kAuthorizationPlatformDoesNotMatch:
				msg = "You purchased a product for a different platform.";
				break;
			case Authorization::kAuthorizationDatabaseError:
				msg = "There were problems verifying your account subscription in our database.";
				break;
			case Authorization::kAuthorizationApiKeyInvalid:
				msg = "This version is no longer supported. Please download a supported version of this product.";
				break;
			case Authorization::kAuthorizationTokenExpired:
				msg = "Your computer's clock has the incorrect date and/or time. Please update with the correct time and try again.";
				break;
			default:
				Rtt_ASSERT_NOT_REACHED(); // If you hit this, then you are missing a case above
				break;
		}

		fprintf( stderr, "%s\n", msg );
	}
	else
	{
		const char *msg = "";

		if ( Authorization::kAuthorizationLoginFailed == code )
		{
			msg = "Incorrect e-mail and/or password.";
			authorizer->GetServices().SetPreference( user, NULL );
		}
		else if ( Authorization::kAuthorizationPassword == code )
		{
			msg = "Login succeeded, but we were unable to store your Corona password in your Keychain. You may have to login again in the future.";
		}
		else if ( Authorization::kAuthorizationAgreementError == code )
		{
			msg = "The Corona Connect terms and conditions have changed. Click OK to read and agree to the new terms.";
		}
		else
		{
			msg = "Could not verify account information.";
		}

		fprintf( stderr, "ERROR: %s\n", msg );
	}

	return result;
}

int
ConsoleAuthorizationDelegate::Authorize( Authorization *authorizer, const char *user, const char *encryptedPassword ) const
{
	int code = authorizer->Authorize( user, encryptedPassword, true );

	return code;
}

bool
ConsoleAuthorizationDelegate::ShowLicense( const Authorization& sender )
{
	fIsPreviouslyAuthorized = false;
	return true;
}

// Return true if user accepted EULA; false otherwise
bool
ConsoleAuthorizationDelegate::ShowTrial( const Authorization& sender, const AuthorizationTicket& ticket )
{
	return true;
}

bool
ConsoleAuthorizationDelegate::ShowDeviceBuildUpsell( const Authorization& sender, const AuthorizationTicket& ticket, TargetDevice::Platform platform )
{
	return true;
}

bool
ConsoleAuthorizationDelegate::ShowTrialUpsell( const Authorization& sender, const AuthorizationTicket& ticket, const char *message )
{
	return true;
}

void
ConsoleAuthorizationDelegate::ShowRenewal( const Authorization& sender, const AuthorizationTicket& ticket )
{
}

// When ticket is not installed, it corresponds to first launch
// Return true if caller should reattempt to find ticket; false otherwise
bool
ConsoleAuthorizationDelegate::TicketNotInstalled( const Authorization& sender )
{
	bool result = false;

	if ( sender.GetServices().IsInternetAvailable() )
	{
		result = Login( & const_cast< Authorization& >( sender ) );
	}

	return result;
}

// Shows dialog asking user for password or NULL if user cancelled
// Caller must verify if password is correct
const char*
ConsoleAuthorizationDelegate::PasswordInvalid( const Authorization& sender, const char *usr )
{
	return NULL;
}

// Ticket was invalid
// void TicketInvalid( const Authorization& sender ) const;

// Return true if caller should reattempt to authorize with server; false otherwise
bool
ConsoleAuthorizationDelegate::AuthorizationDidFail( const Authorization& sender ) const
{
	return false;
}

// Return true if caller should reattempt to verify ticket; false otherwise
bool
ConsoleAuthorizationDelegate::VerificationDidFail( const Authorization& sender ) const
{
	bool result = false;

	const AuthorizationTicket *ticket = sender.GetTicket();

	if ( ticket )
	{
		const char *windowTitle = NULL;
		const char *title = NULL;

		if ( ticket->IsTrial() )
		{
			windowTitle = "Corona Trial";
			title = "Trial users are not allowed to use this product. Please contact Ansca to purchase ";
		}
		else
		{
			windowTitle = "Corona Subscription Expired";
			title = "Your subscription has expired. Please contact Ansca to renew ";
		}

		const char *subscription = AuthorizationTicket::StringForSubscription( ticket->GetSubscription() );
		fprintf( stderr, "%s: %s '%s'.\n", windowTitle, title, subscription );
	}

	if ( ! result )
	{
		sender.GetServices().Terminate();
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

