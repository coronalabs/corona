//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/NSObject.h>
#import <AppKit/NSNibDeclarations.h>

// ----------------------------------------------------------------------------

@class NSPopUpButton;
@class NSString;
@class NSWindow;

namespace Rtt
{
	class MacSimulator;
}

@interface IPhoneBuild : NSObject
{
	IBOutlet NSWindow* fBuildWindow;
	IBOutlet NSPopUpButton* fSigningIdentities;
	NSString* appName;
	NSString* appVersion;
	NSString* username;
	NSString* password;
}

@property (nonatomic, readwrite, assign) NSString *appName;
@property (nonatomic, readwrite, assign) NSString *appVersion;
@property (nonatomic, readwrite, assign) NSString *username;
@property (nonatomic, readwrite, assign) NSString *password;

-(IBAction)openForBuild:(id)sender;
-(IBAction)build:(id)sender;
-(IBAction)cancelBuild:(id)sender;

@end
