//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
