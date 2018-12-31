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

#import "FileWatch.h"
#import "AppDelegate.h"

FSEventStreamRef FileWatch_StartMonitoringFolder(NSString* folderPath,
												 FSEventStreamCallback callbackFunction,
												 FSEventStreamContext* callbackUserData)
{
	if (nil == folderPath)
	{
		return NULL;
	}

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
		(CFArrayRef) pathsToWatch,
		kFSEventStreamEventIdSinceNow,
		timeLatency,
		kFSEventStreamCreateFlagFileEvents
	);

	if (eventStream != NULL)
	{
		FSEventStreamSetExclusionPaths(eventStream, (CFArrayRef) pathsToExclude);

		FSEventStreamScheduleWithRunLoop(eventStream, [[NSRunLoop currentRunLoop] getCFRunLoop], kCFRunLoopDefaultMode);

		FSEventStreamStart(eventStream);
	}

	return eventStream;
}

void FileWatch_StopMonitoringFolder(FSEventStreamRef eventStream)
{
	if (NULL != eventStream)
	{
		FSEventStreamStop(eventStream);
		FSEventStreamInvalidate(eventStream);
		FSEventStreamRelease(eventStream);
	}
}

void FolderChangeNotificationDelegate_LuaFolderChangedCallbackFunction(ConstFSEventStreamRef eventStream,
																	   void *clientCallBackInfo,
																	   size_t numEvents,
																	   void *eventPaths,
																	   const FSEventStreamEventFlags eventFlags[],
																	   const FSEventStreamEventId eventIds[])
{
	char **filePaths = (char **) eventPaths;

#if defined(Rtt_DEBUG)
	NSLog(@"FolderChangeNotificationDelegate: Callback called, numEvents=%ld\n", numEvents);
	for (size_t i = 0; i < numEvents; i++)
	{
		printf("  change %llu in '%s', flags 0x%08x\n", eventIds[i], filePaths[i], (unsigned int) eventFlags[i]);
	}
#endif

	id appDelegate = (id) clientCallBackInfo;
	BOOL needsReload = NO;

	for (size_t i = 0; i < numEvents; i++)
	{
		// We only care about files
		if (eventFlags[i] & kFSEventStreamEventFlagItemIsFile)
		{
			BOOL skip = NO;
			NSArray *pathComponents = [[NSString stringWithExternalString:filePaths[i]] pathComponents];

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
		[appDelegate promptAndRelaunchSimulatorIfUserApproves];
	}
}
