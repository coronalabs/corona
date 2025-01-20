//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "SampleCodeLocator.h"

#import "AppDelegate.h"

// ----------------------------------------------------------------------------

static SampleCodeLocator *sInstance = nil;

@interface SampleCodeLocator()
@property(nonatomic, assign) BOOL spotlightQueryIsRunning;
@property(nonatomic, assign) BOOL didFindSampleCodeDirectory;
@property(nonatomic, copy) NSString* sampleCodeDirectory;
- (NSString*) findSampleCodePath;
@end


@implementation SampleCodeLocator

@synthesize spotlightQueryIsRunning;
@synthesize didFindSampleCodeDirectory;
@synthesize sampleCodeDirectory;

+ (SampleCodeLocator*) sharedInstance
{
	@synchronized( self )
	{
		if ( ! sInstance )
		{
			sInstance = [[self alloc] init];
		}
	}

	return sInstance;
}

- (id)init
{
	self = [super init];
	if ( self )
	{
		metadataQuery = nil;
		spotlightQueryIsRunning = NO;
		didFindSampleCodeDirectory = NO;
		sampleCodeDirectory = nil;
		[self findSampleCodePath];
	}
	return self;
}

- (void)dealloc
{
	[sampleCodeDirectory release];
	[super dealloc];
}

- (NSString*)path
{
	// The spotlight query is asynchronous, but seems to be running stuff on the main thread.
	// So I can't sleep or busy-wait/spin to let the query finish because it will block the runloop
	// and prevent the query from finishing.
	// I found this article that said starting a new runloop seems to solve the problem.
	// http://confuseddevelopment.blogspot.com/2006/10/waiting-for-spotlight-query-to-finish.html
	if ( self.spotlightQueryIsRunning )
	{
		CFRunLoopRun();
	}

	return sampleCodeDirectory;
}

#pragma mark Spotlight support methods
- (void) spotlightForExampleDirectory
{
    NSMetadataQuery* metadata_query = [[NSMetadataQuery alloc] init];
	metadataQuery = metadata_query;

	// We use this file as the anchor or magnet file for Spotlight.
	NSString* predicate_string = @"kMDItemDisplayName == 'About_Corona_Sample_Code.txt'";
	
	NSPredicate* search_predicate = [NSPredicate predicateWithFormat:predicate_string]; 
    [metadata_query setPredicate:search_predicate];
    
	[metadata_query setSortDescriptors:[NSArray arrayWithObject:[[[NSSortDescriptor alloc] initWithKey:(id)kMDItemFSContentChangeDate ascending:NO] autorelease]]];
		
		
    [[NSNotificationCenter defaultCenter]
		addObserver:self
		selector:@selector(queryHandler:)
		name:NSMetadataQueryDidFinishGatheringNotification
		object:metadata_query];

	self.spotlightQueryIsRunning = YES;
    [metadata_query startQuery];
    // whatever comes now happens immediately:
}

// Runs a quick check to see if we can find a sample lua program from the sample code base path
// example expected string: /Volumes/CoronaSDK/Corona2.0-Beta5/SampleCode
- (BOOL) quickTestForExistanceWithSampleCodePath:(NSString*)sample_code_path
{
	static NSString* const kDefaultPathToCoronaExampleForQuickTestOfSampleDirectory = @"GettingStarted/HelloWorld/main.lua";

	NSString* relative_path_to_quick_test = kDefaultPathToCoronaExampleForQuickTestOfSampleDirectory;
	
	NSString* test_path = [sample_code_path stringByAppendingPathComponent:relative_path_to_quick_test];


	if([[NSFileManager defaultManager] fileExistsAtPath:test_path])
	{
		return YES;
	}
	else
	{
		return NO;
	}
}

- (void) queryHandler:(NSNotification*)the_notification
{
	// Assertion: metadataQuery == metadata_query
	NSMetadataQuery* metadata_query = [the_notification object];
	[metadata_query stopQuery];

	for(NSMetadataItem* metadata_item in [metadata_query results])
	{
/*
		NSLog(@"results: %@", [metadata_item attributes]);
		for(id an_attribute in [metadata_item attributes])
		{
			NSLog(@"an_attribute: %@, %@", an_attribute, [metadata_item valueForAttribute:an_attribute]);
		}
*/
		// Strange: kMDItemPath is not listed in the list of attributes, but we can ask for it directly
		// example expected string: /Volumes/CoronaSDK/Corona2.0-Beta8/SampleCode/About_Corona_Sample_Code.txt
		NSString* store_path = [[metadata_item valueForAttribute:(NSString*)kMDItemPath] stringByResolvingSymlinksInPath];
//		NSLog(@"store_path: %@", store_path);
		// example expected string: /Volumes/CoronaSDK/Corona2.0-Beta5/SampleCode
		NSString* spotlight_sample_code_dir = [store_path stringByDeletingLastPathComponent];
//		NSLog(@"spotlight sample_code dir: %@", spotlight_sample_code_dir);
		
		BOOL passed_test = [self quickTestForExistanceWithSampleCodePath:spotlight_sample_code_dir];
		if(YES == passed_test)
		{
//			NSLog(@"Odds are pretty good this is what we are looking for");
			didFindSampleCodeDirectory = YES;
			self.sampleCodeDirectory = spotlight_sample_code_dir;
			// Save to the cache
			AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
			[app_delegate setCachedSampleDirectoryPath:spotlight_sample_code_dir];

			break;
		}
		else
		{
			continue;
		}
						
	}

	[[NSNotificationCenter defaultCenter]
		removeObserver:self
		name:NSMetadataQueryDidFinishGatheringNotification
		object:metadata_query];

	[metadataQuery release];
	metadataQuery = nil;
	metadata_query = nil;
	
	self.spotlightQueryIsRunning = NO;
	// The spotlight query is asynchronous, but seems to be running stuff on the main thread.
	// So I can't sleep or busy-wait/spin to let the query finish because it will block the runloop
	// and prevent the query from finishing.
	// I found this article that said starting a new runloop seems to solve the problem.
	// http://confuseddevelopment.blogspot.com/2006/10/waiting-for-spotlight-query-to-finish.html
	// This part will stop the CFRunloop if it is running and allow the other section that is waiting
	// on it to resume.	
	CFRunLoopStop(CFRunLoopGetCurrent());


}

// Uses Spotlight in the bad case, so we want to call early to minimize user waiting
// If nil is returned, a spotlight query is running and you have to get the value later.
- (NSString*) findSampleCodePath
{
	// Check if we already found the path
	AppDelegate* app_delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	NSString* cached_sample_dir_path = [app_delegate cachedSampleDirectoryPath];
	if(nil != cached_sample_dir_path)
	{
		self.sampleCodeDirectory = cached_sample_dir_path;
		return cached_sample_dir_path;
	}
	
	
	NSString* path_to_sample_code = nil;
	NSString* corona_simulator_path = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
	NSFileManager* file_manager = [NSFileManager defaultManager];
	path_to_sample_code = [corona_simulator_path stringByAppendingPathComponent:@"SampleCode"];
	BOOL is_directory = NO;


	if([file_manager fileExistsAtPath:path_to_sample_code isDirectory:&is_directory] && YES == is_directory)
	{
	
		BOOL passed_test = [self quickTestForExistanceWithSampleCodePath:path_to_sample_code];
		if(YES == passed_test)
		{
//			NSLog(@"We found the sample code directory: %@", path_to_sample_code);
			// save the path to the cache
			[app_delegate setCachedSampleDirectoryPath:path_to_sample_code];
			self.sampleCodeDirectory = path_to_sample_code;
			return path_to_sample_code;
		}
		else
		{
//			NSLog(@"We did not find the sample code directory, try something else");
			[self spotlightForExampleDirectory];
		}
	}
	else
	{
//		NSLog(@"We did not find the sample code directory, try something else");
		[self spotlightForExampleDirectory];
		
	}
	return nil;
}

- (NSString*) examplePathFromFragment:(NSString*)fragment_string
{
	return [self.sampleCodeDirectory stringByAppendingPathComponent:fragment_string];
}

@end

// ----------------------------------------------------------------------------

