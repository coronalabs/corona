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

#include "Rtt_MacAuthorizationDelegate.h"

#include "Rtt_Authorization.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_WebServicesSession.h"

#import "AppDelegate.h"

#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>
#import <AppKit/NSAlert.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSPanel.h>

static NSUInteger kMaxUsernameLength = 255;
static NSUInteger kMaxPasswordLength = 255;

// ----------------------------------------------------------------------------

typedef struct AuthorizationContext
{
	Rtt::Authorization *authorizer;
	Rtt::MacAuthorizationDelegate *delegate;
}
AuthorizationContext;

// ----------------------------------------------------------------------------

static const char kDoNotStopModal[] = "";

// ----------------------------------------------------------------------------

@interface LoginDelegate : NSObject< DialogControllerDelegate >
{
	const Rtt::MPlatform *fPlatform;
	int fAlertViewStopModalCode;
}

// EW says we need the colon
@property (nonatomic, readwrite, getter=alertViewStopModalCode, setter=setAlertViewStopModalCode:) int fAlertViewStopModalCode;

-(id)initWithPlatform:(const Rtt::MPlatform*)platform ;
-(BOOL)validateValue:(id *)ioValue forKeyPath:(NSString *)inKeyPath error:(NSError **)outError;

// For alert view delegate
-(void)alertDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;

@end


@implementation LoginDelegate

@synthesize fAlertViewStopModalCode;

-(id)initWithPlatform:(const Rtt::MPlatform*)platform
{
	self = [super init];
	if ( self )
	{
		Rtt_ASSERT( platform );
		fPlatform = platform;
		fAlertViewStopModalCode = kActionUnknown;
	}
	return self;
}

-(BOOL)validateValue:(id *)value forKeyPath:(NSString *)inKeyPath error:(NSError **)outError
{
	if (value == nil)
	{
		return YES;
	}

	BOOL result = YES;

	return result;
}

-(BOOL)shouldStopModal:(DialogController*)sender withCode:(NSInteger)code
{
	BOOL result = YES;

	switch( code )
	{
		case kActionDefault:
			{
				AuthorizationContext* c = (AuthorizationContext*)sender.userdata;
				result = c->delegate->Login( c->authorizer );
			}
			break;
		case kActionAlternate:
			fPlatform->OpenURL( Rtt::Authorization::kUrlRegister );
			result = NO;
			break;
		case kActionOther1:
			fPlatform->OpenURL( Rtt::Authorization::kUrlHome );
			result = NO;
			break;
		case kActionOther2:
			// We no longer exit the app on cancellation of login
			// [NSApp terminate:self];
			break;
	}

	return result;
}

-(BOOL)isActionEnabled:(DialogController*)sender forCode:(NSInteger)code
{
	BOOL result =
		(kActionDefault != code)
		|| ( [sender.inputs valueForKey:@"username"] && [sender.inputs valueForKey:@"password"] );

	return result;
}

-(void)alertDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	if ( NSAlertFirstButtonReturn == returnCode )
	{
		if ( contextInfo )
		{
			fPlatform->OpenURL( (const char*)contextInfo );
		}

		if ( fAlertViewStopModalCode != kActionUnknown )
		{
			[[NSApplication sharedApplication] stopModalWithCode:fAlertViewStopModalCode];
		}
	}
	else
	{
		[[NSApplication sharedApplication] terminate:self];
	}
}

// TODO: Unused???
-(void)sheetDidEnd:(NSWindow*)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	// NSLog( @"sheetDidEnd: returnCode: %ld ", (long) returnCode );

	[sheet close];
	DialogController *controller = (DialogController*)contextInfo;
	[self shouldStopModal:controller withCode:returnCode];
}

@end

// ----------------------------------------------------------------------------

@interface TrialDelegate : NSObject< DialogControllerDelegate >
{
	const Rtt::MPlatform *fPlatform;
	const char *fUrl; // NEW TO GE
}

-(id)initWithPlatform:(const Rtt::MPlatform*)platform;

-(void)setUrl:(const char*)url; // NEW TO GE

// For alert view delegate
-(void)alertDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;

@end


@implementation TrialDelegate

-(id)initWithPlatform:(const Rtt::MPlatform*)platform
{
	self = [super init];
	if ( self )
	{
		Rtt_ASSERT( platform );
		fPlatform = platform;
		fUrl = Rtt::Authorization::kUrlPurchase; // NEW TO GE
	}
	return self;
}

-(void)setUrl:(const char*)url // NEW TO GE
{
	Rtt_ASSERT( url );
	fUrl = url;
}

-(BOOL)shouldStopModal:(DialogController*)sender withCode:(NSInteger)code
{
	BOOL result = YES;

	switch( code )
	{
		case kActionDefault:
			fPlatform->OpenURL( fUrl );
			result = NO;
			break;
		case kActionAlternate:
			{
				AuthorizationContext* c = (AuthorizationContext*)sender.userdata;
				result = c->delegate->Reauthorize( c->authorizer, sender, self );
			}
			break;
		case kActionOther1:
			result = YES;
			break;
	}

	return result;
}

-(void)alertDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	if ( NSAlertFirstButtonReturn == returnCode )
	{
		if ( kDoNotStopModal == contextInfo )
		{
			return;
		}

		if ( contextInfo )
		{
			fPlatform->OpenURL( (const char*)contextInfo );
		}
		else
		{
			[[NSApplication sharedApplication] stopModalWithCode:kActionAlternate];
		}
	}
	else
	{
		// App flow no longer means cancelling login should exit
		// [[NSApplication sharedApplication] terminate:self];
	}
}

-(void)sheetDidEnd:(NSWindow*)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	DialogController *controller = (DialogController*)contextInfo;
	if ( returnCode == kActionDefault )
	{
		[sheet close];
		AuthorizationContext* c = (AuthorizationContext*)controller.userdata;
		c->delegate->Reauthorize( c->authorizer, controller, self );
	}
}

@end

// ----------------------------------------------------------------------------

@implementation DialogController

@synthesize inputs;
@synthesize userdata;
@synthesize fWindow;

-(id)initWithNibNamed:(NSString*)name
{
	return [self initWithNibNamed:name delegate:nil];
}

-(id)initWithNibNamed:(NSString*)name delegate:(id<DialogControllerDelegate>)delegate
{
	self = [super init];
	if ( self )
	{
		fDelegate = delegate;
		fSheetDelegate = nil;
		inputs = [[NSMutableDictionary alloc] init];
		userdata = NULL;
		[NSBundle loadNibNamed:name owner:self];
		[fWindow setReleasedWhenClosed:NO];
		fState = kDialogControllerNormal;
	}
	return self;
}

-(void)dealloc
{
	[fWindow release];
	[inputs release];
	[super dealloc];
}

-(NSInteger)runModal
{
	return [self runModalWithMessage:nil];
}

-(NSInteger)runModalWithMessage:(NSString *)message
{
	int response = kActionUnknown;
	if ( Rtt_VERIFY( kDialogControllerNormal == fState ) )
	{
		[fWindow center];

		if (message != nil)
		{
			[fWindow makeKeyAndOrderFront:self];
			
			NSAlert* alert = [[[NSAlert alloc] init] autorelease];
			[alert addButtonWithTitle:@"OK"];
			[alert setMessageText:message];
			[alert setAlertStyle:NSInformationalAlertStyle];
			[alert beginSheetModalForWindow:fWindow
							  modalDelegate:nil
							 didEndSelector:nil
								contextInfo:nil];
		}
		
		fState = kDialogControllerModal;
		response = (int) [[NSApplication sharedApplication] runModalForWindow:fWindow];

		// Dismiss window
		[fWindow close];

		fState = kDialogControllerNormal;
	}

	return response;
}

-(void)stopModalWithCode:(NSInteger)code
{
	[[NSApplication sharedApplication] stopModalWithCode:code];
}

-(IBAction)close:(id)sender
{
	[self stopModalWithCode:kActionClose];
}

-(void)beginSheet:(NSWindow*)parent modalDelegate:(id)delegate contextInfo:(void*)contextInfo
{
	if ( Rtt_VERIFY( kDialogControllerNormal == fState ) )
	{
		fState = kDialogControllerSheet;
		fSheetDelegate = delegate;

		// When the sheet ends, we call self's didEndSelector which calls fSheetDelegate
		// and then resets the state of the dialog controller
		SEL didEndSelector = @selector(sheetDidEnd:returnCode:contextInfo:);
		[NSApp beginSheet:fWindow modalForWindow:parent modalDelegate:self didEndSelector:didEndSelector contextInfo:contextInfo];
        [NSApp runModalForWindow:fWindow];
	}
}

-(void)sheetDidEnd:(NSWindow*)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	if ( fSheetDelegate && [fSheetDelegate respondsToSelector:@selector(sheetDidEnd:returnCode:contextInfo:)] )
	{
		[fSheetDelegate sheetDidEnd:sheet returnCode:returnCode contextInfo:contextInfo];
	}

    [self stopModalWithCode:kActionDefault];
	fSheetDelegate = nil;
	fState = kDialogControllerNormal;
}

-(BOOL)isActionEnabled:(NSInteger)code
{
	return ( ! fDelegate || ! [fDelegate respondsToSelector:@selector(isActionEnabled:forCode:)] ? YES : [fDelegate isActionEnabled:self forCode:code] );
}

-(BOOL)enabledDefault
{
	return [self isActionEnabled:kActionDefault];
}

-(BOOL)enabledAlternate
{
	return [self isActionEnabled:kActionAlternate];
}

-(BOOL)enabledOther1
{
	return [self isActionEnabled:kActionOther1];
}

-(void)handleAction:(id)sender withCode:(NSInteger)code
{
	if ( ! fDelegate || [fDelegate shouldStopModal:self withCode:code] )
	{
		if ( kDialogControllerModal == fState )
		{
			[[NSApplication sharedApplication] stopModalWithCode:code];
		}
		else
		{
			Rtt_ASSERT( kDialogControllerSheet == fState );
			[NSApp endSheet:fWindow returnCode:code];
			[fWindow orderOut:self];

			fState = kDialogControllerNormal;
		}		
	}
}

-(IBAction)actionDefault:(id)sender
{
	[self handleAction:sender withCode:kActionDefault];
}

-(IBAction)actionAlternate:(id)sender
{
	[self handleAction:sender withCode:kActionAlternate];
}

-(IBAction)actionOther1:(id)sender
{
	[self handleAction:sender withCode:kActionOther1];
}

-(IBAction)actionOther2:(id)sender
{
	[self handleAction:sender withCode:kActionOther2];
}

-(BOOL)validateValue:(id *)ioValue forKey:(NSString *)key error:(NSError **)outError
{
	if ( ! fDelegate || ! [fDelegate respondsToSelector:@selector(validateValue:forKey:error:)] )
	{
		return [super validateValue:ioValue forKey:key error:outError];
	}
	else
	{
		return [fDelegate validateValue:ioValue forKey:key error:outError];
	}
}

-(BOOL)validateValue:(id *)ioValue forKeyPath:(NSString *)inKeyPath error:(NSError **)outError
{
	if ( ! fDelegate || ! [fDelegate respondsToSelector:@selector(validateValue:forKeyPath:error:)] )
	{
		return [super validateValue:ioValue forKeyPath:inKeyPath error:outError];
	}
	else
	{
		return [fDelegate validateValue:ioValue forKeyPath:inKeyPath error:outError];
	}
}

@end

// ----------------------------------------------------------------------------

@interface LoginController : DialogController
{
}
@end


@implementation LoginController

+(NSSet*)keyPathsForValuesAffectingEnabledDefault
{
	return [NSSet setWithObjects:@"inputs.username", @"inputs.password", nil];
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacAuthorizationDelegate::MacAuthorizationDelegate()
:	fLicenseController( nil ),
	fLoginController( nil ),
	fLoginControllerDelegate( nil )
{
}

MacAuthorizationDelegate::~MacAuthorizationDelegate()
{
	[fLoginControllerDelegate release];
	[fLoginController release];
	[fLicenseController release];
}

int
MacAuthorizationDelegate::Authorize( Authorization *authorizer, const char *user, const char *encryptedPassword, NSWindow *parent ) const
{
	AppDelegate *d = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	if ( parent )
	{
		[d beginProgressSheet:parent];
	}

	int code = authorizer->Authorize( user, encryptedPassword, true );

	if ( parent )
	{
		[d endProgressSheet];
	}

	return code;
}

bool
MacAuthorizationDelegate::Reauthorize( Authorization *authorizer, DialogController *controller, id delegate ) const
{
	bool result = false;

	const AuthorizationTicket *ticket = authorizer->GetTicket();
	if ( Rtt_VERIFY( ticket ) )
	{
		NSWindow *window = controller.window; Rtt_ASSERT( window );
		const MPlatformServices& services = authorizer->GetServices();

		const char* usr = ticket->GetUsername();
		Rtt::String encryptedPassword;
		
		if ( Rtt_VERIFY( usr ) )
		{
			AppDelegate *d = (AppDelegate*)[[NSApplication sharedApplication] delegate];
			NSString *p = [d getAndReleaseResultFromPasswordSheet];
			if ( p )
			{
				// Grab pwd from fPasswordController
				const char * pwd = [p UTF8String];
				Rtt_ASSERT( pwd );
				// didPrompt = true;

				Rtt::Authorization::Encrypt( pwd, &encryptedPassword );
			}
			else
			{
				services.GetPreference( usr, &encryptedPassword  );

				if ( ! encryptedPassword.GetString() )
				{
					NSString *username = [[NSString alloc] initWithUTF8String:usr];
					[d beginPasswordSheetWithUser:username
								   modalForWindow:window
									modalDelegate:delegate
										  message:@"Please enter your password to continue."
									  contextInfo:controller];
					[username release];
				}
			}
		}

		if ( encryptedPassword.GetString() )
		{
			if ( ! services.IsInternetAvailable() )
			{
				NSDictionary *details = [[NSDictionary alloc] initWithObjectsAndKeys:@"Could not connect to server. Please check your Internet connection.", NSLocalizedDescriptionKey, nil];
				NSError *error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:501 userInfo:details];
				[NSApp presentError:error modalForWindow:window delegate:nil didPresentSelector:nil contextInfo:nil];
				[error release];
				[details release];
			}
			else
			{
				// TODO: What if no pwd or pwd bogus
				int code = Authorize( authorizer, usr, encryptedPassword.GetString(), controller.window );

				if ( Authorization::kAuthorizationReceivedTicket == code )
				{
					ticket = authorizer->GetTicket();
					if ( ticket && ticket->IsTrial() )
					{
						code = Authorization::kAuthorizationTrialOnly;
					}
				}

				// Handle errors
				if ( Authorization::kAuthorizationReceivedTicket >= code )
				{
					const char *url = Authorization::kUrlPurchase;
					NSString *msg = nil;
					NSString *defaultButton = nil;
					NSString *alternateButton = @"Quit";
					NSAlertStyle alertStyle = NSCriticalAlertStyle;

					switch( code )
					{
						case Authorization::kAuthorizationReceivedTicket:
							if ( ticket->GetNumDaysLeft() > 10 )
							{
								msg = NSLocalizedString( @"Thank you for your purchase! The purchased version of Corona is now ready for use on this computer.", nil );
								defaultButton = NSLocalizedString( @"Continue", nil );
								url = NULL;
							}
							else
							{
								msg = NSLocalizedString( @"Could not verify your renewal. Please check that your purchase was completed and try again.", nil );
								defaultButton = NSLocalizedString( @"OK", nil );
								url = kDoNotStopModal;
							}

							alternateButton = nil;
							alertStyle = NSInformationalAlertStyle;
							break;
						case Authorization::kAuthorizationQuotaLimitReached:
							msg = @"You are already using this software on two other computers. Please deauthorize one of your other computers before logging in on this computer.\n\nOn the other computer, go to Preferences… / Deauthorize and Quit";
							defaultButton = @"Purchase";
							break;
						case Authorization::kAuthorizationUnverifiedUserError:
							msg = @"Your account has not been verified yet. A verification email has been sent to you with further instructions on how to validate your account.";
							defaultButton = @"OK";
							alternateButton = nil;
							url = NULL;
							break;
						case Authorization::kAuthorizationPlatformDoesNotMatch:
							msg = @"You purchased a product for a different platform. You can click Purchase for an additional license for this computer.";
							defaultButton = @"Purchase";
							break;
						case Authorization::kAuthorizationApiKeyInvalid:
							url = Authorization::kUrlDownload;
							msg = @"This version is no longer supported. Please click Download to obtain a supported version of this product.";
							defaultButton = @"Download";
							break;
						case Authorization::kAuthorizationTokenExpired:
							msg = @"Your computer's clock has the incorrect date and/or time. Please update with the correct time and try again.";
							defaultButton = @"OK";
							alternateButton = nil;
							url = NULL;
							break;
						default:
							Rtt_ASSERT_NOT_REACHED();
							msg = [NSString stringWithFormat:@"An unknown error occurred: %d", code];
							defaultButton = @"OK";
							break;
					}

					NSAlert* alert = [[[NSAlert alloc] init] autorelease];
					[alert addButtonWithTitle:defaultButton];
					[alert addButtonWithTitle:alternateButton];
					[alert setMessageText:msg];
					[alert setAlertStyle:alertStyle];
					[alert beginSheetModalForWindow:window
							modalDelegate:delegate
							didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
							contextInfo:const_cast< char* >( url )];
				}
				else
				{
					NSDictionary* details = nil;
					NSError* error = nil;
					id del = nil;
					SEL sel = nil;
					void *ud = NULL;

					switch( code )
					{
						case Authorization::kAuthorizationLoginFailed:
							details = [[NSDictionary alloc] initWithObjectsAndKeys:@"Incorrect email and/or password.", NSLocalizedDescriptionKey, nil];
							error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:101 userInfo:details];
							services.SetPreference( usr, NULL );
							break;
						case Authorization::kAuthorizationPassword:
							details = [[NSDictionary alloc] initWithObjectsAndKeys:@"Login succeeded, but we were unable to store your Corona password in your Keychain. You may have to login again in the future.", NSLocalizedDescriptionKey, nil];
							error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:209 userInfo:details];
							break;
						case Authorization::kAuthorizationAgreementError:
							del = [NSApp delegate];
							sel = @selector(didPresentError:contextInfo:);
							ud = (void*)WebServicesSession::kAgreementError;
							details = [[NSDictionary alloc] initWithObjectsAndKeys:@"The Corona Connect terms and conditions have changed. Click OK to read and agree to the new terms.", NSLocalizedDescriptionKey, nil];
							error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:404 userInfo:details];
							break;
						case Authorization::kAuthorizationTrialOnly:
							details = [[NSDictionary alloc] initWithObjectsAndKeys:@"Our records show you have not purchased the product yet. The account status is currently set to a starter account.", NSLocalizedDescriptionKey, nil];
							error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:201 userInfo:details];
							break;
						default:
							details = [[NSDictionary alloc] initWithObjectsAndKeys:@"Could not verify account information.", NSLocalizedDescriptionKey, nil];
							error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:201 userInfo:details];
							break;
					}

					[NSApp presentError:error modalForWindow:window delegate:del didPresentSelector:sel contextInfo:ud];

					[error release];
					[details release];
				}
			}
		}
	}

	return result;
}

bool
MacAuthorizationDelegate::Login( Authorization *authorizer ) const
{
	// Fetch login info from controller
	NSString *u = [fLoginController.inputs valueForKey:@"username"];
	NSString *p = [fLoginController.inputs valueForKey:@"password"];

	if ([u length] > kMaxUsernameLength)
	{
		u = [u substringToIndex:kMaxUsernameLength];
	}
	if ([p length] > kMaxPasswordLength)
	{
		p = [p substringToIndex:kMaxPasswordLength];
	}

	const char *user = [u UTF8String]; Rtt_ASSERT( user );
	const char *pass = [p UTF8String]; Rtt_ASSERT( pass );

	Rtt::String encryptedPassword;
	Rtt::Authorization::Encrypt( pass, &encryptedPassword );

	int code = Authorize( authorizer, user, encryptedPassword.GetString(), fLoginController.window );

	bool result = false;

	// Handle errors
	if ( Authorization::kAuthorizationReceivedTicket >= code )
	{
		const char *url = Authorization::kUrlPurchase;
		NSString *msg = nil;
		NSString *defaultButton = nil;
		NSString *alternateButton = @"Quit";
		NSAlertStyle alertStyle = NSCriticalAlertStyle;

		switch( code )
		{
			case Authorization::kAuthorizationReceivedTicket:
				msg = @"Congratulations! The Corona is now ready for use on this computer.";
				defaultButton = @"Continue";
				alternateButton = nil;
				url = NULL;
				alertStyle = NSInformationalAlertStyle;

				// Let delegate dismiss modal dialog, so we can show the informational alert
				fLoginControllerDelegate.alertViewStopModalCode = kActionDefault;
				//result = true;
				break;
			case Authorization::kAuthorizationDeviceBlacklisted:
				msg = @"This computer was already used for a trial. Support our efforts to make great products by clicking Purchase.";
				defaultButton = @"Purchase";
				break;
			case Authorization::kAuthorizationUnverifiedUserError:
				msg = @"Your account has not been verified yet. A verification email has been sent to you with further instructions on how to validate your account.";
				defaultButton = @"OK";
				alternateButton = nil;
				url = NULL;
				break;
			case Authorization::kAuthorizationTrialExpired:
				msg = @"Your trial has expired. To continue using, click Purchase and support our efforts to make great products.";
				defaultButton = @"Purchase";
				break;
			case Authorization::kAuthorizationBasicExpired:
				url = Authorization::kUrlRenew;
				msg = @"To use the latest updates, you must have an active subscription. Click Renew now to continue making great iPhone applications.";
				defaultButton = @"Renew";
				break;
			case Authorization::kAuthorizationQuotaLimitReached:
				msg = @"You are already using this software on two other computers. Please deauthorize one of your other computers before logging in on this computer.\n\nOn the other computer, go to Preferences… / Deauthorize and Quit";
				defaultButton = @"Purchase";
				break;
			case Authorization::kAuthorizationPlatformDoesNotMatch:
				msg = @"You purchased a product for a different platform. You can click Purchase for an additional license for this computer.";
				defaultButton = @"Purchase";
				break;
				break;
			case Authorization::kAuthorizationApiKeyInvalid:
				url = Authorization::kUrlDownload;
				msg = @"This version is no longer supported. Please click Download to obtain a supported version of this product.";
				defaultButton = @"Download";
				break;
			case Authorization::kAuthorizationTokenExpired:
				msg = @"Your computer's clock has the incorrect date and/or time. Please update with the correct time and try again.";
				defaultButton = @"OK";
				alternateButton = nil;
				url = NULL;
				break;
			default:
				Rtt_ASSERT_NOT_REACHED(); // If you hit this, then you are missing a case above
				msg = [NSString stringWithFormat:@"An unexpected account configuration error occurred.  Please contact Corona Labs customer service at support@coronalabs.com\n\nAccount: %@\nError: %d", u, code];
				defaultButton = @"OK";
				break;
		}

		NSAlert* alert = [[[NSAlert alloc] init] autorelease];
		[alert addButtonWithTitle:defaultButton];
		[alert addButtonWithTitle:alternateButton];
		[alert setMessageText:msg];
		[alert setAlertStyle:alertStyle];
		[alert beginSheetModalForWindow:fLoginController.window
				modalDelegate:fLoginControllerDelegate
				didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
				contextInfo:const_cast< char* >( url )];
	}
	else
	{
		NSDictionary* details = nil;
		NSError* error = nil;
		id del = nil;
		SEL sel = nil;
		void *ud = NULL;

		if ( Authorization::kAuthorizationLoginFailed == code )
		{
			details = [[NSDictionary alloc] initWithObjectsAndKeys:@"Incorrect email and/or password\n\nEnter the email address and password you registered with the Corona Developer Portal and try again (press the \"Register\" button if you need to create an account)", NSLocalizedDescriptionKey, nil];
			error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:101 userInfo:details];
			authorizer->GetServices().SetPreference( user, NULL );
		}
		else if ( Authorization::kAuthorizationPassword == code )
		{
			details = [[NSDictionary alloc] initWithObjectsAndKeys:@"Login succeeded, but we were unable to store your Corona password in your Keychain. You may have to login again in the future.", NSLocalizedDescriptionKey, nil];
			error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:209 userInfo:details];
		}
		else if ( Authorization::kAuthorizationAgreementError == code )
		{
			del = [NSApp delegate];
			sel = @selector(didPresentError:contextInfo:);
			ud = (void*)WebServicesSession::kAgreementError;
			details = [[NSDictionary alloc] initWithObjectsAndKeys:@"The Corona Connect terms and conditions have changed. Click OK to read and agree to the new terms.", NSLocalizedDescriptionKey, nil];
			error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:404 userInfo:details];
		}
		else
		{
			details = [[NSDictionary alloc] initWithObjectsAndKeys:@"Could not verify account information.", NSLocalizedDescriptionKey, nil];
			error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:201 userInfo:details];
		}

		[NSApp presentError:error modalForWindow:fLoginController.window delegate:del didPresentSelector:sel contextInfo:ud];

		[error release];
		[details release];
	}

	return result;
}

bool
MacAuthorizationDelegate::ShowLicense( const Authorization& sender )
{
	if ( ! fLicenseController )
	{
		fLicenseController = [[DialogController alloc] initWithNibNamed:@"License"];
	}

	bool result = ( kActionDefault == [fLicenseController runModal] );

	if ( ! result )
	{
		[[NSApplication sharedApplication] terminate:nil];
	}

	return result;
}

bool
MacAuthorizationDelegate::ShowTrial( const Authorization& sender, const AuthorizationTicket& ticket )
{
	// NOTE: This assumes that the controller blocks in a modal sesion
	AuthorizationContext c = { const_cast< Authorization* >( & sender ), const_cast< MacAuthorizationDelegate* >( this ) };

	TrialDelegate *delegate = [[TrialDelegate alloc] initWithPlatform:& sender.GetServices().Platform()];
	DialogController *controller = [[DialogController alloc] initWithNibNamed:@"Trial" delegate:delegate];
	controller.userdata = & c; // const_cast< MacAuthorizationDelegate* >( this );

	Rtt_ASSERT( ticket.IsTrial() );
	//time_t current = time( NULL );
	//time_t expiration = ticket.GetExpiration();
	//time_t kNumSecondsPerDay = 24*60*60;
	// ticket stores old expiration which is 30 days from the first day of trial
	//int numDaysSinceTrialBegan = 30 + (current - expiration)/kNumSecondsPerDay;

	NSString *msg = @"We hope you're having a blast with Corona! Upgrade to Pro and build apps with features like analytics and in-app purchases. Take your apps to the next level.";

	[controller.inputs setValue:NSLocalizedString(@"Corona", nil) forKey:@"windowTitle"];
	[controller.inputs setValue:NSLocalizedString(@"Upgrade Today", nil) forKey:@"title"];
	[controller.inputs setValue:msg forKey:@"message"];
	[controller.inputs setValue:NSLocalizedString(@"Buy Now", nil) forKey:@"buttonDefault"];
	[controller.inputs setValue:NSLocalizedString(@"Verify Purchase", nil) forKey:@"buttonAlternate"];
	[controller.inputs setValue:NSLocalizedString(@"Continue", nil) forKey:@"buttonOther1"];


	// Trial dialog only disappears when user clicks "Continue" or "Already Purchased"
	// and in the latter case, the reauth must be successful for the dialog to dismiss.
	NSInteger code = [controller runModal];
	bool result = Rtt_VERIFY( kActionDefault != code );

	[controller release];
	[delegate release];

	return result;
}

bool
MacAuthorizationDelegate::ShowTrialBuild( const Authorization& sender, const AuthorizationTicket& ticket )
{
	// NOTE: This assumes that the controller blocks in a modal sesion
	AuthorizationContext c = { const_cast< Authorization* >( & sender ), const_cast< MacAuthorizationDelegate* >( this ) };

	TrialDelegate *delegate = [[TrialDelegate alloc] initWithPlatform:& sender.GetServices().Platform()];
	DialogController *controller = [[DialogController alloc] initWithNibNamed:@"Trial" delegate:delegate];
	controller.userdata = & c; // const_cast< MacAuthorizationDelegate* >( this );

	NSString *msgPrefix = NSLocalizedString( @"Do you want to build more powerful apps? Upgrade to Pro and get access to Daily Builds, analytics, in-app purchases and more.", nil );
	
	NSString *msg = [[NSString alloc] initWithString:msgPrefix];

	[controller.inputs setValue:NSLocalizedString(@"Corona", nil) forKey:@"windowTitle"];
	[controller.inputs setValue:NSLocalizedString(@"Upgrade Today", nil) forKey:@"title"];
	[controller.inputs setValue:msg forKey:@"message"];
	[controller.inputs setValue:NSLocalizedString(@"Buy Now", nil) forKey:@"buttonDefault"];
	[controller.inputs setValue:NSLocalizedString(@"Verify Purchase", nil) forKey:@"buttonAlternate"];
	[controller.inputs setValue:NSLocalizedString(@"Continue", nil) forKey:@"buttonOther1"];

	[msg release];

	// Trial dialog only disappears when user clicks "Continue" or "Already Purchased"
	// and in the latter case, the reauth must be successful for the dialog to dismiss.
	NSInteger code = [controller runModal];
	bool result = Rtt_VERIFY( kActionDefault != code );

	[controller release];
	[delegate release];

	return result;
}

bool
MacAuthorizationDelegate::ShowDeviceBuildUpsell( const Authorization& sender, const AuthorizationTicket& ticket, TargetDevice::Platform platform )
{
	// NOTE: This assumes that the controller blocks in a modal sesion
	AuthorizationContext c = { const_cast< Authorization* >( & sender ), const_cast< MacAuthorizationDelegate* >( this ) };

	TrialDelegate *delegate = [[TrialDelegate alloc] initWithPlatform:& sender.GetServices().Platform()];
	[delegate setUrl:Rtt::Authorization::kUrlUpgradeToPro];
	DialogController *controller = [[DialogController alloc] initWithNibNamed:@"Trial" delegate:delegate];
	controller.userdata = & c; // const_cast< MacAuthorizationDelegate* >( this );

/*
	time_t current = time( NULL );
	time_t expiration = ticket.GetExpiration();
	time_t kNumSecondsPerDay = 24*60*60;
	int numDaysLeft = ( expiration > current ? (expiration - current)/kNumSecondsPerDay : 0 );
*/
	const char *desiredPlatform = TargetDevice::StringForPlatform( platform );

	NSString *format = NSLocalizedString( @"Your subscription restricts device builds on %@ to Trial mode.", nil );
	NSString *msgPrefix = [[NSString alloc] initWithFormat:format, [NSString stringWithExternalString:desiredPlatform] ];
	NSString *msgSuffix = NSLocalizedString(@"Upgrade your subscription to get access to priority device builds.", nil );
	NSString *msg = [[NSString alloc] initWithFormat:@"%@ %@", msgPrefix, msgSuffix];

	[controller.inputs setValue:NSLocalizedString(@"Corona", nil) forKey:@"windowTitle"];
	[controller.inputs setValue:NSLocalizedString(@"Upgrade Today", nil) forKey:@"title"];
	[controller.inputs setValue:msg forKey:@"message"];
	[controller.inputs setValue:NSLocalizedString(@"Upgrade", nil) forKey:@"buttonDefault"];
	[controller.inputs setValue:NSLocalizedString(@"Verify Purchase", nil) forKey:@"buttonAlternate"];
	[controller.inputs setValue:NSLocalizedString(@"Continue", nil) forKey:@"buttonOther1"];

	[msg release];
	[msgPrefix release];

	// Trial dialog only disappears when user clicks "Continue" or "Already Renewed"
	// and in the latter case, the reauth must be successful for the dialog to dismiss.
	NSInteger code = [controller runModal];
	bool result = Rtt_VERIFY( kActionDefault != code );


	[controller release];
	[delegate release];

	return result;
}

bool
MacAuthorizationDelegate::ShowTrialUpsell( const Authorization& sender, const AuthorizationTicket& ticket, const char *message )
{
	// NOTE: This assumes that the controller blocks in a modal sesion
	AuthorizationContext c = { const_cast< Authorization* >( & sender ), const_cast< MacAuthorizationDelegate* >( this ) };

	TrialDelegate *delegate = [[TrialDelegate alloc] initWithPlatform:& sender.GetServices().Platform()];
	[delegate setUrl:Rtt::Authorization::kUrlPurchase];
	DialogController *controller = [[DialogController alloc] initWithNibNamed:@"Trial" delegate:delegate];
	controller.userdata = & c; // const_cast< MacAuthorizationDelegate* >( this );

	bool isTrial = ticket.IsTrial();

	NSString *msgPrefix = [NSString stringWithExternalString:message];
	NSString *msgSuffix = NSLocalizedString( @"Buy a subscription to get access to full functionality.", nil );
	NSString *title = NSLocalizedString(@"Upgrade Today", nil);
	NSString *buttonDefault = NSLocalizedString(@"Buy Now", nil);
	NSString *windowTitle = NSLocalizedString(@"Corona", nil);

	if ( ! isTrial )
	{
		msgSuffix = NSLocalizedString( @"Upgrade your subscription to get access to full functionality.", nil );
		title = NSLocalizedString(@"Upgrade Today", nil);
		buttonDefault = NSLocalizedString(@"Upgrade", nil);
		[delegate setUrl:Rtt::Authorization::kUrlUpgradeToPro];
		windowTitle = NSLocalizedString(@"Upgrade Corona", nil);
	}

	NSString *msg = [NSString stringWithFormat:@"%@ %@", msgPrefix, msgSuffix];

	[controller.inputs setValue:windowTitle forKey:@"windowTitle"];
	[controller.inputs setValue:title forKey:@"title"];
	[controller.inputs setValue:msg forKey:@"message"];
	[controller.inputs setValue:buttonDefault forKey:@"buttonDefault"];
	[controller.inputs setValue:NSLocalizedString(@"Verify Purchase", nil) forKey:@"buttonAlternate"];
	[controller.inputs setValue:NSLocalizedString( @"OK", nil ) forKey:@"buttonOther1"];

	// Trial dialog only disappears when user clicks "Continue" or "Already Renewed"
	// and in the latter case, the reauth must be successful for the dialog to dismiss.
	NSInteger code = [controller runModal];
	bool result = Rtt_VERIFY( kActionDefault != code );


	[controller release];
	[delegate release];

	return result;
}

void
MacAuthorizationDelegate::ShowRenewal( const Authorization& sender, const AuthorizationTicket& ticket )
{
	float numDaysLeft = ticket.GetNumDaysLeft();

	if ( numDaysLeft < 10 )
	{
		bool shouldRemind = true;
		Rtt::String tStr;

		time_t current = time( NULL );
        
		sender.GetServices().GetPreference( Authorization::kRenewalReminderKey, &tStr );
		if ( tStr.GetString() )
		{
			const time_t kNumSecondsPerDay = 24*60*60;
			long t = strtol( tStr.GetString(), NULL, 16 );
			if ( t > 0 && t < current )
			{
				time_t dt = current - t;
				shouldRemind = ( dt > kNumSecondsPerDay*1 ); // Every 1 days
			}
		}

		if ( shouldRemind && Rtt_VERIFY( ticket.IsPaidSubscription() ) )
		{
			// NOTE: This assumes that the controller blocks in a modal sesion
			AuthorizationContext c = { const_cast< Authorization* >( & sender ), const_cast< MacAuthorizationDelegate* >( this ) };

			TrialDelegate *delegate = [[TrialDelegate alloc] initWithPlatform:& sender.GetServices().Platform()];
			DialogController *controller = [[DialogController alloc] initWithNibNamed:@"Trial" delegate:delegate];
			controller.userdata = & c; // const_cast< MacAuthorizationDelegate* >( this );

			NSString *msgPrefix;
			if ( numDaysLeft > 0 )
			{
				if ( numDaysLeft > 1 )
				{
					NSString *prefixFormat = NSLocalizedString( @"Your subscription will expire in %d days.", nil );
					msgPrefix = [NSString stringWithFormat:prefixFormat, (int)numDaysLeft];
				}
				else
				{
					msgPrefix = NSLocalizedString( @"Your subscription is about to expire.", nil );
				}
			}
			else
			{
				msgPrefix = NSLocalizedString( @"Your subscription has expired.", nil );
			}

			NSString *msgSuffix = NSLocalizedString( @"Renew today so you can access all the great features of Corona.", nil );
			NSString *format = @"%@ %@";
			NSString *msg = [[NSString alloc] initWithFormat:format, msgPrefix, msgSuffix];
			NSString *title = NSLocalizedString( @"Renew today!", nil );

			[controller.inputs setValue:NSLocalizedString( @"Corona Expiration", nil ) forKey:@"windowTitle"];
			[controller.inputs setValue:title forKey:@"title"];
			[controller.inputs setValue:msg forKey:@"message"];
			[controller.inputs setValue:@"Renew Now" forKey:@"buttonDefault"];
			[controller.inputs setValue:@"Verify Renewal" forKey:@"buttonAlternate"];
			[controller.inputs setValue:@"Remind Me Later" forKey:@"buttonOther1"];

			// Trial dialog only disappears when user clicks "Remind Me Later" or "Already Renewed"
			// and in the latter case, the reauth must be successful for the dialog to dismiss.
			NSInteger code = [controller runModal];
			Rtt_ASSERT( kActionDefault != code );

			if ( kActionAlternate == code )
			{
				sender.GetServices().SetPreference( Authorization::kRenewalReminderKey, NULL );
			}
			else if ( kActionOther1 == code )
			{
				char timestamp[32];
				sprintf( timestamp, "%x", (unsigned int) current );
				sender.GetServices().SetPreference( Authorization::kRenewalReminderKey, timestamp );
			}

			[msg release];

			[controller release];
			[delegate release];
		}
	}
}

bool
MacAuthorizationDelegate::TicketNotInstalled( const Authorization& sender )
{
	bool result = false;

	if ( sender.GetServices().IsInternetAvailable() )
	{
		// Show login dialog
		if ( ! fLoginController )
		{
			Rtt_ASSERT( ! fLoginControllerDelegate );
			fLoginControllerDelegate = [[LoginDelegate alloc] initWithPlatform:& sender.GetServices().Platform()];
			fLoginController = [[LoginController alloc] initWithNibNamed:@"Login" delegate:fLoginControllerDelegate];
			// fLoginController.userdata = const_cast< MacAuthorizationDelegate* >( this );
			//[fLoginController.window setDelegate:fLoginControllerDelegate];
		}

		// NOTE: This assumes that the controller blocks in a modal sesion
		AuthorizationContext c = { const_cast< Authorization* >( & sender ), const_cast< MacAuthorizationDelegate* >( this ) };
		fLoginController.userdata = & c;

		// Login dialog only disappears after a successful login (or the user cancels). If they
		// choose register or the help button, the dialog will not disappear.
		// When a login is attempted, MacAuthorizationDelegate::Login() is called
		// while the modal dialog is still running.
		result = Rtt_VERIFY( kActionDefault == [fLoginController runModalWithMessage:[NSString stringWithExternalString:sender.GetLoginMessage()]] );
	}

	return result;
}

const char*
MacAuthorizationDelegate::PasswordInvalid( const Authorization& sender, const char *usr )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
	NSString *username = [[NSString alloc] initWithUTF8String:usr];
	NSString *pwd = nil; //[[[NSApplication sharedApplication] delegate] promptPasswordForUser:username];
	[username release];
	return [pwd UTF8String];
}

/*
void
MacAuthorizationDelegate::TicketInvalid( const Authorization& sender ) const
{
	NSRunAlertPanel( @"", msgFormat, @"Quit", nil, nil )
	NSApplication* application = [NSApplication sharedApplication];
	[application terminate:application];
}
*/

bool
MacAuthorizationDelegate::AuthorizationDidFail( const Authorization& sender ) const
{
	return false;
}

bool
MacAuthorizationDelegate::VerificationDidFail( const Authorization& sender ) const
{
#if 1
	bool result = false;

	const AuthorizationTicket *ticket = sender.GetTicket();

	if ( ! ticket )
	{
		// We should never reach this point.  Something is seriously broken upstream
		// if we do not have a ticket when this method is called!!!
		Rtt_ASSERT_NOT_REACHED();
		result = const_cast< MacAuthorizationDelegate* >( this )->TicketNotInstalled( sender );
		if ( ! result )
		{
			NSRunAlertPanel( @"This application has been corrupted.", @"Please download and reinstall.", @"Quit", nil, nil );
		}
	}
	else
	{
		// NOTE: This assumes that the controller blocks in a modal sesion
		AuthorizationContext c = { const_cast< Authorization* >( & sender ), const_cast< MacAuthorizationDelegate* >( this ) };

		TrialDelegate *delegate = [[TrialDelegate alloc] initWithPlatform:& sender.GetServices().Platform()];
		DialogController *controller = [[DialogController alloc] initWithNibNamed:@"Trial" delegate:delegate];
		controller.userdata = & c; // const_cast< MacAuthorizationDelegate* >( this );

		NSString *windowTitle = nil;
		NSString *title = nil;
		NSString *message = nil;
		NSString *buttonDefault = nil;
		NSString *buttonAlternate = nil;
		NSString *buttonOther1 = @"Quit";
		const char *url = nil;

		//This is no longer needed
		if ( ticket->IsTrial() )
		{
			windowTitle = @"Corona Expired";
			title = @"Your trial evaluation has expired";
			message = @"To continue using, please click Buy Now to purchase the Corona.";
			buttonDefault = @"Buy Now";
			buttonAlternate = @"Already Purchased";
			url = Authorization::kUrlPurchase;
		}
		else
		{
			windowTitle = @"Corona Subscription Expired";
			title = @"You cannot use the latest updates without an active subscription.";
			message = @"To qualify for the latest updates, you need to renew your subscription.";
			buttonDefault = @"Renew";
			buttonAlternate = @"Already Renewed";
			url = Authorization::kUrlRenew;
		}

		[controller.inputs setValue:windowTitle forKey:@"windowTitle"];
		[controller.inputs setValue:title forKey:@"title"];
		[controller.inputs setValue:message forKey:@"message"];
		[controller.inputs setValue:buttonDefault forKey:@"buttonDefault"];
		[controller.inputs setValue:buttonAlternate forKey:@"buttonAlternate"];
		[controller.inputs setValue:buttonOther1 forKey:@"buttonOther1"];

		// Dialog only disappears when user clicks "Quit" or "Already Purchased"/"Already Renewed"
		// and in the latter case, the reauth must be successful for the dialog to dismiss.
		NSInteger code = [controller runModal]; Rtt_ASSERT( kActionDefault != code );

		// Here, we further stipulate that the user must have clicked on "Already Purchased/Renewed"
		// to return to the caller; otherwise, we quit.
		result = kActionAlternate == code;

		[controller release];
		[delegate release];
	}

	if ( ! result )
	{
		NSApplication* application = [NSApplication sharedApplication];
		[application terminate:application];
	}

	return result;


#else

	NSString *title = nil;
	NSString *msgFormat = nil;
	NSString *defaultButton = nil;
	NSString *alternateButton = nil;
	const char *url = nil;

	const AuthorizationTicket *ticket = sender.GetTicket();

	if ( ! ticket )
	{
		Rtt_ASSERT_NOT_REACHED();
		title = @"This application has been corrupted.";
		msgFormat = @"Please download and reinstall.";
		defaultButton = @"Quit";
	}
	else
	{
		// Two cases --- see logic of AuthorizationTicket::IsAppAllowedToRun()
		// If it's trial, then we failed b/c trial expired
		// If it's not a trial, then we failed b/c updates only work up until 
		// expiration of the ticket.
		if ( ticket->IsTrial() )
		{
			title = @"Your trial evaluation has expired.";
			msgFormat = @"To continue using, please click Buy Now to purchase the Corona.";
			defaultButton = @"Buy Now";
			alternateButton = @"Cancel";
			url = Authorization::kUrlPurchase;
		}
		else
		{
			title = @"Your license does not extend to this version.";
			msgFormat = @"To qualify for the latest updates, you need to renew your subscription.";
			defaultButton = @"Renew";
			alternateButton = @"Cancel";
			url = Authorization::kUrlRenew;
		}
	}

	bool result = ( NSAlertDefaultReturn == NSRunAlertPanel( title, msgFormat, defaultButton, alternateButton, nil ) );

	if ( result )
	{
		sender.GetServices().Platform().OpenURL( url );

		if ( ticket->IsTrial() )
		{
			title = @"After you have completed your purchase, press Continue.";
		}
		else
		{
			title = @"After you have completed your renewal, press Continue.";
		}

		msgFormat = @"You will be prompted to login using your Corona Developer email and password.";
		defaultButton = @"Continue";
		alternateButton = @"Remind Me Later";

		result = ( NSAlertDefaultReturn == NSRunAlertPanel( title, msgFormat, defaultButton, alternateButton, nil ) );

		if ( result )
		{
			result = const_cast< MacAuthorizationDelegate* >( this )->TicketNotInstalled( sender );

			if ( ! result )
			{
				NSRunAlertPanel( @"Could not authorize this computer to use Corona Simulator", @"Please check your Internet connection.", nil, nil, nil );
			}
		}
	}

	if ( ! result )
	{
		NSApplication* application = [NSApplication sharedApplication];
		[application terminate:application];
	}

	return result;
#endif
}

// TODO: Unused???
void
MacAuthorizationDelegate::ShowLoginSheet( const Authorization& sender, NSWindow *parent )
{
	[fLoginController beginSheet:parent modalDelegate:fLoginControllerDelegate contextInfo:fLoginController];
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

