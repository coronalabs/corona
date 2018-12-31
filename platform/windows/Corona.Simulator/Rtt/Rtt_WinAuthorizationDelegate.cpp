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

#include "stdafx.h"

#include "Core/Rtt_Build.h"

#include "Rtt_WinAuthorizationDelegate.h"

#include "Rtt_Authorization.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_WebServicesSession.h"
#include "Rtt_WinDevice.h"

#include "WinString.h"
#include "CoronaInterface.h"
#include "LicenseDlg.h"
#include "LoginDlg.h"
#include "DevPasswordDlg.h"
#include "MessageDlg.h"
#include "ProgressWnd.h"


// ----------------------------------------------------------------------------

typedef struct AuthorizationContext
{
	Rtt::Authorization *authorizer;
	Rtt::WinAuthorizationDelegate *delegate;
}
AuthorizationContext;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

WinAuthorizationDelegate::WinAuthorizationDelegate() :
    pLoginDlg( NULL )
{
    pLoginDlg = new CLoginDlg;
    Rtt_ASSERT( pLoginDlg );
}

WinAuthorizationDelegate::~WinAuthorizationDelegate()
{
    if( pLoginDlg )
        delete pLoginDlg;
}

int
WinAuthorizationDelegate::Authorize( Authorization *authorizer, const char *user, const char *pass, void *parent ) const
{
	int code = authorizer->Authorize( user, pass, true );

	return code;
}

bool
WinAuthorizationDelegate::Reauthorize( Authorization *authorizer ) const
{
	bool result = false;

    // Contact server w/ existing username & password (upgraded?)
    int code = authorizer->Reauthorize();
	const AuthorizationTicket *ticket = authorizer->GetTicket();

    // If that works, all done.
    if( Authorization::kAuthorizationReceivedTicket == code )
	{
		if ( ticket && ticket->IsTrial() )
		{
			code = Authorization::kAuthorizationTrialOnly;
			CMessageDlg messageDlg;
			messageDlg.SetText( IDS_STILLTRIAL );
			messageDlg.SetIconStyle( MB_ICONEXCLAMATION );
			messageDlg.DoModal();
		}
        return true;
	}

	// username & password didn't work, maybe password changed on server
	// Get current username
	if ( Rtt_VERIFY( ticket ) )
	{
		const MPlatformServices& services = authorizer->GetServices();

		const char* usr = ticket->GetUsername();
		Rtt::String encryptedPassword;
		
		if ( Rtt_VERIFY( usr ) )
		{
			WinString strUser, strPassword;
			strUser.SetUTF8( usr );
			CDevPasswordDlg passwordDlg;
			passwordDlg.SetUser( strUser.GetTCHAR() );
			if( passwordDlg.DoModal() == IDOK )
			{
                strPassword.SetTCHAR( passwordDlg.GetPassword() );
                const char *pwd = strPassword.GetUTF8();
				Rtt::Authorization::Encrypt( pwd, &encryptedPassword );
			}
		}

		if ( encryptedPassword.GetString() )
		{
			int code = Authorize( authorizer, usr, encryptedPassword.GetString(), NULL );

			if ( Authorization::kAuthorizationReceivedTicket == code )
			{
				ticket = authorizer->GetTicket();
				if ( ticket && ticket->IsTrial() )
				{
					code = Authorization::kAuthorizationTrialOnly;
				}
			}

			if ( Authorization::kAuthorizationReceivedTicket >= code )
			{
				const char *url = Authorization::kUrlPurchase;
				int idMsg = 0;
				int idButtonDefault = IDS_OK;
				int idButtonAlt = IDS_QUIT;
				int alertStyle = MB_ICONEXCLAMATION;

				switch( code )
				{
				case Authorization::kAuthorizationReceivedTicket:
					idMsg = IDS_THANKSPURCHASE;
					idButtonDefault = IDS_CONTINUE;
					idButtonAlt = 0;
					alertStyle = MB_ICONINFORMATION;
					url = NULL;
					break;
				case Authorization::kAuthorizationQuotaLimitReached:
					idMsg = IDS_QUOTALIMIT;
					idButtonAlt = 0;
					url = NULL;
					break;
				case Authorization::kAuthorizationUnverifiedUserError:
					idMsg = IDS_LOGINUNVERIFIED;
					idButtonDefault = IDS_OK;
					idButtonAlt = 0;
					url = NULL;
					break;
				case Authorization::kAuthorizationPlatformDoesNotMatch:
					idMsg = IDS_PLATFORMMISMATCH;
					idButtonDefault = IDS_PURCHASE;
					break;
				case Authorization::kAuthorizationApiKeyInvalid:
					idMsg = IDS_APIKEYINVALID;
					url = Authorization::kUrlDownload;
					idButtonDefault = IDS_DOWNLOAD;
					break;
				case Authorization::kAuthorizationTokenExpired:
					idMsg = IDS_TOKENEXPIRED;
					idButtonDefault = IDS_OK;
					idButtonAlt = 0;
					url = NULL;
					break;
				case Authorization::kAuthorizationConnectionError:
					idMsg = IDS_NO_INTERNET;
					idButtonDefault = IDS_OK;
					idButtonAlt = 0;
					url = NULL;
					break;
				default:
					Rtt_ASSERT_NOT_REACHED(); // If you hit this, then you are missing a case above
					idMsg = IDS_UNKNOWNERROR;
					idButtonDefault = IDS_OK;
					idButtonAlt = 0;
					url = NULL;
					break;
				}
				CMessageDlg messageDlg;
				messageDlg.SetText( idMsg );
				messageDlg.SetDefaultText( idButtonDefault );
				messageDlg.SetAltText( idButtonAlt );
				messageDlg.SetUrl( url );
				messageDlg.SetIconStyle( alertStyle );
				messageDlg.DoModal();
			}
			else
			{
				const char *url = NULL;
				int idMsg = 0;
				int idButtonDefault = IDS_OK;
				int idButtonAlt = IDS_CANCEL;
				int alertStyle = MB_ICONEXCLAMATION;

				if ( Authorization::kAuthorizationLoginFailed == code )
				{
					idMsg = IDS_LOGINFAILED;
					// TODO: Should we reset password?
					// authorizer->GetServices().SetSecurePreference( usr, NULL );
				}
				else if ( Authorization::kAuthorizationPassword == code )
				{
					idMsg = IDS_PWDNOTSAVED;
				}
				else if ( Authorization::kAuthorizationAgreementError == code )
				{
					idMsg = IDS_AGREEMENTCHANGED;
					url = Authorization::kUrlAgreement;
				}
				else if ( Authorization::kAuthorizationTrialOnly == code )
				{
                     idMsg = IDS_STILLTRIAL;
				}
				else
				{
					idMsg = IDS_LOGINERROR;
				}

				CMessageDlg messageDlg;
				messageDlg.SetText( idMsg );
				messageDlg.SetDefaultText( idButtonDefault );
				messageDlg.SetAltText( idButtonAlt );
				messageDlg.SetUrl( url );
				messageDlg.SetIconStyle( alertStyle );
				messageDlg.DoModal();
			}
		}
	}

	return result;
}

bool
WinAuthorizationDelegate::Login( Authorization *authorizer ) const
{
	Rtt::String encryptedPassword;
    WinString strUser, strPassword;
	int code;
	bool result = false;

	// Display a progress window and hourglass mouse cursor.
	CAutoProgressWnd progressWindow;
    CWaitCursor wait;

	// Attempt to login the user and authorize the machine.
    strUser.SetTCHAR( pLoginDlg->GetUser() );
    strPassword.SetTCHAR( pLoginDlg->GetPassword() );
	Rtt::Authorization::Encrypt( strPassword.GetUTF8(), &encryptedPassword );
	code = Authorize( authorizer, strUser.GetUTF8(), encryptedPassword.GetString(), pLoginDlg );
    if (Authorization::kAuthorizationReceivedTicket == code)
	{
        result = true;
	}

	// Close the progress window and restore the mouse cursor.
	progressWindow.Close();
	wait.Restore();

	// Handle the result of the login.
	if ( Authorization::kAuthorizationReceivedTicket >= code )
	{
		const char *url = Authorization::kUrlPurchase;
        int idMsg = 0;
		int idButtonDefault = 0;
		int idButtonAlt = IDS_QUIT;
		int alertStyle = MB_ICONEXCLAMATION;

		switch( code )
		{
			case Authorization::kAuthorizationReceivedTicket:
                idMsg = IDS_LOGINSUCCESSFUL;
                idButtonDefault = IDS_CONTINUE;
                idButtonAlt = 0;
                alertStyle = MB_ICONINFORMATION;
                url = NULL;
				break;
			case Authorization::kAuthorizationDeviceBlacklisted:
                idMsg = IDS_LOGINBLACKLISTED;
                idButtonDefault = IDS_PURCHASE;
				break;
			case Authorization::kAuthorizationUnverifiedUserError:
				idMsg = IDS_LOGINUNVERIFIED;
                idButtonDefault = IDS_OK;
                idButtonAlt = 0;
                url = NULL;
				break;
			case Authorization::kAuthorizationTrialExpired:
                idMsg = IDS_TRIALEXPIRED;
                idButtonDefault = IDS_PURCHASE;
				break;
			case Authorization::kAuthorizationBasicExpired:
                idMsg = IDS_BASICEXPIRED;
                idButtonDefault = IDS_RENEW;
				url = Authorization::kUrlRenew;
				break;
			case Authorization::kAuthorizationQuotaLimitReached:
                idMsg = IDS_QUOTALIMIT;
                idButtonDefault = IDS_OK;
                idButtonAlt = 0;
                url = NULL;
				break;
			case Authorization::kAuthorizationPlatformDoesNotMatch:
                idMsg = IDS_PLATFORMMISMATCH;
                idButtonDefault = IDS_PURCHASE;
				break;
			case Authorization::kAuthorizationApiKeyInvalid:
                idMsg = IDS_APIKEYINVALID;
				url = Authorization::kUrlDownload;
                idButtonDefault = IDS_DOWNLOAD;
				break;
			case Authorization::kAuthorizationTokenExpired:
                idMsg = IDS_TOKENEXPIRED;
                idButtonDefault = IDS_OK;
                idButtonAlt = 0;
                url = NULL;
				break;
			case Authorization::kAuthorizationConnectionError:
				idMsg = IDS_NO_INTERNET;
				idButtonDefault = IDS_OK;
				idButtonAlt = 0;
				url = NULL;
				break;
			default:
				Rtt_ASSERT_NOT_REACHED(); // If you hit this, then you are missing a case above
				idMsg = IDS_UNKNOWNERROR;
				idButtonDefault = IDS_OK;
                idButtonAlt = 0;
                url = NULL;
				break;
		}
        CMessageDlg messageDlg;
        messageDlg.SetText( idMsg );
        messageDlg.SetDefaultText( idButtonDefault );
        messageDlg.SetAltText( idButtonAlt );
        messageDlg.SetUrl( url );
        messageDlg.SetIconStyle( alertStyle );
        messageDlg.DoModal();
	}
	else
	{
		const char *url = NULL;
        int idMsg = 0;
		int idButtonDefault = IDS_OK;
		int idButtonAlt = IDS_CANCEL;
		int alertStyle = MB_ICONEXCLAMATION;

		if ( Authorization::kAuthorizationLoginFailed == code )
		{
            idMsg = IDS_LOGINFAILED;
			// TODO: Should we reset password?
			// authorizer->GetServices().SetSecurePreference( usr, NULL );
		}
		else if ( Authorization::kAuthorizationPassword == code )
		{
            idMsg = IDS_PWDNOTSAVED;
		}
		else if ( Authorization::kAuthorizationAgreementError == code )
		{
            idMsg = IDS_AGREEMENTCHANGED;
			url = Authorization::kUrlAgreement;
		}
		else
		{
            idMsg = IDS_LOGINERROR;
		}

        CMessageDlg messageDlg;
        messageDlg.SetText( idMsg );
        messageDlg.SetDefaultText( idButtonDefault );
        messageDlg.SetAltText( idButtonAlt );
        messageDlg.SetUrl( url );
        messageDlg.SetIconStyle( alertStyle );
        messageDlg.DoModal();
	}

	return result;
}

bool
WinAuthorizationDelegate::ShowReauthorizationRequiredWarning( const Authorization& sender ) const
{
	CString message;
	bool wasDeauthorized = false;

	// Inform the user that this machine needs to be reauthorized.
	message.LoadString(IDS_APP_NEEDS_REAUTHORIZATION);
	if (AfxMessageBox((LPCTSTR)message, MB_OKCANCEL | MB_ICONINFORMATION, 0) == IDOK)
	{
		// Deauthorize this machine.
		wasDeauthorized = appDeauthorize();
	}

	// Close this application.
	// User is not allowed to run this app until it has been started with a valid device ID.
	sender.GetServices().Terminate();

	// If this machine was successfully deauthorized, then relaunch this application
	// which forces the user to reauthorize this machine.
	if (wasDeauthorized)
	{
		CString applicationPath;
		GetModuleFileName(NULL, applicationPath.GetBuffer(512), 512);
		applicationPath.ReleaseBuffer();
		STARTUPINFO startupInfo;
		PROCESS_INFORMATION processInfo;
		memset(&processInfo, 0, sizeof(processInfo));
		memset(&startupInfo, 0, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		BOOL result = ::CreateProcess(
							(LPCTSTR)applicationPath, ::GetCommandLine(), NULL, NULL, FALSE,
							0, NULL, NULL, &startupInfo, &processInfo);
	}

	return wasDeauthorized;
}

bool
WinAuthorizationDelegate::ShowLicense( const Authorization& sender )
{
    bool result = false;

    CLicenseDlg dlgLicense;

    if( dlgLicense.DoModal() == IDOK )
        result = true;    

	return result;
}

bool
WinAuthorizationDelegate::ShowTrial( const Authorization& sender, const AuthorizationTicket& ticket )
{
	// Do not show a trial window for non-trial users.
	Rtt_ASSERT( ticket.IsTrial() );
	if (ticket.IsTrial() == false)
	{
		return true;
	}
	
	// Load trial warning message.
	static const time_t kNumSecondsPerDay = 24 * 60 * 60;
	time_t current = time(nullptr);
	time_t expiration = ticket.GetExpiration();
	int numDaysSinceTrialBegan = 30 + (int)((current - expiration) / kNumSecondsPerDay);
	int idMsg = IDS_TRIALFUN_d;
	CStringA message;
	message.Format(idMsg);
	
	// Display a trial window.
	return ShowTrialUpsell(sender, ticket, message);
}

bool
WinAuthorizationDelegate::ShowTrialUpsell( const Authorization& sender, const AuthorizationTicket& ticket, const char *message )
{
	// Set up the message box and display it.
	CMessageDlg messageDlg;
	messageDlg.SetTitle(ticket.IsTrial() ? IDS_TITLE_BUY_CORONA : IDS_TITLE_UPGRADE_CORONA);
	messageDlg.SetText(CString(message));
	messageDlg.SetDefaultText(ticket.IsTrial() ? IDS_BUYNOW : IDS_UPGRADE);
	messageDlg.SetAltText(IDS_VERIFY_PURCHASE);
	messageDlg.SetButton3Text(ticket.IsTrial() ? IDS_CONTINUETRIAL : IDS_CONTINUE);
	messageDlg.SetUrl(ticket.IsTrial() ? Rtt::Authorization::kUrlPurchase : Rtt::Authorization::kUrlUpgradeToPro);
	messageDlg.SetIconStyle(MB_ICONEXCLAMATION);
	int code =  messageDlg.DoModal();
	
	// If user clicks "Verify Purchase", then re-authorize the application.
    bool result = true;
	if (ID_MSG_BUTTON2 == code)
	{
		result = Reauthorize( const_cast< Authorization*>( &sender ) );
	}
	return result;
}

void
WinAuthorizationDelegate::ShowRenewal( const Authorization& sender, const AuthorizationTicket& ticket )
{
//TODO: Change this code to call the  ticket.GetNumDaysLeft() function like Mac?
	time_t current = time( NULL );
	time_t expiration = ticket.GetExpiration();
	time_t kNumSecondsPerDay = 24*60*60;
	float numDaysLeft = ( expiration > current ? ((float)expiration - current)/kNumSecondsPerDay : 0 );

	if ( numDaysLeft < 10 )
	{
		// Determine if we should pester the user with a subscription expiration warning/reminder.
		bool shouldRemind = true;
		Rtt::String pref;
		sender.GetServices().GetPreference( "RenewalReminder", &pref );
		if ( pref.GetString() )
		{
			S32 t = strtol( pref.GetString(), NULL, 16 );
			if ( t > 0 && t < current )
			{
				time_t dt = current - t;
				shouldRemind = ( dt > kNumSecondsPerDay*10 ); // Every 10 days
			}
		}

		// Fetch the user's subscription type.
		auto subscription = ticket.GetSubscription();

		// Do not show a reminder if the subscription is unknown or if it has expired.
		// Note: The only subscription that can expire now is Enterprise, which doesn't impact simulator usage.
		if ((numDaysLeft <= 0) || (AuthorizationTicket::kUnknownSubscription == subscription))
		{
			shouldRemind = false;
		}

		// Show the expiration warning, if appropriate.
		if ( shouldRemind && Rtt_VERIFY( ticket.IsPaidSubscription() ) )
		{
			CMessageDlg messageDlg;
			messageDlg.SetTitle(numDaysLeft > 0 ? IDS_SUBSCRIPTION_EXPIRING : IDS_SUBSCRIPTION_EXPIRED);

			WinString subscriptionName;
			subscriptionName.SetUTF8(ticket.DisplayStringForSubscription(subscription));

			CString message;
			if (numDaysLeft > 0)
			{
				if ( numDaysLeft > 1 )
				{
					message.Format(IDS_SUBSCRIPTION_EXPIRING_DAYS, subscriptionName.GetTCHAR(), (int)numDaysLeft);
				}
				else
				{
					message.Format(IDS_SUBSCRIPTION_EXPIRING_SOON, subscriptionName.GetTCHAR());
				}
			}
			else
			{
				message.Format(IDS_SUBSCRIPTION_EXPIRED_MODE, subscriptionName.GetTCHAR());
			}
			messageDlg.SetText(message);
			messageDlg.SetDefaultText(IDS_RENEW_NOW);
			messageDlg.SetAltText(IDS_VERIFY_PURCHASE);
			messageDlg.SetButton3Text(IDS_REMIND_ME_LATER);
			messageDlg.SetUrl(Rtt::Authorization::kUrlPurchase);
			messageDlg.SetIconStyle(MB_ICONEXCLAMATION);

			int code =  messageDlg.DoModal();

			// If user clicks "Verify Purchase", then re-authorize the application.
			bool result = true;
			if (ID_MSG_BUTTON2 == code)
			{
				bool result = Reauthorize( const_cast< Authorization*>( &sender ) );

				if ( ! result )
				{
					PostQuitMessage( 0 );
				}
			}
//TODO: When the "Remind Me Later" button is clicked, update the ticket to delay the reminder for 1 day like Mac.
		}		

	}
}

bool
WinAuthorizationDelegate::TicketNotInstalled( const Authorization& sender )
{
	bool result = false;

	// Login dialog only disappears after a succesful login. If they
	// choose register or the help button, the dialog will not disappear.
	// When a login is attempted, WinAuthorizationDelegate::Login() is called
	// while the modal dialog is still running.
    pLoginDlg->SetAuthorizer( this, &sender );
	if( pLoginDlg->DoModal() == IDOK )
	{
		result = true;
	}
	return result;
}

const char*
WinAuthorizationDelegate::PasswordInvalid( const Authorization& sender, const char *usr )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
    return NULL;  // not implemented on Mac side
}

// Checks if the ticket's unique machine ID is using the old ID generating scheme based on the MAC address,
// and if so, attempts to de-authorizes it and forces the user to use the new machine ID based on machine SID.
// Returns true if verification has passed. Returns false if failed, which prevents the authorization
// object from performing its default verification.
bool
WinAuthorizationDelegate::PreVerification( const Authorization& sender ) const
{
	bool reauthorizationRequired = false;

	// Fetch the ticket.
	const AuthorizationTicket *ticketPointer = sender.GetTicket();
	if (NULL == ticketPointer)
	{
		return false;
	}

	// Verify the authorization token's device ID.
	const char *deviceId = ticketPointer->GetDeviceId();
	if (NULL == deviceId)
	{
		return false;
	}
	if (strlen(deviceId) < 32)
	{
		// The device ID was assigned using an old ID generating scheme based on a non-hashed MAC address.
		// Inform the user that this machine needs to be reauthorized.
		reauthorizationRequired = true;
	}
	else
	{
		// Check if the authorization token's device ID still matches a device on this machine.
		Rtt::WinDevice& device = (Rtt::WinDevice&)(sender.GetServices().Platform().GetDevice());
		if (strcmp(deviceId, device.GetUniqueIdentifier( MPlatformDevice::kDeviceIdentifier )) != 0)
		{
			// The primary ID of this machine has changed since it was last authorized.
			// This can happen if network device order has changed or if the machine SID has changed.
			// Check if this machine has any other device that has a matching ID.
			if (device.ContainsUniqueIdentifier(deviceId))
			{
				// Found a device ID on the local machine. Make it the primary machine ID.
				// This allows authorization/deauthorization to continue to work.
				device.SetUniqueIdentifier(deviceId);
			}
			else
			{
				// The authorization token's device ID cannot be found on the machine. This means that the
				// hardware has changed or the user has copied authorization data from another machine.
				// Force the user to re-authorize this machine before running this software.
				reauthorizationRequired = true;
			}
		}
	}

	// Reauthorize this machine if the above code requires it. This is typically needed if the
	// authorization token contains a device ID that doesn't match any device IDs on this machine.
	// This will deauthorize this machine and then restart this application for authorization.
	if (reauthorizationRequired)
	{
		ShowReauthorizationRequiredWarning(sender);
		return false;
	}

	// Custom verification has passed.
	return true;
}

bool
WinAuthorizationDelegate::AuthorizationDidFail( const Authorization& sender ) const
{
	return false;
}

bool
WinAuthorizationDelegate::VerificationDidFail( const Authorization& sender ) const
{
	bool result = false;

	const AuthorizationTicket *ticket = sender.GetTicket();

	if ( ! ticket )
	{
		// We should never reach this point.  Something is seriously broken upstream
		// if we do not have a ticket when this method is called!!!
		Rtt_ASSERT_NOT_REACHED();
		result = ((WinAuthorizationDelegate *)this)->TicketNotInstalled( sender );      // show Login dialog
		if ( ! result )
		{
			CMessageDlg messageDlg;
			messageDlg.SetText( IDS_APPCORRUPTED );
			messageDlg.SetDefaultText( IDS_QUIT );
			messageDlg.SetIconStyle( MB_ICONSTOP );
			messageDlg.DoModal();
		}
	}
	else
	{
		const char *url = NULL;
        int idTitle = 0;
        int idMsg = 0;
		int idButtonDefault = 0;
		int idButtonAlt = IDS_QUIT;
		int alertStyle = MB_ICONEXCLAMATION;

		if ( ticket->IsTrial() )
		{
            idTitle = IDS_TRIAL_EXPIRED;
			idMsg = IDS_CLICKBUYNOW;
			idButtonDefault = IDS_BUYNOW;
			idButtonAlt = IDS_VERIFY_PURCHASE;
			url = Authorization::kUrlPurchase;
		}
		else
		{
			idTitle = IDS_SUBSCRIPTION_EXPIRED;
			idMsg = IDS_NEEDTORENEW;
			idButtonDefault = IDS_RENEW;
			idButtonAlt = IDS_ALREADYRENEWED;
			url = Authorization::kUrlRenew;
		}

        CMessageDlg messageDlg;
        messageDlg.SetTitle( idTitle );
        messageDlg.SetText( idMsg );
        messageDlg.SetDefaultText( idButtonDefault );
        messageDlg.SetAltText( idButtonAlt );
        messageDlg.SetButton3Text( IDS_QUIT );
        messageDlg.SetUrl( url );
        messageDlg.SetIconStyle( alertStyle );

		int code =  messageDlg.DoModal();

        // Mac code comment says:
		    // Dialog only disappears when user clicks "Quit" or "Already Purchased"/"Already Renewed"
         	// and in the latter case, the reauth must be successful for the dialog to dismiss.
        // Windows will behave differently for now.
        // App will exit on Quit or Buy Now (they can re-launch after purchase)
        // Already purchased leads to re-auth here.
        if( code == ID_MSG_BUTTON2 )  // Alternate button = ID_MSG_BUTTON2 = Already purchased/renewed
		{
			result = Reauthorize( const_cast< Authorization*>( &sender ) );   // show Login dialog
		}
	}

	if ( ! result )
	{
         PostQuitMessage( 0 );
	}

	return result;
}

bool 
WinAuthorizationDelegate::ShowDeviceBuildUpsell( const Authorization& sender, const AuthorizationTicket& ticket, TargetDevice::Platform platform )
{
	// Build the message string.
	CString message;
	if (TargetDevice::kWebPlatform != platform )
	{
		message.Format(IDS_BUILDUPSELL, CString(TargetDevice::StringForPlatform(platform)));
	}
	else
	{
		message.Format(IDS_BUILDWEBUPSELL, CString(TargetDevice::StringForPlatform(platform)));
	}
	
	
	// Set up message box and display it.
	CMessageDlg messageDlg;
	messageDlg.SetTitle(IDS_TITLE_UPGRADE_CORONA);
	messageDlg.SetText(message);
	messageDlg.SetDefaultText(IDS_UPGRADE);
	messageDlg.SetAltText(IDS_VERIFY_PURCHASE);
	messageDlg.SetButton3Text(IDS_OK);
	messageDlg.SetUrl(Rtt::Authorization::kUrlUpgradeToPro);
	messageDlg.SetIconStyle(MB_ICONEXCLAMATION);
	int code =  messageDlg.DoModal();
	
	// If user clicks "Verify Purchase", then re-authorize the application.
    bool result = true;
	if (ID_MSG_BUTTON2 == code)
	{
		result = Reauthorize( const_cast< Authorization*>( &sender ) );
	}
	return result;
}

bool
WinAuthorizationDelegate::ShowTrialBuild( const Authorization& sender, const AuthorizationTicket& ticket )
{
	// Do not show a trial window for non-trial users.
	Rtt_ASSERT( ticket.IsTrial() );
	if (ticket.IsTrial() == false)
	{
		return true;
	}
	
	// Set up message box and display it.
	CMessageDlg messageDlg;
	messageDlg.SetTitle(IDS_TITLE_BUY_CORONA);
	messageDlg.SetText(IDS_TRIALBUILD);
	messageDlg.SetDefaultText(IDS_BUYNOW);
	messageDlg.SetAltText(IDS_VERIFY_PURCHASE);
	messageDlg.SetButton3Text(IDS_CONTINUETRIAL);
	messageDlg.SetUrl(Rtt::Authorization::kUrlPurchase);
	messageDlg.SetIconStyle(MB_ICONEXCLAMATION);
	int code =  messageDlg.DoModal();
	
	// If user clicks "Verify Purchase", then re-authorize the application.
    bool result = true;
	if (ID_MSG_BUTTON2 == code)
	{
		result = Reauthorize( const_cast< Authorization*>( &sender ) );
	}
	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

