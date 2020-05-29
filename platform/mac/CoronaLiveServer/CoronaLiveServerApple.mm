//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaLiveServerApple.h"
#include "CoronaLiveServerCore.h"
#include "CoronaHTTPServer.h"


@interface LiveServer ()

-(void)update:(ConstFSEventStreamRef)source;

@property (assign, nonatomic) Rtt_LiveServer* server;
@property (strong, nonatomic) NSNetService* announcer;
@property (strong, nonatomic) NSMutableDictionary< NSValue*, NSString* >* watchers;

@end


void FilesWasChangedInDir(ConstFSEventStreamRef eventStream,
										   void *clientCallBackInfo,
										   size_t numEvents,
										   void *eventPaths,
										   const FSEventStreamEventFlags eventFlags[],
										   const FSEventStreamEventId eventIds[])
{
	char **filePaths = (char **) eventPaths;

	BOOL needsReload = NO;

	for (size_t i = 0; i < numEvents; i++)
	{
		// We only care about files
		if (eventFlags[i] & (kFSEventStreamEventFlagItemIsFile | kFSEventStreamEventFlagItemIsDir | kFSEventStreamEventFlagItemIsSymlink))
		{
			BOOL skip = NO;
			NSArray *pathComponents = [[NSString stringWithUTF8String:filePaths[i]] pathComponents];

			// If any component of the path starts with a dot or ends with various uninteresting
			// extensions, skip the path
			for (NSString *component in pathComponents)
			{
				if ([component hasPrefix:@"."] ||
					[component hasSuffix:@".bak"] ||
					[component hasSuffix:@".tmp"] ||
					[component hasSuffix:@".orig"])
				{
					skip = YES;

					break;
				}
			}

			if (skip)
			{
				continue;
			}

			// Any of these events could affect the project (or the file's availability to it)
			if (eventFlags[i] & kFSEventStreamEventFlagItemCreated ||
				eventFlags[i] & kFSEventStreamEventFlagItemRemoved ||
				eventFlags[i] & kFSEventStreamEventFlagItemRenamed ||
				eventFlags[i] & kFSEventStreamEventFlagItemModified ||
				eventFlags[i] & kFSEventStreamEventFlagItemInodeMetaMod ||
				eventFlags[i] & kFSEventStreamEventFlagItemXattrMod ||
				eventFlags[i] & kFSEventStreamEventFlagItemChangeOwner)
			{
				needsReload = YES;

				break;
			}
		}
	}

	if (needsReload)
	{
		LiveServer* vs = (__bridge LiveServer *)(clientCallBackInfo);
		[vs update:eventStream];
	}
}

FSEventStreamRef FileWatch_StartMonitoringFolder(NSString* folderPath,
									 FSEventStreamCallback callbackFunction,
									 FSEventStreamContext* callbackUserData)
{
	// These exclusions help maintain sanity if we're asked to open a main.lua in the user's home directory
	NSArray *foldersToExclude = @[@"Library/", @"System/", @".Trash/",];
	NSArray *pathsToWatch = [NSArray arrayWithObject:folderPath];
	NSMutableArray *pathsToExclude = [[NSMutableArray alloc] initWithCapacity:[foldersToExclude count]];

	for (NSString *folder in foldersToExclude)
	{
		[pathsToExclude addObject:[NSString pathWithComponents:@[folderPath, folder]]];
	}

	FSEventStreamRef eventStream = NULL;
	CFAbsoluteTime timeLatency = 0.5; /* Latency in seconds */

	// Ask for all file events
	eventStream = FSEventStreamCreate(
									  NULL,
									  callbackFunction,
									  callbackUserData,
									  (__bridge CFArrayRef) pathsToWatch,
									  kFSEventStreamEventIdSinceNow,
									  timeLatency,
									  kFSEventStreamCreateFlagFileEvents
									  );

	if (eventStream != NULL)
	{
		FSEventStreamSetExclusionPaths(eventStream, (__bridge CFArrayRef) pathsToExclude);

		FSEventStreamScheduleWithRunLoop(eventStream, [[NSRunLoop currentRunLoop] getCFRunLoop], kCFRunLoopDefaultMode);

		FSEventStreamStart(eventStream);
	}
	return eventStream;
}





@implementation LiveServer


- (instancetype)init
{
	self = [super init];
	if (self) {
		self.server = NULL;
		self.announcer = NULL;
		self.watchers = [[NSMutableDictionary alloc] init];
	}
	return self;
}


-(BOOL)addProject:(NSString *)path
{
	if(!path)
		return NO;

	if(self.server == nil)
		self.server = new Rtt_LiveServer();

	int port = self.server->Add(path.UTF8String);
	if(port < 0) {
		return NO;
	}
	if(port > 0 && self.announcer == nil) {
		self.announcer = [[NSNetService alloc] initWithDomain:@"local" type:@"_corona_live._tcp." name:@"" port:port];
		[self.announcer publish];
	}

	if([[self.watchers allKeysForObject:path] count] == 0)
	{
		FSEventStreamContext callback_info =
		{
			0,
			(__bridge void * _Nullable)(self),
			CFRetain,
			CFRelease,
			NULL
		};
		FSEventStreamRef fileEvents = FileWatch_StartMonitoringFolder(path, FilesWasChangedInDir, &callback_info);
		[self.watchers setObject:path forKey:[NSValue valueWithPointer:fileEvents]];
	}
	return YES;

}

-(void)stopProject:(NSString *)path
{
	if(self.server && path) {
		self.server->Remove(path.UTF8String);

		for (NSValue *key in [self.watchers allKeysForObject:path]) {
			FSEventStreamRef fsstream = (FSEventStreamRef)key.pointerValue;
			FSEventStreamStop(fsstream);
			FSEventStreamInvalidate(fsstream);
			FSEventStreamRelease(fsstream);
			[self.watchers removeObjectForKey:key];
		}
	}
}

- (void)dealloc
{
	[self.watchers enumerateKeysAndObjectsUsingBlock:^(NSValue* key, NSString* obj, BOOL* stop) {
		FSEventStreamRef fsstream = (FSEventStreamRef)key.pointerValue;
		FSEventStreamStop(fsstream);
		FSEventStreamInvalidate(fsstream);
		FSEventStreamRelease(fsstream);
	}];
	self.watchers = nil;

	[self.announcer stop];
	self.announcer = nil;

	if(self.server) {
		delete self.server;
		self.server = NULL;
	}
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

-(void)update:(ConstFSEventStreamRef)source
{
	NSString *path = [self.watchers objectForKey:[NSValue valueWithPointer:source]];
	if(path) {
		self.server->Update(path.UTF8String);
	}
}

@end


@interface LiveHTTPServer ()

@property (assign, nonatomic) Rtt_HTTPServer* server;

@end


@implementation LiveHTTPServer


- (instancetype)init
{
	self = [super init];
	if (self) {
		self.server = NULL;
	}
	return self;
}


-(void)setPath:(NSString *)path
{
	if(!path)
		return;

	if(self.server == nil)
		self.server = new Rtt_HTTPServer();

	self.server->Set(path.UTF8String);
}

-(NSURL*)getServerURL
{
	if(self.server == nil)
		return nil;
	std::string url = self.server->GetServerAddress();

	if(url.empty())
		return nil;

	return [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
}

- (void)dealloc
{
	if(self.server) {
		delete self.server;
		self.server = NULL;
	}
#if !__has_feature(objc_arc)
	[super dealloc];
#endif
}

@end

