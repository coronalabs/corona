//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "FoundationUtilities.h"

@implementation FoundationUtilities

+ (NSString*) bundleApplicationName
{
	NSString* bundledisplayname = [[[NSBundle mainBundle] localizedInfoDictionary] objectForKey:@"CFBundleDisplayName"];
	if(nil == bundledisplayname)
	{
		if(nil == bundledisplayname)
		{
			bundledisplayname = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDisplayName"];
			if(nil == bundledisplayname)
			{
				bundledisplayname = [[[NSBundle mainBundle] localizedInfoDictionary] objectForKey:@"CFBundleName"];
				if(nil == bundledisplayname)
				{
					bundledisplayname = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
				}
			}
		}
	}
	return bundledisplayname;
}

@end
