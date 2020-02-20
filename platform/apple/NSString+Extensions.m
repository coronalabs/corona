//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "NSString+Extensions.h"

@implementation NSString (Extensions)

//
// Use in preference to [NSString stringWithUTF8String:] because it handles unexpected encodings better
// and [NSString stringWithUTF8String:] returns nil when it fails which is unhelpful
//
+ (_Nonnull instancetype) stringWithExternalString:(const char  * _Nullable) nullTerminatedCString
{
	NSString *str = nil;

	if (nullTerminatedCString == NULL)
	{
#ifdef  Rtt_DEBUG
		NSLog(@"stringWithExternalString: null C string\n%@", [NSThread callStackSymbols]);
#endif
		
		return @"";
	}

	str = [NSString stringWithUTF8String:nullTerminatedCString];

	if (str == nil)
	{
		str = [NSString stringWithCString:nullTerminatedCString encoding:NSMacOSRomanStringEncoding]; // OS X centric
	}

	if (str == nil)
	{
		str = [NSString stringWithCString:nullTerminatedCString encoding:NSASCIIStringEncoding]; // lossy
	}

	if (str == nil)
	{
		str = @"[unrecognized string encoding]";
	}

	return str;
}

- (BOOL)contains:(NSString *)str
{
	NSRange range = [self rangeOfString:str];
	return (range.location != NSNotFound);
}

@end