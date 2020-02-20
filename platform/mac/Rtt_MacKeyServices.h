//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>

// Key codes not found in Carbon.
enum KeyCodes
{
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 101200
	kVK_RightCommand = 0x36,
#endif
	kVK_Menu = 0x6E,
	kVK_Back = 0x7F,
};

@interface MacKeyServices : NSObject

+ (NSString*)getNameForKey:(NSNumber*)keyCode;

+ (NSUInteger)getModifierMaskForKey:(unsigned short)keyCode;

@end
