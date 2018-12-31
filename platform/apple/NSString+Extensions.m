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