//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "IntegerValidationFormatter.h"


@implementation IntegerValidationFormatter

- (BOOL)isPartialStringValid:(NSString**)partialStringPtr
       proposedSelectedRange:(NSRangePointer)proposedSelRangePtr
              originalString:(NSString*)origString
       originalSelectedRange:(NSRange)origSelRange
            errorDescription:(NSString**)error
{
	if( [*partialStringPtr length] > 0 )
	{
		// This will return the number set 0 through 9
		NSCharacterSet* digitchars = [NSCharacterSet decimalDigitCharacterSet];
		NSScanner* scanner = [NSScanner scannerWithString:*partialStringPtr];
		// This will scan through the string until it encounters a non-digit character.
		// If the scanner reaches the end of the string, it means it is all numbers and thus valid.
		// Otherwise, we should reject the string.
		[scanner scanCharactersFromSet:digitchars intoString:NULL];
		if( [scanner isAtEnd] )
		{
			return YES;
		}
		else
		{
			// Should I beep?
			NSBeep();
			return NO;
		}
	}
	else
	{
		return YES;
	}
}

@end
