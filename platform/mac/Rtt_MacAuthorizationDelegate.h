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

#ifndef _Rtt_MacAuthorizationDelegate_H__
#define _Rtt_MacAuthorizationDelegate_H__

#include "Rtt_MAuthorizationDelegate.h"

#import <Foundation/NSObject.h>
#import <AppKit/NSNibDeclarations.h>

// ----------------------------------------------------------------------------

@class DialogController;
@class LoginDelegate;
@class NSButton;
@class NSMutableDictionary;
@class NSWindow;

// ----------------------------------------------------------------------------

enum DialogControllerActionCodes
{
	kActionClose = -2,
	kActionUnknown = -1,
	kActionDefault = 0,
	kActionAlternate,
	kActionOther1,
	kActionOther2,

	kNumActions
};

// ----------------------------------------------------------------------------

@protocol DialogControllerDelegate

-(BOOL)shouldStopModal:(DialogController*)sender withCode:(NSInteger)code;

@optional
-(BOOL)isActionEnabled:(DialogController*)sender forCode:(NSInteger)code;

// Optional portion
// -(BOOL)validateValue:(id *)ioValue forKey:(NSString *)key error:(NSError **)outError
// -(BOOL)validateValue:(id *)ioValue forKeyPath:(NSString *)inKeyPath error:(NSError **)outError

@end

// ----------------------------------------------------------------------------

enum DialogControllerState
{
	kDialogControllerNormal = 0, // No interaction
	kDialogControllerModal, // Running as a modal interaction
	kDialogControllerSheet, // Running as a sheet

	kDialogControllerNumStates
};

@interface DialogController : NSObject
{
	id fDelegate;
	id fSheetDelegate;
	NSMutableDictionary *inputs;
	void *userdata;

	IBOutlet NSWindow *fWindow;
	IBOutlet NSButton *fDefault;
	IBOutlet NSButton *fAlternate;
	IBOutlet NSButton *fOther1;
	IBOutlet NSButton *fOther2;

	U32 fState;
}

@property (nonatomic, readonly) NSMutableDictionary *inputs;
@property (nonatomic, readwrite, assign) void *userdata;
@property (nonatomic, readonly, getter=window) NSWindow *fWindow;

-(id)initWithNibNamed:(NSString*)name;
-(id)initWithNibNamed:(NSString*)name delegate:(id<DialogControllerDelegate>)delegate;

// Modal interaction
-(NSInteger)runModal;
-(NSInteger)runModalWithMessage:(NSString *)message;
-(void)stopModalWithCode:(NSInteger)code;

-(IBAction)close:(id)sender;

// Sheet interaction
// delegate should respond to the selector with signature:
//		-(void)sheetDidEnd:(NSWindow*)sheet
//				returnCode:(NSInteger)returnCode
//				contextInfo:(void *)contextInfo
//
-(void)beginSheet:(NSWindow*)parent modalDelegate:(id)delegate contextInfo:(void*)contextInfo;

-(BOOL)enabledDefault;
-(BOOL)enabledAlternate;
-(BOOL)enabledOther1;

-(IBAction)actionDefault:(id)sender;
-(IBAction)actionAlternate:(id)sender;
-(IBAction)actionOther1:(id)sender;
-(IBAction)actionOther2:(id)sender;

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacAuthorizationDelegate : public MAuthorizationDelegate
{
	public:
		MacAuthorizationDelegate();
		~MacAuthorizationDelegate();

	protected:
		int Authorize( Authorization *authorizer, const char *user, const char *encryptedPassword, NSWindow *parent ) const;

	public:
		bool Reauthorize( Authorization *authorizer, DialogController *controller, id delegate ) const;
		bool Login( Authorization *authorizer ) const;

	public:
		virtual bool ShowLicense( const Authorization& sender );
		virtual bool ShowTrial( const Authorization& sender, const AuthorizationTicket& ticket );
		virtual bool ShowTrialBuild( const Authorization& sender, const AuthorizationTicket& ticket );
		virtual bool ShowDeviceBuildUpsell( const Authorization& sender, const AuthorizationTicket& ticket, TargetDevice::Platform platform );
		virtual bool ShowTrialUpsell( const Authorization& sender, const AuthorizationTicket& ticket, const char *message );

		virtual void ShowRenewal( const Authorization& sender, const AuthorizationTicket& ticket );

		virtual bool TicketNotInstalled( const Authorization& sender );

		virtual const char* PasswordInvalid( const Authorization& sender, const char *usr );

		virtual bool AuthorizationDidFail( const Authorization& sender ) const;
		virtual bool VerificationDidFail( const Authorization& sender ) const;

	public:
		void ShowLoginSheet( const Authorization& sender, NSWindow *parent );

	private:
		DialogController *fLicenseController;
		DialogController *fLoginController;
		LoginDelegate *fLoginControllerDelegate;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacAuthorizationDelegate_H__
