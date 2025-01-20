//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>

@interface SDKList : NSObject <NSCoding, NSCopying>

@property (nonatomic, retain) NSArray *SDKItems;

+ (instancetype)modelObjectWithDictionary:(NSDictionary *)dict;
- (instancetype)initWithDictionary:(NSDictionary *)dict platform:(NSString *)platform;
- (NSDictionary *)dictionaryRepresentation;

@end

//
//  SDKItem
//

@interface SDKItem : NSObject <NSCoding, NSCopying>

@property (nonatomic, retain) NSString *label;
@property (nonatomic, retain) NSString *xcodeVersion;
@property (nonatomic, assign) double coronaVersion;
@property (nonatomic, assign) BOOL beta;
@property (nonatomic, retain) NSString *failMessage;
@property (nonatomic, retain) NSString *customTemplate;

+ (instancetype)modelObjectWithDictionary:(NSDictionary *)dict;
- (instancetype)initWithDictionary:(NSDictionary *)dict;
- (NSDictionary *)dictionaryRepresentation;

@end
