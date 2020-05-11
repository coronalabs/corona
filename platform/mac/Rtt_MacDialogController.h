//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacDialogControllerDelegate_H__
#define _Rtt_MacDialogControllerDelegate_H__

#import <Foundation/NSObject.h>
#import <AppKit/NSNibDeclarations.h>

// ----------------------------------------------------------------------------

@class DialogController;
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

#endif // _Rtt_MacDialogControllerDelegate_H__
