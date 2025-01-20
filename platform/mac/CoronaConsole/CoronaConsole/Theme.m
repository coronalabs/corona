//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "Theme.h"
#import "ThemeConsole.h"

@implementation Theme

@synthesize console;

+ (NSArray*) ThemeWithArray: (NSArray*) array
{
    NSMutableArray * resultsArray = [[NSMutableArray alloc] init];

    for(id entry in array)
    {
        if(![entry isKindOfClass: [NSDictionary class]]) continue;

        [resultsArray addObject: [Theme ThemeWithDictionary: entry]];
    }

    return JSONAutoRelease(resultsArray);
} // End of ThemeWithArray

+ (Theme *)ThemeWithDictionary:(NSDictionary *)dictionary
{
    Theme *instance = [[Theme alloc] initWithDictionary: dictionary];
    return JSONAutoRelease(instance);
} // End of ThemeWithDictionary

+ (Theme *)ThemeWithJSONString: (NSString *) jsonString usingEncoding: (NSStringEncoding) stringEncoding error: (NSError**) error
{
    NSData * jsonData = [jsonString dataUsingEncoding: stringEncoding];

    NSDictionary * jsonDictionary = [NSJSONSerialization JSONObjectWithData: jsonData
                                                                    options: 0
                                                                      error: error];

    if(nil != error && nil != jsonDictionary)
    {
        return [Theme ThemeWithDictionary: jsonDictionary];
    }

    return nil;
} // End of ThemeWithJSONString

- (id)initWithDictionary:(NSDictionary *)dictionary
{
    self = [super init];
    if(self)
    {
        if(nil != [dictionary objectForKey: @"console"])
        {
            self.console = [ThemeConsole ThemeConsoleWithDictionary: [dictionary objectForKey: @"console"]];
        }


    }

    return self;
} // End of initWithDictionary

- (NSString*) description
{
    NSMutableString * descriptionOutput = [[NSMutableString alloc] init];
    [descriptionOutput appendFormat: @"console: { %@ }\r\n", console];

    return JSONAutoRelease(descriptionOutput);
} // End of description

- (void)dealloc
{
#if ! __has_feature(objc_arc)
	[console release];

    [super dealloc];
#endif
} // End of dealloc

@end
