//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "ThemeConsole.h"
#import "Theme.h"

@implementation ThemeConsole

@synthesize fontSize, timestampBgColor, textBgColor, fontName, errorFgColor, timestampFgColor, errorBgColor, warningBgColor, textFgColor, warningFgColor;

+ (NSArray*) ThemeConsoleWithArray: (NSArray*) array
{
    NSMutableArray * resultsArray = [[NSMutableArray alloc] init];

    for(id entry in array)
    {
        if(![entry isKindOfClass: [NSDictionary class]]) continue;

        [resultsArray addObject: [ThemeConsole ThemeConsoleWithDictionary: entry]];
    }

    return JSONAutoRelease(resultsArray);
} // End of ThemeConsoleWithArray

+ (ThemeConsole *)ThemeConsoleWithDictionary:(NSDictionary *)dictionary
{
    ThemeConsole *instance = [[ThemeConsole alloc] initWithDictionary: dictionary];
    return JSONAutoRelease(instance);
} // End of ThemeConsoleWithDictionary

+ (ThemeConsole *)ThemeConsoleWithJSONString: (NSString *) jsonString usingEncoding: (NSStringEncoding) stringEncoding error: (NSError**) error
{
    NSData * jsonData = [jsonString dataUsingEncoding: stringEncoding];

    NSDictionary * jsonDictionary = [NSJSONSerialization JSONObjectWithData: jsonData
                                                                    options: 0
                                                                      error: error];

    if(nil != error && nil != jsonDictionary)
    {
        return [ThemeConsole ThemeConsoleWithDictionary: jsonDictionary];
    }

    return nil;
} // End of ThemeConsoleWithJSONString

- (id)initWithDictionary:(NSDictionary *)dictionary
{
    self = [super init];
    if(self)
    {
        if(nil != [dictionary objectForKey: @"fontSize"])
        {
            self.fontSize = [dictionary objectForKey: @"fontSize"];
        }

        if(nil != [dictionary objectForKey: @"timestampBgColor"])
        {
            self.timestampBgColor = [dictionary objectForKey: @"timestampBgColor"];
        }

        if(nil != [dictionary objectForKey: @"textBgColor"])
        {
            self.textBgColor = [dictionary objectForKey: @"textBgColor"];
        }

        if(nil != [dictionary objectForKey: @"fontName"])
        {
            self.fontName = [dictionary objectForKey: @"fontName"];
        }

        if(nil != [dictionary objectForKey: @"errorFgColor"])
        {
            self.errorFgColor = [dictionary objectForKey: @"errorFgColor"];
        }

        if(nil != [dictionary objectForKey: @"timestampFgColor"])
        {
            self.timestampFgColor = [dictionary objectForKey: @"timestampFgColor"];
        }

        if(nil != [dictionary objectForKey: @"errorBgColor"])
        {
            self.errorBgColor = [dictionary objectForKey: @"errorBgColor"];
        }

        if(nil != [dictionary objectForKey: @"warningBgColor"])
        {
            self.warningBgColor = [dictionary objectForKey: @"warningBgColor"];
        }

        if(nil != [dictionary objectForKey: @"textFgColor"])
        {
            self.textFgColor = [dictionary objectForKey: @"textFgColor"];
        }

        if(nil != [dictionary objectForKey: @"warningFgColor"])
        {
            self.warningFgColor = [dictionary objectForKey: @"warningFgColor"];
        }


    }

    return self;
} // End of initWithDictionary

- (NSString*) description
{
    NSMutableString * descriptionOutput = [[NSMutableString alloc] init];
    [descriptionOutput appendFormat: @"fontSize = \"%@\"\r\n", fontSize];
    [descriptionOutput appendFormat: @"timestampBgColor = \"%@\"\r\n", timestampBgColor];
    [descriptionOutput appendFormat: @"textBgColor = \"%@\"\r\n", textBgColor];
    [descriptionOutput appendFormat: @"fontName = \"%@\"\r\n", fontName];
    [descriptionOutput appendFormat: @"errorFgColor = \"%@\"\r\n", errorFgColor];
    [descriptionOutput appendFormat: @"timestampFgColor = \"%@\"\r\n", timestampFgColor];
    [descriptionOutput appendFormat: @"errorBgColor = \"%@\"\r\n", errorBgColor];
    [descriptionOutput appendFormat: @"warningBgColor = \"%@\"\r\n", warningBgColor];
    [descriptionOutput appendFormat: @"textFgColor = \"%@\"\r\n", textFgColor];
    [descriptionOutput appendFormat: @"warningFgColor = \"%@\"\r\n", warningFgColor];

    return JSONAutoRelease(descriptionOutput);
} // End of description

- (void)dealloc
{
#if ! __has_feature(objc_arc)
	[fontSize release];
	[timestampBgColor release];
	[textBgColor release];
	[fontName release];
	[errorFgColor release];
	[timestampFgColor release];
	[errorBgColor release];
	[warningBgColor release];
	[textFgColor release];
	[warningFgColor release];

    [super dealloc];
#endif
} // End of dealloc

@end
