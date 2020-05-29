//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>

#if ! __has_feature(objc_arc)
	#define JSONAutoRelease(param) ([param autorelease]);
#else
	#define JSONAutoRelease(param) (param)
#endif

@class ThemeConsole;

@interface Theme : NSObject
{
} // End of Theme

+ (NSArray*) ThemeWithArray: (NSArray*) array;
+ (Theme *)ThemeWithDictionary: (NSDictionary *) dictionary;
+ (Theme *)ThemeWithJSONString: (NSString *) jsonString usingEncoding: (NSStringEncoding) stringEncoding error: (NSError**) error;
- (id)initWithDictionary: (NSDictionary *) dictionary;
- (NSString*) description;

@property(nonatomic, retain) ThemeConsole * console;

@end
