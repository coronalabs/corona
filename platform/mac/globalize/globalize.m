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

#import <Foundation/Foundation.h>

// Usage: ./globalize domain key value

int
main (int argc, const char * argv[])
{
	int result = 1; // default to failure

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	if ( 3 == argc || 4 == argc )
	{
		const char *d = argv[1];
		const char *k = argv[2];
		const char *v = ( 4 == argc ? argv[3] : NULL );

		NSArray *paths = NSSearchPathForDirectoriesInDomains( NSLibraryDirectory, NSLocalDomainMask, true );
		NSString *prefDir = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences"];

		NSString *domain = [NSString stringWithUTF8String:d];
		NSString *prefFile = [domain stringByAppendingPathExtension:@"plist"];
		NSString *prefPath = [prefDir stringByAppendingPathComponent:prefFile];

		if ( prefPath )
		{
			NSString *key = [NSString stringWithUTF8String:k];
			NSString *value = v ? [NSString stringWithUTF8String:v] : nil;

			BOOL success = NO;

			if ( [[NSFileManager defaultManager] fileExistsAtPath:prefPath] )
			{
				// Modify existing file
				NSMutableDictionary *root = [NSMutableDictionary dictionaryWithContentsOfFile:prefPath];
				if ( value )
				{
					[root setValue:value forKey:key];
				}
				else
				{
					[root removeObjectForKey:key];
				}
				success = [root writeToFile:prefPath atomically:YES];
			}
			else
			{
				if ( value )
				{
					// Create file
					NSDictionary *root = [NSDictionary dictionaryWithObject:value forKey:key];
					success = [root writeToFile:prefPath atomically:YES];
				}
			}

			if ( success )
			{
				result = 0;
			}
		}
	}

	printf( "%d", result );

    [pool drain];
    return result;
}
