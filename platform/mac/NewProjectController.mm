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

#import "NewProjectController.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_MSimulatorServices.h"
#include "Rtt_LuaLibSimulator.h"

#import "TextEditorSupport.h"

// ----------------------------------------------------------------------------

@implementation NewProjectController

@synthesize fWindow;
@synthesize fProjectName;
@synthesize fTemplateIndex;
@synthesize fScreenSizeIndex;
@synthesize fScreenWidth;
@synthesize fScreenHeight;
@synthesize fOrientationIndex;
@synthesize fProjectPath;
@synthesize fServices;

enum NewProjectControllerReturnCode
{
	kCancelled = 0,
	kNext
};

- (id)initWithWindowNibName:(NSString*)windowNibName resourcePath:(NSString*)path
{
	self = [super init];
	if ( self )
	{
		[NSBundle loadNibNamed:windowNibName owner:self];
		[fWindow setReleasedWhenClosed:NO];
		[fWindow close];

		[self addObserver:self forKeyPath:@"screenWidth" options:0 context:NULL];
		[self addObserver:self forKeyPath:@"screenHeight" options:0 context:NULL];
		[self addObserver:self forKeyPath:@"screenSizeIndex" options:0 context:NULL];
		[self addObserver:self forKeyPath:@"templateIndex" options:0 context:NULL];

		[self willChangeValueForKey:@"screenWidth"];
		fScreenWidth = [NSNumber numberWithInt:320];
		[self didChangeValueForKey:@"screenWidth"];

		[self willChangeValueForKey:@"screenHeight"];
		fScreenHeight = [NSNumber numberWithInt:480];
		[self didChangeValueForKey:@"screenHeight"];

		fResourcePath = [path retain];

		fParent = nil;
		fProjectPath = nil;
		fServices = NULL;
	}
	return self;
}

- (void)dealloc
{
	[fResourcePath release];
	[fWindow release];

	[super dealloc];
}

- (void)beginSheetModalForWindow:(NSWindow*)parent
{
//	NSWindow *window = [self window];
//	[self close];
	[NSApp beginSheet:fWindow
			modalForWindow:parent
			modalDelegate:self
			didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:)
			contextInfo:NULL];

	fParent = parent;
}

- (void)endSheet
{
	[fWindow orderOut:self];
	fParent = nil;
	fProjectPath = nil;
}

- (void)alertDidEnd:(NSAlert*)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	if ( fProjectPath )
	{
		if ( NSAlertSecondButtonReturn == returnCode )
		{
			// Reveal in Finder
			(void)Rtt_VERIFY( [[NSWorkspace sharedWorkspace] selectFile:fProjectPath inFileViewerRootedAtPath:@""] );
		}
		else
		{
			// Open in Editor
			NSString *path = [fProjectPath stringByAppendingPathComponent:@"main.lua"];
			TextEditorSupport_LaunchTextEditorWithFile( path, 0 );
		}

		if ( fServices && Rtt_VERIFY( fProjectPath ) )
		{
			fServices->OpenProject( [fProjectPath UTF8String] );
		}
	}
}

static NSButton*
FindButtonWithSelector( NSView *root, SEL sel )
{
	NSButton *result = nil;
	for ( NSView *child in [root subviews] )
	{
		if ( [child isKindOfClass:[NSButton class]] )
		{
			NSButton *button = (NSButton*)child;
			if ( [button action] == sel )
			{
				result = button;
			}
		}
		else
		{
			result = FindButtonWithSelector( child, sel );
		}

		if ( result )
		{
			break;
		}
	}

	return result;
}

- (NSString*)templateName
{
	enum {
		kBlankTemplateIndex = 0,
		kAppTemplateIndex,
		kGameTemplateIndex,
		kEbookTemplateIndex,
	};

	NSString *result = @"blank";

	switch( fTemplateIndex )
	{
		case kAppTemplateIndex:
			result = @"app";
			break;
		case kGameTemplateIndex:
			result = @"game";
			break;
		case kEbookTemplateIndex:
			result = @"ebook";
			break;
		default:
			break;
	}

	return result;
}

Rtt_EXPORT int luaopen_lfs (lua_State *L);

- (void)createProject
{
	lua_State *L = luaL_newstate();
	luaL_openlibs( L );

	Rtt::LuaContext::RegisterModuleLoader( L, "lfs", luaopen_lfs );
#if 0
    // Uncomment if Simulator services are required in newproject.lua
	lua_pushlightuserdata( L, (void *)fServices );
    Rtt::LuaContext::RegisterModuleLoader( L, "simulator", Rtt::LuaLibSimulator::Open, 1 );
#endif // 0

	NSString *script = [fResourcePath stringByAppendingPathComponent:@"newproject.lua"];
	int status = luaL_loadfile( L, [script UTF8String] ); Rtt_ASSERT( 0 == status );
	lua_createtable( L, 0, 6 );
	{
		lua_pushboolean( L, true );
		lua_setfield( L, -2, "isSimulator" );

		lua_pushstring( L, [[self templateName] UTF8String] );
		lua_setfield( L, -2, "template" );

		lua_pushinteger( L, [fScreenWidth intValue] );
		lua_setfield( L, -2, "width" );

		lua_pushinteger( L, [fScreenHeight intValue] );
		lua_setfield( L, -2, "height" );

		lua_pushstring( L, 0 == fOrientationIndex ? "portrait" : "landscapeRight" );
		lua_setfield( L, -2, "orientation" );

		lua_pushstring( L, [fProjectPath UTF8String] );
		lua_setfield( L, -2, "savePath" );

		lua_pushstring( L, [[fResourcePath stringByAppendingPathComponent:@"templates"] UTF8String] );
		lua_setfield( L, -2, "templateBaseDir" );
	}

	status = Rtt::LuaContext::DoCall( L, 1, 0 ); Rtt_ASSERT( 0 == status );
	lua_close( L );
}

- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	[self endSheet];
}

+(NSSet*)keyPathsForValuesAffectingFormComplete
{
	return [NSSet setWithObjects:@"projectName", nil];
}

- (BOOL)formComplete
{
	BOOL result = ( nil != fProjectName );
	return result;
}

static const int kPhonePresetWidth = 320;
static const int kPhonePresetHeight = 480;

static const int kTabletPresetWidth = 768;
static const int kTabletPresetHeight = 1024;

static const int kPhonePresetTag = 0;
static const int kTabletPresetTag = 1;
static const int kCustomPresetTag = 2;

- (void)observeValueForKeyPath:(NSString*)keyPath ofObject:(id)object change:(NSDictionary*)change context:(void*)context
{
	// TODO: Do not use this function as a model for your KV bindings.
	// There's something strange going on...
	if ( [keyPath isEqualToString:@"screenSizeIndex"] )
	{
		NSInteger tag = [[fScreenSize selectedItem] tag];
		int w = -1;
		int h = -1;
		switch ( tag )
		{
			case kPhonePresetTag:
				w = kPhonePresetWidth;
				h = kPhonePresetHeight;
				break;
			case kTabletPresetTag:
				w = kTabletPresetWidth;
				h = kTabletPresetHeight;
				break;
			default:
				break;
		}

		if ( w > 0 && h > 0 )
		{
			// Prevent recursive calls into this method...
			[self removeObserver:self forKeyPath:@"screenWidth"];
			[self removeObserver:self forKeyPath:@"screenHeight"];

			// ... while NSTextFields are updated...
			[self willChangeValueForKey:@"screenWidth"];
			fScreenWidth = [NSNumber numberWithInt:w];
			[self didChangeValueForKey:@"screenWidth"];

			[self willChangeValueForKey:@"screenHeight"];
			fScreenHeight = [NSNumber numberWithInt:h];
			[self didChangeValueForKey:@"screenHeight"];

			// ... and then re-register for KVO
			[self addObserver:self forKeyPath:@"screenWidth" options:0 context:NULL];
			[self addObserver:self forKeyPath:@"screenHeight" options:0 context:NULL];
		}
	}
	else if ( [keyPath isEqualToString:@"templateIndex"] )
	{
		// TODO: For e-books, we want to default to tablet-sized content
		// TODO: Find alternative to magic index number '3'
		if ( 3 == self.templateIndex )
		{
			// TODO: Investigate why we had to add the will/didChange calls
			[self willChangeValueForKey:@"screenSizeIndex"];
			self.screenSizeIndex = kTabletPresetTag;
			[self didChangeValueForKey:@"screenSizeIndex"];
		}
	}
	else
	{
		int w = [fScreenWidth intValue];
		int h = [fScreenHeight intValue];

		int tag = kCustomPresetTag;
		if ( kPhonePresetWidth == w && kPhonePresetHeight == h )
		{
			tag = kPhonePresetTag;
		}
		else if ( kTabletPresetWidth == w && kTabletPresetHeight == h )
		{
			tag = kTabletPresetTag;
		}

		[fScreenSize selectItemWithTag:tag];
	}
}

- (BOOL)validateValue:(id*)value error:(NSError**)outError localizedDescription:(NSString*)description code:(NSInteger)code
{
	if ( ! value )
	{
		// handled in setNilValueForKey
		return YES;
	}

	BOOL result = false;
	
	// Assertion: This is a NSNumber
	NSNumber* version = (NSNumber*)(*value);
	result = ( [version intValue] > 0 );
	if ( ! result )
	{
		NSDictionary* details = [[NSDictionary alloc] initWithObjectsAndKeys:description, NSLocalizedDescriptionKey, nil];
		NSError* error = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:code userInfo:details];

		*outError = error;

		[error autorelease];
		[details release];
	}

	return result;
}

- (BOOL)validateScreenWidth:(id*)value error:(NSError**)outError
{
	NSString *description = NSLocalizedString( @"Width must be an integer greater than 0", nil );
	return [self validateValue:value error:outError localizedDescription:description code:100];
}

- (BOOL)validateScreenHeight:(id*)value error:(NSError**)outError
{
	NSString *description = NSLocalizedString( @"Height must be an integer greater than 0", nil );
	return [self validateValue:value error:outError localizedDescription:description code:100];
}

- (void)setNilValueForKey:(NSString *)key
{
}

- (BOOL)panel:(id)sender validateURL:(NSURL *)url error:(NSError **)outError
{
	BOOL result = YES;

	NSURL *fileUrl = [url filePathURL];
	NSString *path = [fileUrl path];
	path = [path stringByAppendingPathComponent:fProjectName];

	BOOL isDir = NO;
	if ( [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir] )
	{
		Rtt_ASSERT( isDir ); Rtt_UNUSED( isDir );
		NSMutableDictionary *details = [NSMutableDictionary dictionary];
		NSString *format = NSLocalizedString( @"Cannot create new project folder. There is already a folder named '%@'. Please choose a new destination for the new project folder.", nil );
		NSString *msg = [NSString stringWithFormat:format, fProjectName ];
		[details setValue:msg forKey:NSLocalizedDescriptionKey];
		* outError = [NSError errorWithDomain:@"CoronaSimulator" code:1000 userInfo:details];
		result = NO;
	}

	return result;
}

- (IBAction)cancel:(id)sender
{
	[NSApp endSheet:fWindow];
}

- (IBAction)next:(id)sender
{
	NSOpenPanel *panel = [NSOpenPanel openPanel];
    // Try to pick a sensible starting directory; this is complicated by the fact that the default is
    // probably the directory for the last app we looked at.  If the current directory contains a "main.lua"
    // assume it's an app and go to the parent directory (if the user really wants to create their app there,
    // they can go there in the chooser)
    NSString *currDirectory = [[panel directoryURL] path];
    NSString *mainluaPath =[currDirectory stringByAppendingPathComponent:@"main.lua"];
    if ( [[NSFileManager defaultManager] fileExistsAtPath:mainluaPath] )
    {
        [panel setDirectoryURL:[NSURL fileURLWithPath:[currDirectory stringByDeletingLastPathComponent]]];
    }

	[panel setCanCreateDirectories:YES];
	[panel setCanChooseDirectories:YES];
	[panel setCanChooseFiles:NO];
	[panel setAllowsMultipleSelection:NO];
	[panel setDelegate:self];

	NSView *contentView = [panel contentView];
	NSButton *okButton = FindButtonWithSelector( contentView, @selector( ok: ) );
	[okButton setTitle:NSLocalizedString( @"Create", nil )];

	NSButton *cancelButton = FindButtonWithSelector( contentView, @selector( cancel: ) );
	[cancelButton setTitle:NSLocalizedString( @"Back", nil )];
	
	void (^handler)(NSInteger) = ^(NSInteger result)
	{
		if ( NSFileHandlingPanelOKButton == result )
		{
			// Save off parent reference since endSheet: nils it out
			NSWindow *parent = fParent;

			[NSApp endSheet:fWindow];
			
			NSString *path = [[[panel directoryURL] filePathURL] path];
			path = [path stringByAppendingPathComponent:fProjectName];
			self.projectPath = path;

			[[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil];

			[self createProject];

			NSString *message = NSLocalizedString( @"Project creation completed successfully", nil );
			NSString *show = NSLocalizedString( @"Show in Finder...", nil );
			NSString *open = NSLocalizedString( @"Open in Editor...", nil );

			NSAlert* alert = [[[NSAlert alloc] init] autorelease];
			[alert addButtonWithTitle:open];
			[alert addButtonWithTitle:show];
			[alert setMessageText:message];
			[alert setAlertStyle:NSInformationalAlertStyle];
			[alert beginSheetModalForWindow:parent
							  modalDelegate:self
							 didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
								contextInfo:NULL];
		}
	};

	[panel beginSheetModalForWindow:fWindow completionHandler:handler];
}


@end

// ----------------------------------------------------------------------------
