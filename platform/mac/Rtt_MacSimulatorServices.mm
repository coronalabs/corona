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

#include "Rtt_MacSimulatorServices.h"

#import "AppDelegate.h"
#import "CoronaWindowController.h"
#import "NewProjectController.h"
#import "SampleCodeLocator.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_TargetDevice.h"
#include "Rtt_SimulatorRecents.h"
#include "Rtt_SimulatorAnalytics.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaContext.h"
#include "Rtt_MacPlatform.h"
#import "TextEditorSupport.h"
#import "AppBuildController.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacSimulatorServices::MacSimulatorServices( AppDelegate *owner, CoronaWindowController *windowController, NSString *resourcePath )
:	fOwner( owner ),
    fResourcePath( resourcePath )
#ifdef Rtt_INCLUDE_COLOR_PANEL
    , fColorPanel([NSColorPanel sharedColorPanel])
#endif
{
    fWindowController = windowController;

#ifdef Rtt_INCLUDE_COLOR_PANEL
   // Initialize the color picker
    [fColorPanel setContinuous:NO];
    [fColorPanel setShowsAlpha:YES];
#endif

	[SampleCodeLocator sharedInstance];
}

MacSimulatorServices::~MacSimulatorServices()
{
}

bool
MacSimulatorServices::NewProject() const
{
	NSWindow *homeScreen = fOwner.homeScreen.window;
    
    NewProjectController *fNewProject;

    fNewProject = [[[NewProjectController alloc] initWithWindowNibName:@"NewProject" resourcePath:fResourcePath] autorelease];
    
    fNewProject.services = this;
	[fNewProject beginSheetModalForWindow:homeScreen];
    
	return true;
}

bool
MacSimulatorServices::OpenProject( const char *name ) const
{
    if ( name )
    {
        NSString *s = [NSString stringWithExternalString:name];
        
        // Make this call async
        [fOwner performSelector:@selector(openWithPath:) withObject:s afterDelay:0.0]; // [fOwner open:nil];
    }
    else
    {
        // Make this call async
        [fOwner performSelector:@selector(open:) withObject:nil afterDelay:0.0]; // [fOwner open:nil];
    }
    
    return true;
}

bool
MacSimulatorServices::BuildProject( const char *platform ) const
{
	if ( [fOwner isBuildAvailable] )
	{
		SEL selector = nil;
		if ( TargetDevice::kAndroidPlatform == TargetDevice::PlatformForString( platform ) )
		{
			selector = @selector(openForBuildAndroid:);
		}
		else
		{
			selector = @selector(openForBuildiOS:);
		}

		// Make this call async
		[fOwner performSelector:selector withObject:nil afterDelay:0.0];
	}
	else
	{
		// TODO: Add an alert saying you must open a project first...
		NSAlert* alert = [[[NSAlert alloc] init] autorelease];
		[alert addButtonWithTitle:NSLocalizedString( @"OK", nil )];
		[alert setMessageText:NSLocalizedString( @"You must have a project open prior to building. Click Simulator to open a project. ",nil )];
		[alert setAlertStyle:NSInformationalAlertStyle];
		[alert beginSheetModalForWindow:fOwner.homeScreen.window
						  modalDelegate:nil
						 didEndSelector:nil
							contextInfo:NULL];
	}

	return true;
}

void
MacSimulatorServices::ShowSampleCode() const
{
    void (^runSampleProject)(NSString*) = ^(NSString* path)
    {
        if ( path )
        {
            [fOwner close:nil];
            [fOwner runApp:path];
        }
    };
    
    void (^showSampleCode)() = ^()
    {
        SampleCodeLocator *instance = [SampleCodeLocator sharedInstance];
        
        NSString *path = [instance path];
        if ( path )
        {
            [fOwner showOpenPanel:nil withAccessoryView:[fOwner openAccessoryView] startDirectory:path completionHandler:runSampleProject];
        }
    };
    
    // Make this call async
    [[NSOperationQueue mainQueue] addOperationWithBlock:showSampleCode];
}

void
MacSimulatorServices::SelectOpenFilename(const char* currDirectory, const char* extn, LuaResource* resource) const
{
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    
    if (currDirectory != NULL)
    {
        [panel setDirectoryURL:[NSURL fileURLWithPath:[[NSString stringWithExternalString:currDirectory] stringByStandardizingPath]]];
    }
    
    if (extn != NULL)
    {
        [panel setAllowedFileTypes:[NSArray arrayWithObject:[NSString stringWithExternalString:extn]]];
    }
    
    // This method displays the panel and returns immediately.
    // The completion handler is called when the user selects an
    // item or cancels the panel.
    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseDirectories:NO];
    
    [panel beginSheetModalForWindow:[fWindowController window] completionHandler:^(NSInteger result){
        const char *resultStr = "cancelled";
        NSString*  filename = nil;
        
        if (result == NSFileHandlingPanelOKButton)
        {
            resultStr = "clicked";
            filename = [[[panel URLs] objectAtIndex:0] path];
        }
        
        CompletionEvent e;
        int nargs = resource->PushListenerAndEvent( e );
        if ( nargs > 0 )
        {
            lua_State *L = resource->L(); Rtt_ASSERT( L );
            
            RuntimeGuard guard( * LuaContext::GetRuntime( L ) );
            
            lua_pushstring( L, [filename UTF8String] );
            lua_setfield( L, -2, "filename" );
            lua_pushstring( L, resultStr );
            lua_setfield( L, -2, "action" );
            LuaContext::DoCall( L, nargs, 0 );
            
            Rtt_DELETE(resource);
        }
        
    }];
    
}

void
MacSimulatorServices::SelectSaveFilename(const char* newFilename, const char* currDirectory, const char* extn, LuaResource* resource) const
{
    NSSavePanel* panel = [NSSavePanel savePanel];
    
    if (currDirectory != NULL)
    {
        [panel setDirectoryURL:[NSURL fileURLWithPath:[[NSString stringWithExternalString:currDirectory] stringByStandardizingPath]]];
    }
    
    if (newFilename != NULL)
    {
        [panel setNameFieldStringValue:[NSString stringWithExternalString:newFilename]];
    }
    
    if (extn != NULL)
    {
        [panel setAllowedFileTypes:[NSArray arrayWithObject:[NSString stringWithExternalString:extn]]];
    }
    
    // This method displays the panel and returns immediately.
    // The completion handler is called when the user selects an
    // item or cancels the panel.
    
    [panel beginSheetModalForWindow:[fWindowController window] completionHandler:^(NSInteger result){
        const char *resultStr = "cancelled";
        NSString*  filename = nil;
        
        if (result == NSFileHandlingPanelOKButton)
        {
            resultStr = "clicked";
            filename = [[panel URL] path];
        }
        
        CompletionEvent e;
        int nargs = resource->PushListenerAndEvent( e );
        if ( nargs > 0 )
        {
            lua_State *L = resource->L(); Rtt_ASSERT( L );
            
            RuntimeGuard guard( * LuaContext::GetRuntime( L ) );
            
            lua_pushstring( L, [filename UTF8String] );
            lua_setfield( L, -2, "filename" );
            lua_pushstring( L, resultStr );
            lua_setfield( L, -2, "action" );
            LuaContext::DoCall( L, nargs, 0 );
            
            Rtt_DELETE(resource);
        }
        
    }];
}

// Get the filesystem path of the currently open project or NULL if none is open
const char*
MacSimulatorServices::GetCurrProjectPath( ) const
{
    const char *currProjPath = NULL;
    
    if (fOwner.fSimulator != nil && fOwner.fAppPath != nil)
    {
        currProjPath = [fOwner.fAppPath UTF8String];
    }
    
    return currProjPath;
}

// Run the specified Simulator extension
void
MacSimulatorServices::RunExtension(const char *extName) const
{
    if (extName != NULL)
    {
        // The delay makes this interface play nice with OpenProject()
        [fOwner performSelector:@selector(runExtension:) withObject:[NSString stringWithExternalString:extName] afterDelay:0.0];
    }
}

// Set the current project resource path
void
MacSimulatorServices::SetProjectResourceDirectory(const char *projectResourceDirectory)
{
    if (fWindowController != nil && fWindowController.fView != nil && fWindowController.fView._platform != nil)
    {
        Rtt::MacPlatform *platform = fWindowController.fView._platform;
        
        platform->SetProjectResourceDirectory(projectResourceDirectory);
    }
}

const char*
MacSimulatorServices::GetSubscription( S32 *expirationTimestamp ) const
{
	const AuthorizationTicket *ticket = [fOwner ticket];

	if ( ticket && expirationTimestamp )
	{
		* expirationTimestamp = ticket->GetExpiration();
	}

	AuthorizationTicket::Subscription sub =
		( ticket ? ticket->GetSubscription() : AuthorizationTicket::kUnknownSubscription );
	return AuthorizationTicket::StringForSubscription( sub );
}

void
MacSimulatorServices::GetRecentDocs(LightPtrArray<RecentProjectInfo> *list) const
{
	NSArray *recentDocs = [fOwner GetRecentDocuments];
	
	int recents = (int)[recentDocs count] > 10 ? 10 : (int)[recentDocs count];
	for (int i = 0; i < recents; i++)
	{
		RecentProjectInfo *info = new RecentProjectInfo();	
		NSString *str = [[recentDocs objectAtIndex:i] path];

		// Previously, we stored the path to the main.lua file in the recent documents list,
		// now we store just the path to the project, so adjust any old paths
		if ([[str lastPathComponent] isEqualToString:@"main.lua"])
		{
			str = [str stringByDeletingLastPathComponent];
		}

        const char *fullPathString = [str UTF8String];
        
        if (fullPathString != NULL)
        {
            info->fullURLString = fullPathString;
            
            const char *formattedString = [[str lastPathComponent] UTF8String];
            
            if (formattedString != NULL)
            {
                info->formattedString = formattedString;
            }
            else
            {
                info->formattedString = fullPathString;
            }

            list->Append(info);
        }
	}
}

void
MacSimulatorServices::SetWindowCloseListener(LuaResource* resource) const
{
    if (resource == NULL)
    {
        [fWindowController setWindowShouldCloseBlock:nil];
    }
    else
    {
        BOOL (^shouldCloseBlock)() = ^()
        {
            BOOL result = true;
            
            CompletionEvent e;
            int nargs = resource->PushListenerAndEvent( e );
            if ( nargs > 0 )
            {
                lua_State *L = resource->L(); Rtt_ASSERT( L );
                
                RuntimeGuard guard( * LuaContext::GetRuntime( L ) );
                
                lua_pushstring( L, "close" );
                lua_setfield( L, -2, "action" );
                
                LuaContext::DoCall( L, nargs, 1 );
                
                if (lua_isboolean( L, -1 ))
                {
                    result = lua_toboolean(L, -1);
                }
                
                lua_pop( L, 1 );
            }
            
            return result;
            
        };
        
        [fWindowController setWindowShouldCloseBlock:shouldCloseBlock];
    }
}

void
MacSimulatorServices::SetWindowResizeListener(LuaResource* resource) const
{
    if (resource == NULL)
    {
        [fWindowController setWindowWillResizeBlock:nil];
    }
    else
    {
        BOOL (^resizeBlock)(int oldWidth, int oldHeight, int newWidth, int newHeight) = ^(int oldWidth, int oldHeight, int newWidth, int newHeight)
        {
            BOOL result = true;
            
            CompletionEvent e;
            int nargs = resource->PushListenerAndEvent( e );
            if ( nargs > 0 )
            {
                lua_State *L = resource->L(); Rtt_ASSERT( L );
                
                RuntimeGuard guard( * LuaContext::GetRuntime( L ) );
                
                lua_pushstring( L, "resize" );
                lua_setfield( L, -2, "action" );
                
                lua_pushinteger( L, oldWidth );
                lua_setfield( L, -2, "oldWidth" );
                
                lua_pushinteger( L, oldHeight );
                lua_setfield( L, -2, "oldHeight" );
                
                lua_pushinteger( L, newWidth );
                lua_setfield( L, -2, "newWidth" );
                
                lua_pushinteger( L, newHeight );
                lua_setfield( L, -2, "newHeight" );
                
                LuaContext::DoCall( L, nargs, 1 );
                
                if (lua_isboolean( L, -1 ))
                {
                    result = lua_toboolean(L, -1);
                }
                
                lua_pop( L, 1 );
            }
            
            return result;
            
        };
        
        [fWindowController setWindowWillResizeBlock:resizeBlock];
    }
}

void
MacSimulatorServices::SetDocumentEdited(bool value) const
{
    [[fWindowController window] setDocumentEdited:value];
}

bool
MacSimulatorServices::GetDocumentEdited( ) const
{
    return [[fWindowController window] isDocumentEdited];
}

void
MacSimulatorServices::CloseWindow() const
{
    [fWindowController close];
}

// Get the value of the given preference (user registry setting)
const char*
MacSimulatorServices::GetPreference(const char *prefName) const
{
    const char *prefValue = [[[NSUserDefaults standardUserDefaults] stringForKey:[NSString stringWithExternalString:prefName]] UTF8String];
    
    return prefValue;
}

// Set the value of the given preference (user registry setting)
void
MacSimulatorServices::SetPreference(const char *prefName, const char *prefValue) const
{
    NSString *nsPrefName = [NSString stringWithExternalString:prefName];
    NSString *nsPrefValue = [NSString stringWithExternalString:prefValue];

    [[NSUserDefaults standardUserDefaults] setObject:nsPrefValue forKey:nsPrefName];
}

void
MacSimulatorServices::SetCursorRect(const char *cursorName, int x, int y, int w, int h) const
{
    if (fWindowController.fView != NULL)
    {
        [fWindowController.fView.glView setCursor:cursorName forRect:NSMakeRect(x, y, w, h)];
    }
}
    
void
MacSimulatorServices::SetWindowTitle(const char *windowTitle) const
{
    [[fWindowController window] setTitle:[NSString stringWithExternalString:windowTitle]];
}

void
MacSimulatorServices::OpenTextEditor(const char *filename) const
{
    NSString *filepath = [NSString stringWithExternalString:filename];
    
	if ( ! [[NSFileManager defaultManager] fileExistsAtPath:filepath] )
    {
        Rtt_TRACE_SIM(("Warning: path '%s' not found for simulator.OpenTextEditor()", filename));
    }
    
    TextEditorSupport_LaunchTextEditorWithFile(filepath, 0);
}

void
MacSimulatorServices::OpenColorPanel(double r, double g, double b, double a, LuaResource* callback) const
{
#ifdef Rtt_INCLUDE_COLOR_PANEL
    if (callback == NULL)
    {
        [fWindowController setColorPanelCallbackBlock:nil];
        [fWindowController hideColorPanel];
    }
    else
    {
        void (^colorPanelCallbackBlock)(double r, double g, double b, double a) = ^(double r, double g, double b, double a)
        {
            CompletionEvent e;
            int nargs = callback->PushListenerAndEvent( e );
            if ( nargs > 0 )
            {
                lua_State *L = callback->L(); Rtt_ASSERT( L );

                RuntimeGuard guard( * LuaContext::GetRuntime( L ) );

                lua_pushstring( L, "colorPicked" );
                lua_setfield( L, -2, "action" );

                lua_pushnumber( L, r );
                lua_setfield( L, -2, "r" );

                lua_pushnumber( L, g );
                lua_setfield( L, -2, "g" );

                lua_pushnumber( L, b );
                lua_setfield( L, -2, "b" );

                lua_pushnumber( L, a );
                lua_setfield( L, -2, "a" );

                LuaContext::DoCall( L, nargs, 0 );
            }
        };

        [fColorPanel setTarget:fWindowController];
        [fColorPanel setAction:@selector(colorPanelAction:)];

        [fWindowController setColorPanelCallbackBlock:colorPanelCallbackBlock];

        [fColorPanel setColor:[NSColor colorWithSRGBRed:(CGFloat)r green:(CGFloat)g blue:(CGFloat)b alpha:(CGFloat)a]];

        [NSApp orderFrontColorPanel:nil];
    }
#endif // Rtt_INCLUDE_COLOR_PANEL
}

void
MacSimulatorServices::SetBuildMessage(const char *message) const
{
	AppBuildController *windowController = (AppBuildController *)fWindowController;

	if ([windowController respondsToSelector:@selector(setBuildMessage:)])
	{
		[windowController setBuildMessage:[NSString stringWithExternalString:message]];
	}
}

void
MacSimulatorServices::SendAnalytics(const char *eventName, const char *keyName, const char *value) const
{
	if (fOwner.analytics != nil)
	{
		fOwner.analytics->Log(eventName, keyName, value);
	}
}

bool
MacSimulatorServices::RelaunchProject( ) const
{
	[fOwner performSelector:@selector(launchSimulator:) withObject:nil afterDelay:0.0];

	return true;
}

bool
MacSimulatorServices::EditProject( const char *name ) const
{
	NSString *projectDir = [NSString stringWithExternalString:name];

	if (! [fOwner.fAppPath isEqualToString:[projectDir stringByStandardizingPath]])
	{
		// Project isn't open, open it before proceeding
		[fOwner openWithPath:projectDir];
	}

	[fOwner performSelector:@selector(openMainLuaInEditor:) withObject:nil afterDelay:0.0];

	return true;
}

bool
MacSimulatorServices::ShowProjectFiles( const char *name ) const
{
	NSString *projectDir = [NSString stringWithExternalString:name];

	if (! [fOwner.fAppPath isEqualToString:[projectDir stringByStandardizingPath]])
	{
		// Project isn't open, open it before proceeding
		[fOwner openWithPath:projectDir];
	}

	[fOwner performSelector:@selector(showProjectFiles:) withObject:nil afterDelay:0.0];

	return true;
}

bool
MacSimulatorServices::ShowProjectSandbox( const char *name ) const
{
	NSString *projectDir = [NSString stringWithExternalString:name];

	if (! [fOwner.fAppPath isEqualToString:[projectDir stringByStandardizingPath]])
	{
		// Project isn't open, open it before proceeding
		[fOwner openWithPath:projectDir];
	}
	else if (fOwner.fSimulator == NULL)
	{
		[fOwner openWithPath:projectDir];
	}

	[fOwner performSelector:@selector(showProjectSandbox:) withObject:nil afterDelay:0.0];

	return true;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

