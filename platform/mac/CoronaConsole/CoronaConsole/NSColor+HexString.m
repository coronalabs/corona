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

#import "NSColor+HexString.h"

@implementation NSColor(HexString)

+ (NSColor *) colorWithHexString: (NSString *) hexString
{
	NSString *colorString = [[hexString stringByReplacingOccurrencesOfString: @"#" withString: @""] uppercaseString];
	CGFloat alpha, red, blue, green;
	switch ([colorString length])
	{
		case 3: // RGB
			alpha = 1.0f;
			red   = [self colorComponentFrom: colorString start: 0 length: 1];
			green = [self colorComponentFrom: colorString start: 1 length: 1];
			blue  = [self colorComponentFrom: colorString start: 2 length: 1];
			break;
		case 4: // ARGB
			alpha = [self colorComponentFrom: colorString start: 0 length: 1];
			red   = [self colorComponentFrom: colorString start: 1 length: 1];
			green = [self colorComponentFrom: colorString start: 2 length: 1];
			blue  = [self colorComponentFrom: colorString start: 3 length: 1];
			break;
		case 6: // RRGGBB
			alpha = 1.0f;
			red   = [self colorComponentFrom: colorString start: 0 length: 2];
			green = [self colorComponentFrom: colorString start: 2 length: 2];
			blue  = [self colorComponentFrom: colorString start: 4 length: 2];
			break;
		case 8: // AARRGGBB
			alpha = [self colorComponentFrom: colorString start: 0 length: 2];
			red   = [self colorComponentFrom: colorString start: 2 length: 2];
			green = [self colorComponentFrom: colorString start: 4 length: 2];
			blue  = [self colorComponentFrom: colorString start: 6 length: 2];
			break;
		default:
			[NSException raise:@"Invalid color value" format: @"Color value %@ is invalid.  It should be a hex value of the form #RBG, #ARGB, #RRGGBB, or #AARRGGBB", hexString];
			break;
	}
	return [NSColor colorWithRed: red green: green blue: blue alpha: alpha];
}

+ (CGFloat) colorComponentFrom: (NSString *) string start: (NSUInteger) start length: (NSUInteger) length
{
	NSString *substring = [string substringWithRange: NSMakeRange(start, length)];
	NSString *fullHex = length == 2 ? substring : [NSString stringWithFormat: @"%@%@", substring, substring];
	unsigned hexComponent;
	[[NSScanner scannerWithString: fullHex] scanHexInt: &hexComponent];
	return hexComponent / 255.0;
}

@end

