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

#import "ValidationSupportMacUI.h"
#import "ValidationToolOutputViewController.h"
#include "Rtt_AndroidSupportTools.h"

static NSString* kAndroidValidationLuaFileName = @"AndroidValidation.lu";


bool ValidationSupportMacUI_ValidateAndroidPackageName( const char* packagename, const char* filepath )
{
	lua_State* L = Rtt_AndroidSupportTools_NewLuaState( filepath );
	bool isvalid = Rtt_AndroidSupportTools_IsAndroidPackageName( L, packagename );
	Rtt_AndroidSupportTools_CloseLuaState(L);
	return isvalid;
}


bool ValidationSupportMacUI_ValidateIOSAppName( const char* name, const char* filepath )
{
#ifdef RTT_VALIDATE_NAMES
	lua_State* L = Rtt_AndroidSupportTools_NewLuaState( filepath );
	bool isvalid = Rtt_CommonSupportTools_ValidateNameForRestrictedASCIICharacters( L, name );
	Rtt_AndroidSupportTools_CloseLuaState(L);
	return isvalid;
#else
    return true;
#endif // RTT_VALIDATE_NAMES
}

@interface ValidationSupportMacUI ()
- (void) displayAlertForFileValidationFailure:(NSString*)alertmessage messageComment:(NSString*)messagecomment informativeText:(NSString*)informativetext informativeComment:(NSString*)informativecomment fileList:(NSArray*)filelist;
@end

@implementation ValidationSupportMacUI

@synthesize parentWindow;

- (id) init
{
	self = [super init];
	if(nil != self)
	{
		NSString* script = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:kAndroidValidationLuaFileName];
		luaState = Rtt_AndroidSupportTools_NewLuaState( [script fileSystemRepresentation] );
	}
	return self;
}

- (id) initWithParentWindow:(NSWindow*)window
{
	self = [super init];
	if(nil != self)
	{
		NSString* script = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:kAndroidValidationLuaFileName];
		luaState = Rtt_AndroidSupportTools_NewLuaState( [script fileSystemRepresentation] );
		parentWindow = [window retain];
	}
	return self;
}

- (void) dealloc
{
	[parentWindow release];
	Rtt_AndroidSupportTools_CloseLuaState( luaState );
	[super dealloc];
}

- (void) finalize
{
	Rtt_AndroidSupportTools_CloseLuaState( luaState );
	[super finalize];
}

-(void) alertDidEndForAndroidFileValidation:(NSAlert*)alert returnCode:(int)returncode contextInfo:(void*)contextinofo
{
	NSArray* filelist = (NSArray*)contextinofo;
	
	if ( NSAlertSecondButtonReturn == returncode )
	{
		NSMutableArray* urllist = [NSMutableArray array];
		for(NSString* file in filelist)
		{
			NSURL* url = [NSURL fileURLWithPath:file isDirectory:NO];
			[urllist addObject:url];
		}
		[[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urllist];
	}
	else if ( NSAlertFirstButtonReturn == returncode )
	{
	}
	
	[filelist release];
}

- (void) displayAlertForFileValidationFailure:(NSString*)alertmessage messageComment:(NSString*)messagecomment informativeText:(NSString*)informativetext informativeComment:(NSString*)informativecomment fileList:(NSArray*)filelist
{
	// flatten the array into a string to be presented
	NSString* messagelist = [[filelist valueForKey:@"description"] componentsJoinedByString:@"\n"];
	NSAlert* alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:NSLocalizedString(@"Dismiss", @"Dismiss")];
	[alert addButtonWithTitle:NSLocalizedString(@"Show in Finder...", @"Show in Finder...")];
	
	[alert setMessageText:NSLocalizedString(alertmessage, messagecomment)];
	
	[alert setInformativeText:NSLocalizedString(informativetext, informativecomment)];
	
	// It so happens that the ValidationToolOutputViewController has the same UI interface we want.
	ValidationToolOutputViewController* validationToolViewController = [[ValidationToolOutputViewController alloc] initWithNibName:@"ValidationToolOutput" bundle:nil];
	[validationToolViewController autorelease];
	
	[validationToolViewController setValidationMessage:messagelist];
	[alert setAccessoryView:validationToolViewController.view];
	
	
	[alert setAlertStyle:NSCriticalAlertStyle];
	[alert setDelegate:self];
	
	[alert beginSheetModalForWindow:[self parentWindow] 
		modalDelegate:self
		didEndSelector:@selector(alertDidEndForAndroidFileValidation:returnCode:contextInfo:)
		contextInfo:[filelist retain]
	];
}

- (bool) runCommonFileValidationTestsInProjectPath:(NSString *)projectpath
{
	bool isvalid = true;
	
	// Validate Subdirectories for Lua files
#if 0
	lua_State* L = luaState;
    
	// Remember that this function leaves 2 elements on the Lua stack which will need to be cleared.
	isvalid = Rtt_CommonSupportTools_ValidateLuaFilesForForbiddenSubdirectories( L, [projectpath fileSystemRepresentation] );
	if ( ! isvalid )
	{
		// The file list array is on the top of the stack. Convert to an NSArray.
		NSArray* filelist = (NSArray*)LuaCocoa_ToPropertyList( L, -1);
		
		[self displayAlertForFileValidationFailure:@"CannontBuildBecauseOfLuaFilesInSubdirectoriesRules" 
			messageComment:@"The build failed because the project contains .lua files in subdirectories."
			informativeText:@"CannontBuildBecauseOfLuaFilesInSubdirectoriesRulesInformative" 
			informativeComment:@"Lua files must be in the project root directory. Please move your files and try again." 
			fileList:filelist
		 ];
		
	}
	// Restore the Lua stack
	lua_pop(L, 2);
#endif
	if( ! isvalid )
	{
		return false;
	}
	return true;
}

- (bool) runAndroidFileValidationTestsInProjectPath:(NSString *)projectpath
{
	// Currently, we don't have any more validation tests to run on the post-press-build-button step.
	return true;
}

- (bool) runIOSAppNameValidation:(NSString*)appname
{
	// Currently, we don't have any more validation tests to run on the post-press-build-button step.
	return true;
}

- (bool) runWebAppNameValidation:(NSString*)appname
{
	bool isvalid = false;
	
	// Validate Subdirectories for Lua files

	if ( [appname length] > 0 )
	{
		lua_State* L = luaState;

		isvalid = Rtt_CommonSupportTools_ValidateNameForRestrictedASCIICharacters( L, [appname UTF8String] );
	}

	if ( ! isvalid )
	{
		NSBeginAlertSheet(
			@"Application Name only supports standard characters.",
			nil,
			nil,
			nil,
			[self parentWindow],
			nil,
			nil,
			nil,
			nil,
			@"Please use only ASCII characters in your application name."
		);
	}

	return isvalid;
}

- (bool) runOSXAppNameValidation:(NSString*)appname
{
	// Currently, we don't have any more validation tests to run on the post-press-build-button step.
	return true;
}

@end
