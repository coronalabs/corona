//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>

@class NSMetadataQuery;

// ----------------------------------------------------------------------------

@interface SampleCodeLocator : NSObject
{
	NSMetadataQuery *metadataQuery;
	BOOL spotlightQueryIsRunning;
	BOOL didFindSampleCodeDirectory;
	NSString* sampleCodeDirectory;
}

+ (SampleCodeLocator*) sharedInstance;

- (NSString*)path;

@end

// ----------------------------------------------------------------------------

