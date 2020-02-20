//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

