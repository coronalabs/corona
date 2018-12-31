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
