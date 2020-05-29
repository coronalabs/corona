//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>


@interface ThemeConsole : NSObject
{
} // End of Console

+ (NSArray*) ThemeConsoleWithArray: (NSArray*) array;
+ (ThemeConsole *)ThemeConsoleWithDictionary: (NSDictionary *) dictionary;
+ (ThemeConsole *)ThemeConsoleWithJSONString: (NSString *) jsonString usingEncoding: (NSStringEncoding) stringEncoding error: (NSError**) error;
- (id)initWithDictionary: (NSDictionary *) dictionary;
- (NSString*) description;

@property(nonatomic, retain) NSNumber * fontSize;
@property(nonatomic, retain) NSString * timestampBgColor;
@property(nonatomic, retain) NSString * textBgColor;
@property(nonatomic, retain) NSString * fontName;
@property(nonatomic, retain) NSString * errorFgColor;
@property(nonatomic, retain) NSString * timestampFgColor;
@property(nonatomic, retain) NSString * errorBgColor;
@property(nonatomic, retain) NSString * warningBgColor;
@property(nonatomic, retain) NSString * textFgColor;
@property(nonatomic, retain) NSString * warningFgColor;

@end
