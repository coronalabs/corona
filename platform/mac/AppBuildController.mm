//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "AppBuildController.h"

#import "AppDelegate.h"

// TODO: We need this header to get the key kDstFolderPath (move those keys somewhere else)
#import "Rtt_MacPlatform.h"

#import "Rtt_MacConsolePlatform.h"

#include "Rtt_PlatformAppPackager.h"

#import "NSAlert-OAExtensions.h"
#import "ValidationSupportMacUI.h"
#include "Rtt_SimulatorAnalytics.h"

// ----------------------------------------------------------------------------

using namespace Rtt;


// ----------------------------------------------------------------------------

@implementation NSAlert (SynchronousSheet)

/**
 * @brief Run an alert truly modally (it doesn't return until a button is pressed)
 * @attention if you supply a completion handler block it @b must call [NSApp stopModalWithCode:] in addition to any other things it does
 * @returns An integer indicating the reason that this method returned. See NSModalResponse for possible return values.
 **/

-(NSInteger) runModalSheetForWindow:(NSWindow *)window completionHandler:(void (^)(NSModalResponse returnCode))completionHandler
{
    if (completionHandler == nil)
    {
        completionHandler = ^(NSModalResponse returnCode) {
            [NSApp stopModalWithCode:returnCode];
        };
    }

    [self beginSheetModalForWindow:window completionHandler:completionHandler];

    [[self window] makeFirstResponder:nil]; // stop the focus ring settling on the last button defined

    NSInteger modalCode = [NSApp runModalForWindow:[self window]];

    return modalCode;
}

/**
 * @brief Run an alert truly modally (it doesn't return until a button is pressed)
 * @returns An integer indicating the reason that this method returned. See NSModalResponse for possible return values.
 **/

-(NSInteger) runModalSheetForWindow:(NSWindow *)window
{
    return [self runModalSheetForWindow:window completionHandler:nil];
}

/**
 * @brief Run an alert on the app's main window truly modally (it doesn't return until a button is pressed)
 * @returns An integer indicating the reason that this method returned. See NSModalResponse for possible return values.
 **/

-(NSInteger) runModalSheet
{
    return [self runModalSheetForWindow:[NSApp mainWindow]];
}

@end

// ----------------------------------------------------------------------------

static NSString *kValueNotSet = @"not set";

@implementation AppBuildController

@synthesize appName;
@synthesize appVersion;
@synthesize dstPath;
@synthesize projectPath;
@synthesize progressSheetMessage;
@synthesize platformName;
@synthesize platformTitle;

+ (NSString*)defaultDstDir
{
	NSString *result = [[NSUserDefaults standardUserDefaults] stringForKey:kDstFolderPath];

	if ( ! result )
	{
		NSArray *paths = NSSearchPathForDirectoriesInDomains(
			NSDesktopDirectory, NSUserDomainMask, YES );
		result = [paths objectAtIndex:0];
	}

	return result;
}

- (id)initWithWindowNibName:(NSString*)nibFile
                projectPath:(NSString *)projPath;
{
	self = [super initWithWindowNibName:nibFile];

	if ( self )
	{
        appDelegate = (AppDelegate*)[NSApp delegate];

		fSigningIdentities = nil;

        self.projectPath = projPath;

		fAnalytics = NULL;

		platformName = @"none";
		platformTitle = @"OS";

        progressBannerParent = nil;

		launchedTask = nil;
	}

	return self;
}

- (void)dealloc
{
	[appName release];
	[appVersion release];
	[dstPath release];
	[projectPath release];
	[super dealloc];
}

- (void)windowDidLoad
{
	if ( fSigningIdentities )
	{
		[fSigningIdentities setAutoenablesItems:NO];
	}

	[self.window center];
	[self.window setReleasedWhenClosed:NO];
}

- (void) showWindow:(id)sender
{
    [super showWindow:sender];

    [self restoreBuildPreferences];

    // Set the post build action's radio group's initial selection (the default is whatever was used last time for a build with this OS)
	NSString *defaultPostBuildAction = [[NSUserDefaults standardUserDefaults] stringForKey:[NSString stringWithFormat:@"last%@PostBuildAction", self.platformName]];
	if ([defaultPostBuildAction length] == 0)
	{
		defaultPostBuildAction = kValueNotSet;
	}
	NSString *savedPostBuildAction = [appDelegate restoreAppSpecificPreference:[self.platformName stringByAppendingString:@"PostBuildAction"] defaultValue:defaultPostBuildAction];
    if ([savedPostBuildAction isEqualToString:kValueNotSet] ||
        [savedPostBuildAction integerValue] >= [postBuildRadioGroup numberOfRows])
    {
        // No saved preference (or a bogus one), select the last ("Do nothing") radio button
        [postBuildRadioGroup selectCellAtRow:([postBuildRadioGroup numberOfRows]-1) column:0];
    }
    else
    {
        [postBuildRadioGroup selectCellAtRow:[savedPostBuildAction integerValue] column:0];
    }
}

- (void) setProgressBarLabel:(NSString *) newLabel;
{
	[self willChangeValueForKey:@"progressSheetMessage"];

	progressSheetMessage = newLabel;

	[self didChangeValueForKey:@"progressSheetMessage"];
}

-(void)updateProgressBarLabel:(id)sender
{
	progressSheetMessage = NSLocalizedString( @"Connecting with server...", nil );
	[self performSelector:@selector(build:) withObject:nil afterDelay:0.1];
}

- (void)setBuildMessage:(NSString *)message
{
	NSString *buildMessage = [NSString stringWithFormat:@"%@…", message];

	[self setProgressBarLabel:buildMessage];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
}

- (void)beginProgressSheetBanner:(NSWindow *)parent withDelay:(NSTimeInterval)delay
{
	[self beginProgressSheetBanner:parent withDelay:delay allowStop:NO];
}

- (void)beginProgressSheetBanner:(NSWindow *)parent withDelay:(NSTimeInterval)delay allowStop:(BOOL)allowStop
{
	if ( progressSheetBanner == nil )
	{
		[NSBundle loadNibNamed:@"AppBuildProgressBanner" owner:self]; Rtt_ASSERT( progressSheetBanner );
		[progressSheetBanner setReleasedWhenClosed:NO];
	}

    if (progressBannerParent != nil)
    {
        [self endProgressSheetBanner:progressBannerParent];
    }
    progressBannerParent = parent;

	appDelegate.stopRequested = NO;

	if (allowStop)
	{
		[stopButton setHidden:NO];
	}
	else
	{
		[stopButton setHidden:YES];
	}

	progressBannerTimer = [NSTimer scheduledTimerWithTimeInterval:delay target:self selector:@selector(showProgressSheetBanner:) userInfo:progressBannerParent repeats:NO];
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]]; // handle the zero delay case
}

- (void)showProgressSheetBanner:(NSTimer *)timer
{
    NSWindow *parent = [timer userInfo];

    // The window must be visible for the progress sheet to make sense
    [parent orderFront:self];

	// Stop the app icon bouncing when we display the modal sheet
	[(CoronaSimulatorApplication *)NSApp setSuppressAttentionRequests:YES];

	[buildProgressIndicator startAnimation:parent];
	[parent beginSheet:progressSheetBanner completionHandler:nil];

	[(CoronaSimulatorApplication *)NSApp setSuppressAttentionRequests:NO];
}

- (void)endProgressSheetBanner:(NSWindow *)parent
{
    if (progressBannerParent != nil)
    {
        Rtt_ASSERT(progressBannerParent == parent);

        [progressBannerTimer invalidate];
        [buildProgressIndicator stopAnimation:self];
        [progressBannerParent endSheet:progressSheetBanner];
        progressBannerParent = nil;
    }
}

// Note enabling "allowStop" is incompatible with having a delay (the sheet always appears immediately)
- (void) runLengthyOperationForWindow:(NSWindow *)window delayProgressWindowBy:(NSTimeInterval)delay allowStop:(BOOL)allowStop withBlock:(void (^)(void))block
{
	appDelegate.buildDownloadProgess = 0;

    [self beginProgressSheetBanner:window withDelay:delay allowStop:allowStop];

    // Make this call async
    NSOperationQueue *backgroundQueue = [[NSOperationQueue alloc] init];
    [backgroundQueue addOperationWithBlock:block];

	/*
    // Wait for it to complete while pumping the run loop
    while ([backgroundQueue operationCount] > 0)
    {
		NSLog(@"Checking runloop");
		[NSTimer scheduledTimerWithTimeInterval:100.0 target:nil selector:nil userInfo:nil repeats:NO];
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];
		runModalSession
		// [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];

		//[[NSRunLoop currentRunLoop] acceptInputForMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:10.2]];
    }
	*/

	NSModalSession session = nil;
	if (allowStop)
	{
		// Stop the app icon bouncing when we display the modal sheet
		[(CoronaSimulatorApplication *)NSApp setSuppressAttentionRequests:YES];

		session = [NSApp beginModalSessionForWindow:progressSheetBanner];
	}

	while ([backgroundQueue operationCount] > 0)
	{
		[buildDownloadProgress setDoubleValue:(appDelegate.buildDownloadProgess * 100)];

		if (allowStop)
		{
			if ([NSApp runModalSession:session] != NSRunContinuesResponse)
			{
				break;
			}
		}

		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
	}

	if (allowStop)
	{
		[NSApp endModalSession:session];

		[(CoronaSimulatorApplication *)NSApp setSuppressAttentionRequests:NO];
	}

    // Discard any events that got queued while waiting (stops buttons being clicked on calling dialog)
    [NSApp discardEventsMatchingMask:NSAnyEventMask beforeEvent:nil];

    [self endProgressSheetBanner:window];
}

- (BOOL)shouldInitiateBuild:(id)sender
{
	BOOL result = [self buildFormComplete];

	if (result )
	{
		NSWindow *buildWindow = [self window];
		[self beginProgressSheetBanner:buildWindow withDelay:0];
	}

	return result;
}

- (const char *)getAppVersion
{
	const char* version = NULL;
	// The number formatter for Android seems to be causing the value to be returned to be a NSNumber instead of NSString.
	// We must react accordingly.
	if( [appVersion isKindOfClass:[NSNumber class]] )
	{
		version = [[(NSNumber*)appVersion stringValue] UTF8String];
	}
	else
	{
		// Assume NSString?
		version = [appVersion UTF8String];
	}

	return version;
}

- (const char *)dstDir
{
	BOOL isDir = NO;
	if ( ! dstPath
		 || ! [[NSFileManager defaultManager] fileExistsAtPath:dstPath isDirectory:&isDir]
		 || ! isDir )
	{
		self.dstPath = [[self class] defaultDstDir];
	}

	const char* dstDir = [dstPath UTF8String];

	return dstDir;
}

- (BOOL)validateProject
{
    BOOL result = YES;

#ifdef NOT_USED
	// Run file validation tools so we can alert the users of any problems before we submit to the server.
	NSWindow *buildWindow = [self window];

	ValidationSupportMacUI *validator = [[ValidationSupportMacUI alloc] initWithParentWindow:buildWindow];
	result = [validator runCommonFileValidationTestsInProjectPath:[self projectPath]];
	[validator release];

	if ( ! result )
	{
		[self logEvent:"build-invalid-generic-project"];
	}
#endif // NOT_USED

	return result;
}

- (BOOL)isStoreBuild
{
	return NO;
}

- (BOOL)isDeveloperBuild
{
	return ! [self isStoreBuild];
}

- (Rtt::PlatformAppPackager*)createAppPackager:(Rtt::MPlatformServices*)services
{
	return NULL;
}

- (Rtt::AppPackagerParams*)createAppPackagerParams:(Rtt::PlatformAppPackager*)packager
{
	return NULL;
}

- (NSString*)alertMessage
{
	return nil;
}

- (void)willShowAlert:(NSAlert*)alert
{
}

- (BOOL)verifyBuildTools:(id)sender
{
	return YES;
}

- (BOOL)shouldRemoveWhitespaceFromPackageName
{
	return NO;
}

- (NSString*)appExtension
{
	Rtt_ASSERT_NOT_REACHED();
	return @"";
}

- (NSString *)appPackageWithExtension:(NSString*)extension
{
	// Slash is illegal in filenames so replace it with an underscore
	NSString* bundleName = [appName stringByReplacingOccurrencesOfString:@"/" withString:@"_"];

	if ( [self shouldRemoveWhitespaceFromPackageName] )
	{
		bundleName = [appName stringByReplacingOccurrencesOfString:@" " withString:@""];
	}

	NSString* dstPackage = bundleName;

	if ( extension )
	{
		dstPackage = [dstPackage stringByAppendingPathExtension:extension];
	}

	return dstPackage;
}

- (NSString*)appPackagePath
{
	NSString *extension = [self appExtension];
	NSString *dstApp = [self appPackageWithExtension:extension];

	dstApp = [self.dstPath stringByAppendingPathComponent:dstApp];
	return dstApp;
}

- (NSString *)appBundleFile
{
    return [self appPackagePath];
}

- (void)setAnalytics:(Rtt::SimulatorAnalytics *)analytics
{
	fAnalytics = analytics;
}

- (void)logEvent:(NSString *)eventName
{
	Rtt_ASSERT(eventName != nil && [eventName length] > 0);

	NSDictionary *keyValues = @{ @"target" : [self platformName] };

	[self logEvent:eventName keyValues:keyValues];
}

- (void)logEvent:(NSString *)eventName key:(NSString *)eventDataKey value:(NSString *)eventDataValue
{
	Rtt_ASSERT(eventName != nil && [eventName length] > 0);
	Rtt_ASSERT(eventDataKey != nil && [eventDataKey length] > 0);
	Rtt_ASSERT(eventDataValue != nil && [eventDataValue length] > 0);

	NSDictionary *keyValues = @{ @"target" : [self platformName], eventDataKey : eventDataValue };

	[self logEvent:eventName keyValues:keyValues];
}

- (void)logEvent:(NSString *)eventName keyValues:(NSDictionary *)keyValues
{
	Rtt_ASSERT(fAnalytics != NULL);
	Rtt_ASSERT(eventName != nil && [eventName length] > 0);

	size_t numItems = [keyValues count];
	char **dataKeys = (char **) calloc(sizeof(char *), numItems);
	char **dataValues = (char **) calloc(sizeof(char *), numItems);

	if ( fAnalytics != NULL )
	{
		size_t i = 0;
		for (NSString *key in keyValues)
		{
			dataKeys[i] = strdup([key UTF8String]);
			dataValues[i] = strdup([keyValues[key] UTF8String]);
			++i;
		}

		fAnalytics->Log( [eventName UTF8String], numItems, dataKeys, dataValues);

		free(dataKeys);
		free(dataValues);
	}
}

-(void)closeBuild:(id)sender
{
	[self.window close];
	projectPath = nil;

	// Unsuspend the Simulator (it's suspended by the Simulator's AppDelegate when we're opened)
	[appDelegate toggleSuspendResume:self];
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void  *)contextInfo
{
	if ( NSAlertSecondButtonReturn == returnCode )
	{
		/*
		// TODO: appName stores the display name --- in the iPhonePackageApp.lua,
		// we strip spaces out of the name b/c bundle.app's cannot have space,
		// so we do it here manually, but we should consolidate the
		// display name/bundle name code
		NSString* bundleName = [appName stringByReplacingOccurrencesOfString:@" " withString:@""];
		NSString* dstApp;

		if ( fIsAndroid ) 
			dstApp = [bundleName stringByAppendingPathExtension:@"apk"];
		else
			dstApp = [bundleName stringByAppendingPathExtension:@"app"];

		dstApp = [dstPath stringByAppendingPathComponent:dstApp];
		*/

		NSString *dstApp = [self appPackagePath];

		// Reveal in Finder
		(void)Rtt_VERIFY( [[NSWorkspace sharedWorkspace] selectFile:dstApp inFileViewerRootedAtPath:@""] );
	}

	[self closeBuild:nil];
}

- (void)didPresentError:(BOOL)didRecover contextInfo:(void*)contextInfo
{
}

- (IBAction)build:(id)sender
{
}

- (IBAction)cancelBuild:(id)sender
{
	[self closeBuild:sender];
}

- (IBAction)browseDstPath:(id)sender
{
	NSOpenPanel *panel = panel = [NSOpenPanel openPanel];

	[panel setAllowsMultipleSelection:NO];
	[panel setCanChooseDirectories:YES];
	[panel setCanCreateDirectories:YES];
	[panel setCanChooseFiles:NO];

	// Pull the user's last folder from user defaults for their convenience
	NSString* initialDir = [[self class] defaultDstDir];
	NSURL *initialURL = [NSURL URLWithString:initialDir];
	[panel setDirectoryURL:initialURL];

	NSInteger result = [panel runModal];
	if ( NSCancelButton != result )
	{
		NSArray* filenames = [panel URLs]; Rtt_ASSERT( [filenames count] <= 1 );
		NSString* path = [[filenames lastObject] path];
		Rtt_ASSERT( [[NSFileManager defaultManager] fileExistsAtPath:path] );
		// Save the chosen path into user defaults so we can use it as the starting path next time
		[[NSUserDefaults standardUserDefaults] setObject:[[panel directoryURL] path] forKey:kDstFolderPath];

//		[self willChangeValueForKey:@"dstPath"];
		self.dstPath = path;
//		[self didChangeValueForKey:@"dstPath"];
	}
}

- (BOOL) buildFormComplete
{
	BOOL folderExists = NO;
    BOOL isDir = NO;

	if (appName == nil || [appName isEqualToString:@""])
	{
		[self showError:@"Missing Application Name" message:@"Enter a name for the application" helpURL:nil parentWindow:[self window]];

		return NO;
	}

	if (appVersion == nil || [appVersion isEqualToString:@""])
	{
		[self showError:@"Missing Version" message:@"Enter a version for the application" helpURL:nil parentWindow:[self window]];

		return NO;
	}

	folderExists = [[NSFileManager defaultManager] fileExistsAtPath:self.dstPath isDirectory:&isDir];
	if (! folderExists || ! isDir)
	{
		NSString *mesg = nil;

		if (! folderExists)
		{
			mesg = @"The Save to Folder must exist before the build starts.  Enter a different folder to save the built app to or press Browse and use the New Folder button to create one";
		}
		else // ! isDir
		{
			mesg = @"The Save to Folder path is not a folder.  Use the Browse button to find a new one";
		}

		[self showError:@"Invalid Save to Folder" message:mesg helpURL:nil parentWindow:[self window]];

		return NO;
	}

    // Reject attempts to build to the project dir
    if ([self.dstPath isEqualToString:self.projectPath] || [self.dstPath hasPrefix:[self.projectPath stringByAppendingString:@"/"]])
    {
        [self showError:@"Invalid Save to Folder" message:@"Save to Folder cannot be the same as the project folder. Choose a different folder to save the built app to" helpURL:nil parentWindow:[self window]];

        return NO;
    }

	return YES;
}

#if 0 // improve error reporting the in the dialog someday
- (BOOL) validateValue:(inout id *)ioValue forKey:(NSString *)inKey error:(out NSError **)outError
{
    NSLog(@"validateValue: %@ %@", *ioValue, inKey);

    if ([inKey isEqualToString:@"dstPath"])
    {
        BOOL isDir = NO;
        NSDictionary *details = [[NSDictionary alloc] initWithObjectsAndKeys:@"Save to folder must be an existing directory", NSLocalizedDescriptionKey, nil];
        *outError = [[NSError alloc] initWithDomain:@"CoronaSimulator" code:100 userInfo:details];

        return ([[NSFileManager defaultManager] fileExistsAtPath:*ioValue isDirectory:&isDir] && isDir);
    }
    return YES;
}
- (BOOL) validateText:(inout id *)ioValue error:(out NSError **)outError
{
    NSLog(@"validateText: %@", *ioValue);
    return YES;
}
#endif // 0

- (NSMenu*)signingIdentitiesMenu
{
	if ( ! fSigningIdentities )
	{
		// Force window to load
		[self window];
	}
	return [fSigningIdentities menu];
}

-(void)didSelectIdentity:(NSNotification*)notification
{
}

- (Rtt::TargetDevice::Platform)targetPlatform
{
	Rtt_ASSERT_NOT_REACHED();
	return Rtt::TargetDevice::kUnknownPlatform;
}

- (void) stopLaunchedTask
{
	@try
	{
		if ([launchedTask isRunning])
		{
			[launchedTask terminate];
			[launchedTask release];
			launchedTask = nil;
		}
	}
	@catch( NSException* exception )
	{
		NSLog( @"stopLaunchedTask: exception %@", exception );
	}
}

// TODO: Not have results affected by DEBUG_BUILD_PROCESS preference.
- (NSString *) launchTaskAndReturnOutput:(NSString *)cmd arguments:(NSArray *)args
{
	NSString *result = @"";
	NSPipe *stdoutPipe = [NSPipe pipe];
	NSPipe *stderrPipe = [NSPipe pipe];
    NSFileHandle *stdoutFileHandle = [stdoutPipe fileHandleForReading];
    NSFileHandle *stderrFileHandle = [stderrPipe fileHandleForReading];

	Rtt_ASSERT(launchedTask == nil);
	launchedTask = [[[NSTask alloc] init] retain];

	[launchedTask setLaunchPath:cmd];
	[launchedTask setArguments:args];

	[launchedTask setStandardOutput:stdoutPipe];
	[launchedTask setStandardError:stderrPipe];

	@try
	{
		[launchedTask launch];
		[launchedTask waitUntilExit];

		if (launchedTask == nil)
		{
			// The task was terminated by stopLaunchedTask (assume user feedback is handled elsewhere)
			return @"";
		}

		if (! [launchedTask isRunning] && [launchedTask terminationStatus] != 0)
		{
			// Command failed, emit any stderr to the log
			NSData *stderrData = [stderrFileHandle readDataToEndOfFile];

			Rtt_Log("Error running %s %s", [cmd UTF8String], [[args description] UTF8String]);

			if ([stderrData length] > 0)
			{
				Rtt_Log("\t%s", (const char *)[stderrData bytes]);

				// Return stderr output
				result = [[[NSMutableString alloc] initWithData:stderrData encoding:NSUTF8StringEncoding] autorelease];
			}

            // Dump any stdout to the console
            NSData *stdoutData = [stdoutFileHandle readDataToEndOfFile];
			NSString *stdoutStr = [[[NSMutableString alloc] initWithData:stdoutData encoding:NSUTF8StringEncoding] autorelease];

			// If there was nothing on stderr, return whatever was on stdout
			if ([result isEqualToString:@""])
			{
				result = stdoutStr;
			}

			Rtt_Log("%s", [stdoutStr UTF8String]);
		}
        else
        {
            // Return stdout output
            NSData *data = [stdoutFileHandle readDataToEndOfFile];
            result = [[[NSMutableString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
        }
	}
	@catch( NSException* exception )
	{
		NSLog( @"launchTaskAndReturnOutput: exception %@ (%@ %@)", exception, cmd, args );
        result = [exception description];
	}
	@finally
	{
		[launchedTask release];
		launchedTask = nil;
	}

	result = [result stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

	return result;
}


// Force links to open in Safari (so that the Keychain gets set up appropriately)
- (BOOL)textView:(NSTextView *)aTextView
   clickedOnLink:(id)link
         atIndex:(NSUInteger)charIndex
{
#if defined(LAUNCH_URLS_WITH_SAFARI)
    if ([link isKindOfClass:[NSURL class]])
    {
        NSURL* url = (NSURL *) link;

        [[NSWorkspace sharedWorkspace] openURLs:@[url]
                        withAppBundleIdentifier:@"com.apple.Safari"
                                        options:NSWorkspaceLaunchDefault
                 additionalEventParamDescriptor:nil
                              launchIdentifiers:nil];

        return YES;
    }
    else
#endif
    if ([link isKindOfClass:[NSString class]])
    {
        NSString *string = (NSString *) link;

        if ([string hasPrefix:@"launch-bundle:"])
        {
			string = [string stringByReplacingOccurrencesOfString:@"launch-bundle:" withString:@""];
			NSArray *components = [string componentsSeparatedByString:@"|"];
			NSString *bundleId = [components firstObject];
			if(![[NSWorkspace sharedWorkspace] launchAppWithBundleIdentifier:bundleId
																	 options:NSWorkspaceLaunchDefault
											  additionalEventParamDescriptor:nil
															launchIdentifier:nil])
			{
				if([components count]>1) {
					[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[components objectAtIndex:1]]];
				}
			}
            
            return YES;
        }
    }

    return NO;
}

/**
 * @brief Limit the length of a string by summarizing it by taking characters from the beginning and end and joining them by an ellipsis
 * @param inString The object of our intentions
 * @param inLength The maximum length allowed (actually the result can be 3 characters longer than this to allow for the separator in the middle)
 * @returns NSString 3 characters longer than specified length
 */
- (NSString *) limitString:(NSString *)inString toLength:(size_t) inLength
{
    size_t len = [inString length];

    if (len < inLength)
    {
        return inString;
    }

    NSString *firstPart = [inString substringWithRange:[inString rangeOfComposedCharacterSequencesForRange:NSMakeRange(0, inLength/2)]];
    NSString *lastPart = [inString substringWithRange:[inString rangeOfComposedCharacterSequencesForRange:NSMakeRange(len - (inLength/2), (inLength/2))]];
    
    return [NSString stringWithFormat:@"%@ … %@", firstPart, lastPart];
}

/**
 * @brief Turn an NSString into an NSAttributed string with clickable links
 * @param string The object of our intentions
 * @returns NSAttributedString with clickable links
 */
- (NSAttributedString *)autoLinkURLs:(NSString *)string
{
    NSMutableAttributedString *linkedString = [[[NSMutableAttributedString alloc] initWithString:string] autorelease];

    NSDataDetector *detector = [NSDataDetector dataDetectorWithTypes:NSTextCheckingTypeLink error:nil];
    [detector enumerateMatchesInString:string options:0 range:NSMakeRange(0, string.length)
                            usingBlock:^(NSTextCheckingResult *match, NSMatchingFlags flags, BOOL *stop)
     {
         if (match.URL)
         {
             NSDictionary *attributes = @{ NSLinkAttributeName: match.URL };
             [linkedString addAttributes:attributes range:match.range];
         }
     }];

    return linkedString;
}

//
// formatBasicMarkdown
//
// Take a string with basic markdown formatting in it (links, bold, italics and code) and return an attributed string with
// the formatting implemented
//
- (NSAttributedString *)formatBasicMarkdown:(NSString *)string
{
    NSMutableAttributedString *formattedString = [[[NSMutableAttributedString alloc] initWithString:string] autorelease];

    NSError *error;
    NSFontManager *fontManager = [NSFontManager sharedFontManager];
    CGFloat fontSize = [NSFont systemFontSize];
    __block NSInteger offsetAdjustment = 0;

    // Regex searches non-greedily for things between asterisks, backquotes or underscores so they can be formatted
    // NSRegularExpression *regex = [[[NSRegularExpression alloc] initWithPattern:@"[*`_](.*?)[*`_]" options:NSRegularExpressionCaseInsensitive error:&error] autorelease];
    NSRegularExpression *regex = [[[NSRegularExpression alloc] initWithPattern:@"([\\_`*])((?:\\\\\\1|.)*?)\\1" options:NSRegularExpressionCaseInsensitive error:&error] autorelease];
    [regex enumerateMatchesInString:string options:0 range:NSMakeRange(0, [string length]) usingBlock:^(NSTextCheckingResult *match, NSMatchingFlags flags, BOOL *stop){
        Rtt_ASSERT([match numberOfRanges] == 3);
        NSRange matchRange = [match range];
        NSRange captureRange = [match rangeAtIndex:2];
        NSString *markDownStr = [string substringWithRange:match.range];
        NSFont *font = [NSFont userFontOfSize:fontSize];

        if ([markDownStr hasPrefix:@"`"])
        {
            font = [NSFont userFixedPitchFontOfSize:fontSize];
        }
        else if ([markDownStr hasPrefix:@"_"])
        {
            font = [fontManager convertFont:[NSFont userFontOfSize:fontSize] toHaveTrait:NSItalicFontMask];
        }
        else if ([markDownStr hasPrefix:@"*"])
        {
            font = [fontManager convertFont:[NSFont userFontOfSize:fontSize] toHaveTrait:NSBoldFontMask];
        }

        NSDictionary *attributes = @{ NSFontAttributeName: font };

        matchRange.location -= offsetAdjustment; // this accounts for the removal of the delimiters
        [formattedString addAttributes:attributes range:matchRange];
        [formattedString replaceCharactersInRange:matchRange withString:[string substringWithRange:captureRange]]; // remove markdown delimiters
        offsetAdjustment += 2;
    }];
    
    // Regex searches non-greedily for simple markdown links: [text of link](url of link)
    string = [[[formattedString string] copy] autorelease]; // get a fresh copy of the input
    offsetAdjustment = 0;
    regex = [[[NSRegularExpression alloc] initWithPattern:@"\\[(.*?)\\]\\((.*?)\\)" options:NSRegularExpressionCaseInsensitive error:&error] autorelease];
    [regex enumerateMatchesInString:string options:0 range:NSMakeRange(0, [string length]) usingBlock:^(NSTextCheckingResult *match, NSMatchingFlags flags, BOOL *stop){
        Rtt_ASSERT([match numberOfRanges] == 3);
        NSRange matchRange = [match range];
        NSRange textOfLinkRange = [match rangeAtIndex:1];
        NSRange urlOfLinkRange = [match rangeAtIndex:2];
        NSString *markDownStr = [string substringWithRange:match.range];
        NSString *linkStr = [string substringWithRange:textOfLinkRange];
        NSString *urlStr = [string substringWithRange:urlOfLinkRange];

        NSDictionary *attributes = @{ NSLinkAttributeName: urlStr };

        textOfLinkRange.location -= (offsetAdjustment + 1);
        matchRange.location -= offsetAdjustment;  // this accounts for the removal of the markdown
        [formattedString addAttributes:attributes range:textOfLinkRange];
        [formattedString replaceCharactersInRange:matchRange withString:linkStr]; // change markdown to link text
        offsetAdjustment += ([markDownStr length] - [linkStr length]);
    }];

#if 0
	// This finds too many false positives (e.g. "plugin.cc") so comment it out for now

    // Make any explicit links in the text clickable
    NSDataDetector *detector = [NSDataDetector dataDetectorWithTypes:NSTextCheckingTypeLink error:nil];
    [detector enumerateMatchesInString:[formattedString string] options:0 range:NSMakeRange(0, [formattedString string].length)
                            usingBlock:^(NSTextCheckingResult *match, NSMatchingFlags flags, BOOL *stop)
     {
         if (match.URL)
         {
             NSDictionary *attributes = @{ NSLinkAttributeName: match.URL };
             [formattedString addAttributes:attributes range:match.range];
         }
     }];
#endif // 0

    return formattedString;
}

/**
 * @brief Display an error in a modal sheet on the specified window
 * @param title Title of the error message
 * @param message A message to be displayed which can include basic Markdown syntax (bold, italic, code and links)
 * @param helpURL If present, causes a help button to be added to the dialog which will send the user to the specified URL when it is pushed
 * @param parentWindow The window to display the sheet on
 */
- (void) showError:(NSString *)title message:(NSString *)message helpURL:(NSString *)helpURL parentWindow:(NSWindow *)parentWindow
{
    Rtt_ASSERT(title != nil);
    Rtt_ASSERT(message != nil);
    Rtt_ASSERT(parentWindow != nil);

	Rtt_LogException("ERROR: %s: %s", [title UTF8String], [message UTF8String]);

    [self showModalSheet:title message:message buttonLabels:nil alertStyle:NSCriticalAlertStyle helpURL:helpURL parentWindow:parentWindow completionHandler:nil];
}

/**
 * @brief Display an error in a modal sheet on the specified window
 * @param title Title of the error message
 * @param message A message to be displayed which can include basic Markdown syntax (bold, italic, code and links)
 * @param helpURL If present, causes a help button to be added to the dialog which will send the user to the specified URL when it is pushed
 * @param parentWindow The window to display the sheet on
 */
- (void) showMessage:(NSString *)title message:(NSString *)message helpURL:(NSString *)helpURL parentWindow:(NSWindow *)parentWindow
{
    Rtt_ASSERT(title != nil);
    Rtt_ASSERT(message != nil);
    Rtt_ASSERT(parentWindow != nil);

    [self showModalSheet:title message:message buttonLabels:@[@"Continue"] alertStyle:NSInformationalAlertStyle helpURL:helpURL parentWindow:parentWindow completionHandler:nil];
}

/**
 * @brief Display an error in a modal sheet on the specified window
 * @param title Title of the error message
 * @param message A message to be displayed which can include basic Markdown syntax (bold, italic, code and links)
 * @param buttonLabels An array of strings for button labels
 * @param alertStyle The type of alert
 * @param helpURL If present, causes a help button to be added to the dialog which will send the user to the specified URL when it is pushed
 * @param parentWindow The window to display the sheet on
 * @completionHandler Block to be run when a button is pressed on the dialog
 * @attention if you supply a completion handler block it @b must call [NSApp stopModalWithCode:] in addition to any other things it does
 */
- (NSModalResponse) showModalSheet:(NSString *)title
                           message:(NSString *)message
                      buttonLabels:(NSArray *)buttonLabels
                        alertStyle:(NSAlertStyle)alertStyle
                           helpURL:(NSString *)helpURL
                      parentWindow:(NSWindow *)parentWindow
                 completionHandler:(void (^)(NSModalResponse returnCode))completionHandler
{
    NSAlert* alert = [[[NSAlert alloc] init] autorelease];
    NSString *errorMsg = [self limitString:message toLength:1500];
    NSTextView *accessory = [[NSTextView alloc] initWithFrame:NSMakeRect(0,0,400,0)];
    [accessory setFont:[NSFont systemFontOfSize:13]];
    [accessory insertText:[self formatBasicMarkdown:errorMsg]];
    [accessory setEditable:NO];
    [accessory setDrawsBackground:NO];
    [accessory setDelegate:self];

    // cancel any pending progress sheet
    [self endProgressSheetBanner:parentWindow];

    if (buttonLabels == nil)
    {
        buttonLabels = @[ @"OK" ];
    }

    for (NSString *buttonLabel : buttonLabels)
    {
        [alert addButtonWithTitle:buttonLabel];
    }

    // Clicks on the help button get handled in the AppDelegate by alertShowHelp:
    if (helpURL != nil)
    {
        [alert setShowsHelp:YES];
        [alert setHelpAnchor:helpURL];
        [alert setDelegate:(AppDelegate *)[NSApp delegate]];
    }

    [alert setAlertStyle:NSInformationalAlertStyle];
	[alert setMessageText:title];
    [alert setAccessoryView:accessory];

    // This makes use of a custom category on NSAlert to display the sheet truly modally
    // because [NSAlert beginSheetModalForWindow:completionHandler:] returns immediately
    // and that makes control-flow problematic

	// NOTE: This will throw a "[Layout] Detected missing constraints" warning in the debugger
	// apparently because we haven't set alert.informativeText.  However, if we do set that, even
	// to the empty string, the layout of the alert is compromised (we don't want informativeText
	// because we are using the accessoryView to achieve the same result more attractively)
    return [alert runModalSheetForWindow:parentWindow completionHandler:completionHandler];
}

- (void) saveBuildPreferences
{
    [appDelegate saveAppSpecificPreference:[self.platformName stringByAppendingString:@"AppName"] value:self.appName];
    [appDelegate saveAppSpecificPreference:[self.platformName stringByAppendingString:@"AppVersion"] value:self.appVersion];
    [appDelegate saveAppSpecificPreference:[self.platformName stringByAppendingString:@"DestFolder"] value:self.dstPath];
    [appDelegate saveAppSpecificPreference:[self.platformName stringByAppendingString:@"PostBuildAction"] value:[NSString stringWithFormat:@"%ld", (long)[postBuildRadioGroup selectedRow]]];

	// Remember the chosen post build action for next time we need a default
	[[NSUserDefaults standardUserDefaults] setInteger:[postBuildRadioGroup selectedRow] forKey:[NSString stringWithFormat:@"last%@PostBuildAction", self.platformName]];
}

- (void) restoreBuildPreferences
{
    self.appName = [appDelegate restoreAppSpecificPreference:[self.platformName stringByAppendingString:@"AppName"]
                                                defaultValue:[self.projectPath lastPathComponent]];
    self.appVersion = [appDelegate restoreAppSpecificPreference:[self.platformName stringByAppendingString:@"AppVersion"]
                                                   defaultValue:@"1.0"];
    self.dstPath = [appDelegate restoreAppSpecificPreference:[self.platformName stringByAppendingString:@"DestFolder"]
                                                defaultValue:[AppBuildController defaultDstDir]];

}


- (IBAction)stop:(id)sender
{
	appDelegate.stopRequested = YES;
	[NSApp stopModal];
}

- (void) stopTailDeviceSyslog
{
	@try
	{
		if ([tailSyslogTask isRunning])
		{
			[tailSyslogTask terminate];
			[tailSyslogTask release];
		}
	}
	@catch( NSException* exception )
	{
		NSLog( @"stopTailDeviceSyslog: exception %@", exception );
	}
}

- (void) startTailDeviceSyslog:(NSString *)deviceSyslogUtility appBundlePath:(NSString *)appBundlePath deviceID:(NSString *)deviceID
{
	Rtt_ASSERT([deviceSyslogUtility length] > 0);
	Rtt_ASSERT([appBundlePath length] > 0);
	Rtt_ASSERT(deviceID != nil);  // pass @"" if there's no device ID

	NSFileHandle *stdoutHandle = [[[NSFileHandle alloc] initWithFileDescriptor:fileno(stdout)] autorelease];

	// Stop any previously started task
	[self stopTailDeviceSyslog];

	tailSyslogTask = [[[NSTask alloc] init] retain];

	[tailSyslogTask setLaunchPath:deviceSyslogUtility];
	[tailSyslogTask setArguments:@[appBundlePath, deviceID]];

	// Allow the Simulator to capture the app's output
	[tailSyslogTask setStandardOutput:stdoutHandle];
	[tailSyslogTask setStandardError:stdoutHandle];
	[tailSyslogTask setTerminationHandler:^(NSTask *) {
		Rtt_Log("---------- Device Log Finished ----------");
	}];

	@try
	{
		Rtt_Log("----------  Device Log Starts  ----------");
		[tailSyslogTask launch];
	}
	@catch( NSException* exception )
	{
		NSLog( @"startTailSyslog: exception %@ (%@)", exception, deviceSyslogUtility);

		[tailSyslogTask release];
	}
}

@end

// ----------------------------------------------------------------------------
