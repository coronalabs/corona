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

#include "Rtt_Authorization.h"

#include "Core/Rtt_VersionTimestamp.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_MAuthorizationDelegate.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_PlatformConnection.h"
#include "Rtt_PlatformDictionaryWrapper.h"
#include "Rtt_WebServicesSession.h"
#include "Core/Rtt_String.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "rsa.hpp"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char Authorization::kUrlHome[] = "https://coronalabs.com/products/corona-sdk/?utm_source=simulator";
const char Authorization::kUrlPurchase[] = "https://coronalabs.com/links/simulator/buy-corona";
const char Authorization::kUrlUpgradeToPro[] = "https://coronalabs.com/links/simulator/upgrade-corona";
const char Authorization::kUrlRenew[] = "https://coronalabs.com/links/simulator/renew";
const char Authorization::kUrlRegister[] = "https://coronalabs.com/links/simulator/register";
const char Authorization::kUrlDownload[] = "https://coronalabs.com/links/simulator/download";
const char Authorization::kUrlAgreement[] = "https://coronalabs.com/links/simulator/agreement";

// ----------------------------------------------------------------------------

const char Authorization::kTicketKey[] = "Resource";
const char Authorization::kSuppressFeedbackKey[] = "SuppressFeedback";
const char Authorization::kVersionKey[] = "Version";

const char Authorization::kUsernameKey[] = "Username";

const char Authorization::kRenewalReminderKey[] = "RenewalReminder";

Authorization::Authorization( const MPlatformServices& services, MAuthorizationDelegate& delegate )
:	fServices( services ),
	fDelegate( delegate ),
	fTicket( NULL ),
	fEulaTimestamp( 0 ),
	fUseLibraryPreferences( true ),
	fLoginMessage( NULL )
{
}

Authorization::~Authorization()
{
	Rtt_DELETE( fTicket );
}

// Move the password from secure preference to encrypted plaintext in user preferences.
// This allows easy migration from user preferences to library preferences.
static void 
MigratePassword( const Rtt::MPlatformServices & services, const char * username )
{
#ifndef Rtt_WIN_ENV
	// Have we migrated before?
	Rtt::String encryptedPassword;
	
	services.GetPreference( username, &encryptedPassword );

	if ( encryptedPassword.GetString() )
		return;

	Rtt::String plainTextPassword;

	services.GetSecurePreference( username, &plainTextPassword );

	if ( plainTextPassword.GetString() )
	{
		Rtt::String encryptedPassword;
		Rtt::Authorization::Encrypt( plainTextPassword.GetString(), &encryptedPassword );
		services.SetPreference( username, encryptedPassword.GetString() );
#if 0
		// Leave this out for now. This allows beta 7 users to use the same PW, instead of resetting it for them.
		services.SetSecurePreference( username, NULL );
#endif
	}
#endif
}

// This function looks for a ticket in the prefs. If none is found, it attempts
// to get one, prompting the user to login if need be. 
// The rights granted by the ticket can be checked by calling Authorization::VerifyTicket()
// after Initialize() is complete.
bool
Authorization::Initialize(bool initiateLogin /* = false */) const
{
	bool result;

	// Load the local machine's ticket, if it has one.
	result = LoadTicket();
	if (fTicket != NULL)
	{
		result = fDelegate.PreVerification(*this);
		if (false == result)
		{
			return false;
		}
		result = fTicket->Verify();
	}
	if ((false == result) || (NULL == fTicket))
	{
		// Ticket not found or is invalid. Clear the machine's ticket info so that
		// the code below can have the user attempt to set up a new one.
		ClearTicket();
		result = false;
	}

	// Check if a valid ticket has been loaded. This indicates if this machine has been authorized in the past.
	if (!result)
	{
#if 0 // We do this in the DMG so we no longer do it here
		// Display the license agreement to the user.
		if (!fDelegate.ShowLicense(*this))
		{
			// User did not agree to the license. Exit out of the application.
			fServices.Terminate();
			return false;
		}
		else
		{
			// Record timestamp of when the user agreed to the license.
			fEulaTimestamp = time( NULL );
			time_t timestamp = time( NULL );
			char value[sizeof( Rtt_STRING_BUILD ) + 32];
			sprintf( value, "%s.%lu", Rtt_STRING_BUILD, timestamp );
			fServices.SetPreference( kVersionKey, value );
		}
#endif // 0
	}
	else
	{
		// This machine is authorized. Copy the user's password to the local machine in case it is not there.
		// This must happen before we display a trial dialog.
		MigratePassword( fServices, fTicket->GetUsername() );
	}

	result = InitializeTicket();
	
	if (!result)
	{
		if (initiateLogin)
		{
			// Display a login window if the user does not have a ticket yet.
			for (; !result && fDelegate.TicketNotInstalled(*this); result = InitializeTicket())
			{
				// Rtt_ASSERT( ! fTicket );
			}
			
			return result;
		}
	}

	// Return true if the user has a ticket and this machine is authorized to use the software.
	return result;
}

// ----------------------------------------------------------------------------

// Public Key

// Modulus: n = p*q
static const U32 kN[] =
{
	0x6eb00c05, 0x9d0c0aab, 0xe33d80f3, 0x90a5e1b0, 0xb69824b6, 0x202fc057, 0x608f5115, 0xf2d9e98d,
	0x88d69cf5, 0x25e33cbe, 0x04b5c501, 0x006b877f, 0xb19f5b5c, 0x0a000ad2, 0x9b2c7309, 0x681bda4b,
	0x7307bbbc, 0x89e557d8, 0xcb1f188a, 0x73f0f646, 0xe3f8461c, 0xbec79c4a, 0x90aab10a, 0x353cb4af,
	0xe794a08f, 0x2a66d2c1, 0x2b9c46f9, 0x2b7845d6, 0xdaffc7b8, 0x82f7cee6, 0x74e2d442, 0x055bea40,
	0x6c59a236, 0x75d8649c, 0x945c028c, 0xb24fdb0a, 0xdd8aa520, 0x494b0d05, 0x78f74aef, 0x80710473,
	0x3a2a45a9, 0x9bdd4666, 0xb4095cad, 0x94784579, 0x0fd5d557, 0x08d60496, 0x67b8689f, 0x527b3efb,
	0x44b957e0, 0x2ff42a04, 0xc31381fe, 0x34475694, 0x0f39235f, 0x0fc5f90f, 0x48fa1061, 0xbb3c8ae6,
	0x1c6f116b, 0x321876a7, 0xe2554ede, 0xfdbe3755, 0x7170e952, 0x183ccd9d, 0x0e1b2c2d, 0x1739e8ef
};

// Exponent: coprime/relative prime to (p-1)*(q-1)
static const U32 kE[] =
{
	0x0000c351
};

// ----------------------------------------------------------------------------

// Loads a new ticket from the local machine if not already done so and then verifies if it is correct.
// Replaces member variable "fTicket" with new ticket information if successfully loaded and validated.
// Returns true if a valid ticket was loaded. Returns false if local ticket not found or if it is invalid.
bool
Authorization::InitializeTicket() const
{
	// If a ticket has not been initialized yet, then do so now.
	if (NULL == fTicket)
	{
		// Load the local machine's ticket.
		LoadTicket();

		// Verify that a ticket has been loaded and that it is valid.
		if ((fTicket != NULL) && (fTicket->Verify() == false))
		{
			// The loaded ticket is invalid. Delete it.
			Rtt_DELETE( fTicket );
			fTicket = NULL;
		}

		// If we don't have a ticket object, then assume that the local machine may be storing
		// invalid ticket data and remove that data from the local machine.
		if (!fTicket)
		{
			ClearTicket();
		}
	}

	// Return true if a "valid" ticket has been loaded.
	return (fTicket != NULL);
}

// Loads the local machine's ticket. Does not verify if the ticket is correct.
// Returns true if the ticket was loaded. Returns false if ticket was not found on the local machine.
bool
Authorization::LoadTicket() const
{
	// Delete the current ticket if it exists.
	if (fTicket != NULL)
	{
		Rtt_DELETE( fTicket );
		fTicket = NULL;
	}

	// Fetch the this machine's encrypted ticket data.
	Rtt::String cipherData;
	fServices.GetLibraryPreference( kTicketKey, &cipherData );
	if ( !cipherData.GetString() )
		fServices.GetPreference( kTicketKey, &cipherData );

	// Decrypt the ticket data and store it in a ticket object.
	fTicket = CreateTicketFrom(cipherData.GetString());

	// Return true if a ticket was loaded.
	return (fTicket != NULL);
}

// Creates and returns an AuthorizationTicket object based on the given encrypted ticket string.
// Returns NULL if given an invalid argument or unable to create a ticket object from it.
// It is the caller's responsibility to delete the return AuthorizationTicket object.
AuthorizationTicket*
Authorization::CreateTicketFrom(const char *encryptedTicketData) const
{
	AuthorizationTicket *ticketPointer = NULL;

	// Validate argument.
	if ((NULL == encryptedTicketData) || (0 == encryptedTicketData[0]))
	{
		return NULL;
	}

	// Decrypt the given ticket data and create a ticket object from it.
	vlong m; m.load( (unsigned int*)kN, sizeof(kN)/sizeof(kN[0]) );
	vlong e; e.load( (unsigned int*)kE, sizeof(kE)/sizeof(kE[0]) );
	public_key keyPublic(m, e);
	size_t numBytes = 0;
	U8 *plainData = AuthorizationTicket::CipherToPlain(keyPublic, encryptedTicketData, numBytes);
	if (plainData)
	{
		const MPlatform& platform = fServices.Platform();
		ticketPointer = Rtt_NEW(&platform.GetAllocator(), AuthorizationTicket(platform.GetDevice(), (char*)plainData));
		free(plainData);
	}

	return ticketPointer;
}

// Deletes the current ticket and removes ticket data from the local machine.
void
Authorization::ClearTicket() const
{
	if (fTicket != NULL)
	{
		Rtt_DELETE( fTicket );
		fTicket = NULL;
	}
	fServices.SetPreference( kTicketKey, NULL );
}

void
Authorization::Encrypt( const char *plaintext, Rtt::String * result )
{
	vlong m; m.load( (unsigned int*)kN, sizeof(kN)/sizeof(kN[0]) );
	vlong e; e.load( (unsigned int*)kE, sizeof(kE)/sizeof(kE[0]) );

	public_key k( m, e );

	size_t numBytes = 0;
	U8 *ciphertext = AuthorizationTicket::PlainToCipherPublic( k, plaintext, numBytes );

	if ( Rtt_VERIFY( ciphertext ) )
	{
		// Each U8 is emitted as a 2 char hex + 1 bytes for null-termination
		
		result->Reserve( numBytes*2 + 1 );

		char *p = result->GetMutableString();
		for ( size_t i = 0; i < numBytes; i++ )
		{
			p += sprintf( p, "%02x", ciphertext[i] );
		}

		free( ciphertext );
	}
}

// Called from trial dialog
int
Authorization::Reauthorize() const
{
	int result = kAuthorizationError;

	// Reauthorization (at least on the client) implies we've got an existing ticket.
	if ( Rtt_VERIFY( fTicket ) )
	{
		const char *usr = fTicket->GetUsername();
		Rtt::String encryptedPassword;

		fServices.GetLibraryPreference( usr, &encryptedPassword );
		if ( !encryptedPassword.GetString() )
		{
			fServices.GetPreference( usr, &encryptedPassword );
		}

		result = Authorize( usr, encryptedPassword.GetString(), true );
	}

	return result;
}

static const char kCoronaAuthorize[] = "kCoronaAuthorize";
static const char kCoronaAuthorizeDeviceBlacklisted[] = "kCoronaAuthorizeDeviceBlacklisted";
static const char kCoronaAuthorizeTrialExpired[] = "kCoronaAuthorizeTrialExpired";
static const char kCoronaAuthorizeBasicExpired[] = "kCoronaAuthorizeBasicExpired";
static const char kCoronaAuthorizeQuotaLimitReached[] = "kCoronaAuthorizeQuotaLimitReached";
static const char kCoronaAuthorizePlatformDoesNotMatch[] = "kCoronaAuthorizePlatformDoesNotMatch";
static const char kCoronaAuthorizeDatabaseError[] = "kCoronaAuthorizeDatabaseError";
static const char kCoronaAuthorizeDBError[] = "kCoronaAuthorizeDBError";
static const char kCoronaAuthorizeApiKeyInvalid[] = "kCoronaAuthorizeApiKeyInvalid";
static const char kCoronaAuthorizeDeauthorizeSuccess[] = "kCoronaAuthorizeDeauthorizeSuccess";
static const char kCoronaAuthorizeUnverifiedUserError[] = "kCoronaAuthorizeUnverifiedUserError";
//static const char [] = "";

static int
ConvertAuthorizeResultToResultCode( const char *str )
{
	int result = Authorization::kAuthorizationError;

	if ( 0 == strncmp( str, kCoronaAuthorize, sizeof( kCoronaAuthorize ) - 1 ) )
	{
		if ( 0 == Rtt_StringCompare( str, kCoronaAuthorizeDeviceBlacklisted ) )
		{
			result = Authorization::kAuthorizationDeviceBlacklisted;
		}
		else if ( 0 == Rtt_StringCompare( str, kCoronaAuthorizeTrialExpired ) )
		{
			result = Authorization::kAuthorizationTrialExpired;
		}
		else if ( 0 == Rtt_StringCompare( str, kCoronaAuthorizeBasicExpired ) )
		{
			result = Authorization::kAuthorizationBasicExpired;
		}
		else if ( 0 == Rtt_StringCompare( str, kCoronaAuthorizeQuotaLimitReached ) )
		{
			result = Authorization::kAuthorizationQuotaLimitReached;
		}
		else if ( 0 == Rtt_StringCompare( str, kCoronaAuthorizePlatformDoesNotMatch ) )
		{
			result = Authorization::kAuthorizationPlatformDoesNotMatch;
		}
		else if ( 0 == Rtt_StringCompare( str, kCoronaAuthorizeDatabaseError )
				  || 0 == Rtt_StringCompare( str, kCoronaAuthorizeDBError ) )
		{
			result = Authorization::kAuthorizationDatabaseError;
		}
		else if ( 0 == Rtt_StringCompare( str, kCoronaAuthorizeApiKeyInvalid ) )
		{
			result = Authorization::kAuthorizationApiKeyInvalid;
		}
		else if ( 0 == Rtt_StringCompare( str, kCoronaAuthorizeDeauthorizeSuccess ) )
		{
			result = Authorization::kAuthorizationDeauthorizeSuccess;
		}
		else if ( 0 == Rtt_StringCompare( str, kCoronaAuthorizeUnverifiedUserError ) )
		{
			result = Authorization::kAuthorizationUnverifiedUserError;
		}
	}
	else
	{
		result = Authorization::kAuthorizationReceivedTicket;
	}

	// To simplify debugging...
	// return Authorization::kAuthorizationDeviceBlacklisted;
	// return Authorization::kAuthorizationTrialExpired;
	// return Authorization::kAuthorizationBasicExpired;
	// return Authorization::kAuthorizationQuotaLimitReached;
	// return Authorization::kAuthorizationPlatformDoesNotMatch;

	return result;
}

static int
WebSessionCodeToAuthorizationCode( int code )
{
	int result = Authorization::kAuthorizationError;

	switch( code )
	{
		case WebServicesSession::kApiKeyError:
			result = Authorization::kAuthorizationApiKeyInvalid;
			break;
		case WebServicesSession::kTokenExpiredError:
			result = Authorization::kAuthorizationTokenExpired;
			break;
		case WebServicesSession::kAgreementError:
			result = Authorization::kAuthorizationAgreementError;
			break;
		case WebServicesSession::kUnverifiedUserError:
			result = Authorization::kAuthorizationUnverifiedUserError;
			break;
		case WebServicesSession::kConnectionError:
			result = Authorization::kAuthorizationConnectionError;
			break;
		default:
			result = Authorization::kAuthorizationLoginFailed;
			break;
	}

	return result;
}

int
Authorization::Authorize( const char *usr, const char *encryptedPassword, bool storeLoginInfo, bool verifyTicket ) const
{
	int result = kAuthorizationError;

	const MPlatformDevice& device = fServices.Platform().GetDevice();
	const char *deviceId = device.GetUniqueIdentifier( MPlatformDevice::kDeviceIdentifier );
	size_t platformType = AuthorizationTicket::StringToPlatform( device.GetPlatformName() );

	if ( Rtt_VERIFY( usr && encryptedPassword && deviceId && platformType ) )
	{
		WebServicesSession session( fServices );

		int loginResult = session.LoginWithEncryptedPassword( WebServicesSession::CoronaServerUrl(fServices), usr, encryptedPassword );
		if ( WebServicesSession::kNoError == loginResult )
		{
			char timestamp[32];
			sprintf( timestamp, "%lx", Rtt_VersionTimestamp() );
			KeyValuePair params[] =
			{
				{ "deviceId", deviceId, kStringValueType },
				{ "timestamp", timestamp, kStringValueType },
				{ "platform", (void *)platformType, kIntegerValueType }
			};
			PlatformDictionaryWrapper* response = session.Call( "corona.authorize", params, sizeof(params)/sizeof(params[0]) );
			if ( response )
			{
				const char *ticketCipherData = response->ValueForKey( "result" ); Rtt_ASSERT( ticketCipherData );

				result = ConvertAuthorizeResultToResultCode( ticketCipherData );
				if ( kAuthorizationReceivedTicket == result )
				{
					fServices.SetPreference( kTicketKey, (char*)ticketCipherData );
					result = kAuthorizationReceivedTicket;

					if ( storeLoginInfo )
					{
                        // The server lowercases the email address (username) which goes into the ticket so we
                        // need to do the same when we save it or we wont find it when we try to load it next time
                        Rtt::String usernameString;

                        usernameString.Set( usr );
                        char *p = usernameString.GetMutableString();

                        while ( *p )
                        {
                            *p = tolower( *p );
                            ++p;
                        }

						// Store username in a visible area to aid support issues
						// All code should obtain the username from the ticket
						// and NEVER from the prefs
						fServices.SetPreference( kUsernameKey, usernameString.GetString() );

						fServices.SetPreference( usernameString.GetString(), encryptedPassword );
					}
				}

				Rtt_DELETE( response );
			}			
		}
		else
		{
			result = WebSessionCodeToAuthorizationCode( loginResult );
		}
	}

	if ( verifyTicket && Authorization::kAuthorizationReceivedTicket == result )
	{
		// We need to update fTicket with ticketCipherData to properly verify the ticket
		Rtt_DELETE( fTicket );
		fTicket = NULL;

		if ( InitializeTicket()
			 && ! fTicket->IsAppAllowedToRun() )
		{
			result = fTicket->IsTrial() ? Authorization::kAuthorizationTrialExpired : Authorization::kAuthorizationBasicExpired;
		}
	}

	return result;
}

int
Authorization::Deauthorize( const char *usr, const char *encryptedPassword, const char *inTicket ) const
{
	AuthorizationTicket *ticketPointer = NULL;
	int result = kAuthorizationError;

	// Handle the placeholder ticket case specially
	if (fTicket != NULL && fTicket->IsPlaceholder())
	{
		result = kAuthorizationDeauthorizeSuccess;
	}
	else
	{
		// Deserialize the given encrypted ticket string to a ticket object.
		ticketPointer = CreateTicketFrom(inTicket);

		// Deauthorize the given ticket if valid.
		if ( Rtt_VERIFY( usr && encryptedPassword && ticketPointer && ticketPointer->GetDeviceId() ) )
		{
			Rtt::String ticketString;
			WebServicesSession session( fServices );

			ticketString.SetSrc( inTicket );
			if ( ! ticketString.GetString() )
			{
				// No ticket found, so attempt reauth to obtain ticket
				if ( kAuthorizationReceivedTicket == Authorize( usr, encryptedPassword, false, false ) )
				{
					fServices.GetPreference( kTicketKey, &ticketString );
				}
			}

			if ( ticketString.GetString() )
			{
				int loginResult = session.LoginWithEncryptedPassword( WebServicesSession::CoronaServerUrl(fServices), usr, encryptedPassword );
				if ( WebServicesSession::kNoError == loginResult )
				{
					char timestamp[32];
					sprintf( timestamp, "%lx", Rtt_VersionTimestamp() );
					KeyValuePair params[] =
					{
						{ "deviceId", ticketPointer->GetDeviceId(), kStringValueType },
						{ "timestamp", timestamp, kStringValueType },
						{ "ticket", ticketString.GetString(), kStringValueType }
					};
					PlatformDictionaryWrapper* response = session.Call( "corona.deauthorize", params, sizeof(params)/sizeof(params[0]) );
					if ( response )
					{
						const char *resultString = response->ValueForKey( "result" ); Rtt_ASSERT( resultString );

						result = ConvertAuthorizeResultToResultCode( resultString );
						if ( kAuthorizationDeauthorizeSuccess == result )
						{
							fServices.SetPreference( kTicketKey, NULL );
							fServices.SetPreference( kSuppressFeedbackKey, NULL );

#if 0
							if ( ! fServices.SetSecurePreference( usr, NULL ) )
							{
								result = kAuthorizationPassword;
							}
#else
							fServices.SetPreference( usr, NULL );
#endif
							// Bug: 2589
							// Clear the kUsernameKey last because SetPreference on Mac 
							// currently uses the key for purposes of managing the two different
							// preference system.
							fServices.SetPreference( kUsernameKey, NULL );
						}
						else
						{
							// Attempt to force a reauthorize and prompt user to try again.
							if ( kAuthorizationReceivedTicket == Authorize( usr, encryptedPassword, false, false ) )
							{
								result = kAuthorizationDeauthorizeTryLater;
							}
							else
							{
								result = WebServicesSession::kCriticalError;
							}
						}

						Rtt_DELETE( response );
					}			
				}
				else
				{
					result = WebSessionCodeToAuthorizationCode( loginResult );
				}
			}
		}
	}

	// Delete the ticket.
	if (ticketPointer != NULL)
	{
		Rtt_DELETE(ticketPointer);
		ticketPointer = NULL;
	}

	return result;
}

bool
Authorization::IsFirstRun() const
{
	InitializeTicket();

	bool result = ! fTicket;

	return result;
}

bool
Authorization::VerifyTicket() const
{
	bool result = VerifyTicketOnce();

	// Delegate returns true if reauthorization occurred
	for ( ;
		  ! result && fDelegate.VerificationDidFail( * this );
		  result = VerifyTicketOnce() )
	{
		Rtt_DELETE( fTicket );
		fTicket = NULL;
	}

	if ( ! result )
	{
		fServices.Terminate();
	}

	return result;
}

bool
Authorization::VerifyTicketOnce() const
{
	bool result = false;

	InitializeTicket();

	if ( Rtt_VERIFY( fTicket ) )
	{
		result = fTicket->IsAppAllowedToRun();
	}

	if ( result )
	{
		if ( fTicket->IsTrial() )
		{
			// Reduce the upsell ...
			// result = fDelegate.ShowTrial( * this,  * fTicket );
		}
		else
		{
			fDelegate.ShowRenewal( *this, * fTicket );
						
			if (! result)
			{
				// Couldn't validate the ticket, force a login
				ClearTicket();
				
				result = Initialize( );
			}

		}
	}

	return result;
}

void
Authorization::SetLoginMessage(const char *mesg)
{
	if (fLoginMessage != NULL)
	{
		free(fLoginMessage);
	}
	
	fLoginMessage = strdup(mesg);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

