//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "SDKList.h"

NSString *const kSDKListSDKItems = @"SDKItems";

@interface SDKList ()

- (id)objectOrNilForKey:(id)aKey fromDictionary:(NSDictionary *)dict;

@end

@implementation SDKList

@synthesize SDKItems = _SDKItems;


+ (instancetype)modelObjectWithDictionary:(NSDictionary *)dict
{
    return [[self alloc] initWithDictionary:dict];
}

- (instancetype)initWithDictionary:(NSDictionary *)dict platform:(NSString *)platform
{
	self = [super init];

	// This check serves to make sure that a non-NSDictionary object
	// passed into the model class doesn't break the parsing.
	if (self && [dict isKindOfClass:[NSDictionary class]])
	{
		NSObject *receivedSDKItem = [dict objectForKey:platform];
		NSMutableArray *parsedSDKItem = [NSMutableArray array];
		if ([receivedSDKItem isKindOfClass:[NSArray class]])
		{
			for (NSDictionary *item in (NSArray *)receivedSDKItem)
			{
				if ([item isKindOfClass:[NSDictionary class]])
				{
					[parsedSDKItem addObject:[SDKItem modelObjectWithDictionary:item]];
				}
			}
		}
		else if ([receivedSDKItem isKindOfClass:[NSDictionary class]])
		{
			[parsedSDKItem addObject:[SDKItem modelObjectWithDictionary:(NSDictionary *)receivedSDKItem]];
		}

		self.SDKItems = [NSArray arrayWithArray:parsedSDKItem];

	}

	return self;

}

- (NSDictionary *)dictionaryRepresentation
{
	NSMutableDictionary *mutableDict = [NSMutableDictionary dictionary];
	NSMutableArray *tempArrayForSDKItems = [NSMutableArray array];
	for (NSObject *subArrayObject in self.SDKItems)
	{
		if ([subArrayObject respondsToSelector:@selector(dictionaryRepresentation)])
		{
			// This class is a model object
			[tempArrayForSDKItems addObject:[subArrayObject performSelector:@selector(dictionaryRepresentation)]];
		}
		else
		{
			// Generic object
			[tempArrayForSDKItems addObject:subArrayObject];
		}
	}
	[mutableDict setValue:[NSArray arrayWithArray:tempArrayForSDKItems] forKey:kSDKListSDKItems];

	return [NSDictionary dictionaryWithDictionary:mutableDict];
}

- (NSString *)description 
{
    return [NSString stringWithFormat:@"%@", [self dictionaryRepresentation]];
}

#pragma mark - SDKList Helper Method
- (id)objectOrNilForKey:(id)aKey fromDictionary:(NSDictionary *)dict
{
    id object = [dict objectForKey:aKey];
    return [object isEqual:[NSNull null]] ? nil : object;
}


#pragma mark - SDKList NSCoding Methods

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super init];

    self.SDKItems = [aDecoder decodeObjectForKey:kSDKListSDKItems];
    return self;
}

- (void)encodeWithCoder:(NSCoder *)aCoder
{

    [aCoder encodeObject:_SDKItems forKey:kSDKListSDKItems];
}

- (id)copyWithZone:(NSZone *)zone
{
	SDKList *copy = [[SDKList alloc] init];

	if (copy)
	{

		copy.SDKItems = [self.SDKItems copyWithZone:zone];
	}

	return copy;
}


- (void)dealloc
{
    [_SDKItems release];
    [super dealloc];
}

@end

#pragma mark - SDKItem

NSString *const kSDKItemLabel = @"label";
NSString *const kSDKItemXcodeVersion = @"xcodeVersion";
NSString *const kSDKItemCoronaVersion = @"coronaVersion";
NSString *const kIosBeta = @"beta";
NSString *const kIosFailMessage = @"failMessage";
NSString *const kCustomTemplate = @"customTemplate";

@interface SDKItem ()

- (id)objectOrNilForKey:(id)aKey fromDictionary:(NSDictionary *)dict;

@end

@implementation SDKItem

@synthesize label = _label;
@synthesize xcodeVersion = _xcodeVersion;
@synthesize coronaVersion = _coronaVersion;
@synthesize beta = _beta;
@synthesize failMessage = _failMessage;
@synthesize customTemplate = _customTemplate;

+ (instancetype)modelObjectWithDictionary:(NSDictionary *)dict
{
    return [[self alloc] initWithDictionary:dict];
}

- (instancetype)initWithDictionary:(NSDictionary *)dict
{
	self = [super init];

	// This check serves to make sure that a non-NSDictionary object
	// passed into the model class doesn't break the parsing.
	if (self && [dict isKindOfClass:[NSDictionary class]])
	{
		self.label = [self objectOrNilForKey:kSDKItemLabel fromDictionary:dict];
		self.xcodeVersion = [self objectOrNilForKey:kSDKItemXcodeVersion fromDictionary:dict];
		self.coronaVersion = [[self objectOrNilForKey:kSDKItemCoronaVersion fromDictionary:dict] doubleValue];
		self.beta = [[self objectOrNilForKey:kIosBeta fromDictionary:dict] boolValue];
		self.failMessage = [self objectOrNilForKey:kIosFailMessage fromDictionary:dict];
		self.customTemplate = [self objectOrNilForKey:kCustomTemplate fromDictionary:dict];
	}

	return self;

}

- (NSDictionary *)dictionaryRepresentation
{
	NSMutableDictionary *mutableDict = [NSMutableDictionary dictionary];
	[mutableDict setValue:self.label forKey:kSDKItemLabel];
	[mutableDict setValue:self.xcodeVersion forKey:kSDKItemXcodeVersion];
	[mutableDict setValue:[NSNumber numberWithDouble:self.coronaVersion] forKey:kSDKItemCoronaVersion];
    [mutableDict setValue:[NSNumber numberWithBool:self.beta] forKey:kIosBeta];
	[mutableDict setValue:self.failMessage forKey:kIosFailMessage];
	[mutableDict setValue:self.customTemplate forKey:kCustomTemplate];

	return [NSDictionary dictionaryWithDictionary:mutableDict];
}

- (NSString *)description 
{
    return [NSString stringWithFormat:@"%@", [self dictionaryRepresentation]];
}

#pragma mark - SDKItem Helper Method
- (id)objectOrNilForKey:(id)aKey fromDictionary:(NSDictionary *)dict
{
    id object = [dict objectForKey:aKey];
    return [object isEqual:[NSNull null]] ? nil : object;
}


#pragma mark - SDKItem NSCoding Methods

- (id)initWithCoder:(NSCoder *)aDecoder
{
	self = [super init];

	self.label = [aDecoder decodeObjectForKey:kSDKItemLabel];
	self.xcodeVersion = [aDecoder decodeObjectForKey:kSDKItemXcodeVersion];
	self.coronaVersion = [aDecoder decodeDoubleForKey:kSDKItemCoronaVersion];
	self.beta = [aDecoder decodeBoolForKey:kIosBeta];
	self.failMessage = [aDecoder decodeObjectForKey:kIosFailMessage];
	self.customTemplate = [aDecoder decodeObjectForKey:kCustomTemplate];

	return self;
}

- (void)encodeWithCoder:(NSCoder *)aCoder
{

    [aCoder encodeObject:_label forKey:kSDKItemLabel];
    [aCoder encodeObject:_xcodeVersion forKey:kSDKItemXcodeVersion];
    [aCoder encodeDouble:_coronaVersion forKey:kSDKItemCoronaVersion];
	[aCoder encodeBool:_beta forKey:kIosBeta];
	[aCoder encodeObject:_failMessage forKey:kIosFailMessage];
	[aCoder encodeObject:_customTemplate forKey:kCustomTemplate];
}

- (id)copyWithZone:(NSZone *)zone
{
    SDKItem *copy = [[SDKItem alloc] init];
    
    if (copy)
	{

        copy.label = [self.label copyWithZone:zone];
        copy.xcodeVersion = [self.xcodeVersion copyWithZone:zone];
        copy.coronaVersion = self.coronaVersion;
		copy.beta = self.beta;
		copy.failMessage = [self.failMessage copyWithZone:zone];
		copy.customTemplate = [self.customTemplate copyWithZone:zone];
    }
    
    return copy;
}


- (void)dealloc
{
    [_label release];
    [_xcodeVersion release];
	[_failMessage release];
	[_customTemplate release];
    [super dealloc];
}

@end
