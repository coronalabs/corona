#import "XMLRPCRequest.h"
#import "XMLRPCEncoder.h"
#import "XMLRPCDefaultEncoder.h"

static const NSTimeInterval DEFAULT_TIMEOUT = 240;

@implementation XMLRPCRequest

- (id)initWithURL: (NSURL *)URL withEncoder: (id<XMLRPCEncoder>)encoder {
    self = [super init];
    if (self) {
        if (URL) {
            myRequest = [[NSMutableURLRequest alloc] initWithURL: URL];
        } else {
            myRequest = [[NSMutableURLRequest alloc] init];
        }

		// The default NSURLRequest timeout is 60 seconds but sometimes server responses
		// can take longer than that so increase it
		[myRequest setTimeoutInterval:DEFAULT_TIMEOUT];

        myXMLEncoder = encoder;
#if ! __has_feature(objc_arc)
        [myXMLEncoder retain];
#endif

        myTimeout = DEFAULT_TIMEOUT;
    }
    
    return self;
}

- (id)initWithURL: (NSURL *)URL {
#if ! __has_feature(objc_arc)
    return [self initWithURL:URL withEncoder:[[[XMLRPCDefaultEncoder alloc] init] autorelease]];
#else
    return [self initWithURL:URL withEncoder:[[XMLRPCDefaultEncoder alloc] init]];
#endif
}

#pragma mark -

- (void)setURL: (NSURL *)URL {
    [myRequest setURL: URL];
}

- (NSURL *)URL {
    return [myRequest URL];
}

#pragma mark -

- (void)setUserAgent: (NSString *)userAgent {
    if (![self userAgent]) {
        [myRequest addValue: userAgent forHTTPHeaderField: @"User-Agent"];
    } else {
        [myRequest setValue: userAgent forHTTPHeaderField: @"User-Agent"];
    }
}

- (NSString *)userAgent {
    return [myRequest valueForHTTPHeaderField: @"User-Agent"];
}

#pragma mark -

- (void)setEncoder:(id<XMLRPCEncoder>)encoder {
    NSString *method = [myXMLEncoder method];
    NSArray *parameters = [myXMLEncoder parameters];
#if ! __has_feature(objc_arc)
    [myXMLEncoder release];
    
    myXMLEncoder = [encoder retain];
#else
    myXMLEncoder = encoder;
#endif
    
    [myXMLEncoder setMethod: method withParameters: parameters];
}

- (void)setMethod: (NSString *)method {
    [myXMLEncoder setMethod: method withParameters: nil];
}

- (void)setMethod: (NSString *)method withParameter: (id)parameter {
    NSArray *parameters = nil;
    
    if (parameter) {
        parameters = [NSArray arrayWithObject: parameter];
    }
    
    [myXMLEncoder setMethod: method withParameters: parameters];
}

- (void)setMethod: (NSString *)method withParameters: (NSArray *)parameters {
    [myXMLEncoder setMethod: method withParameters: parameters];
}

- (void)setTimeoutInterval: (NSTimeInterval)timeout
{
    myTimeout = timeout;
	[myRequest setTimeoutInterval:myTimeout];
}

#pragma mark -

- (NSString *)method {
    return [myXMLEncoder method];
}

- (NSArray *)parameters {
    return [myXMLEncoder parameters];
}

- (NSTimeInterval)timeout
{
    return myTimeout;
}

#pragma mark -

- (NSString *)body {
    return [myXMLEncoder encode];
}

#pragma mark -

- (NSURLRequest *)request {
    NSData *content = [[self body] dataUsingEncoding: NSUTF8StringEncoding];
    NSNumber *contentLength = [NSNumber numberWithUnsignedInteger:[content length]];
    
    if (!myRequest) {
        return nil;
    }
    
    [myRequest setHTTPMethod: @"POST"];
    
    if (![myRequest valueForHTTPHeaderField: @"Content-Type"]) {
        [myRequest addValue: @"text/xml" forHTTPHeaderField: @"Content-Type"];
    } else {
        [myRequest setValue: @"text/xml" forHTTPHeaderField: @"Content-Type"];
    }
    
    if (![myRequest valueForHTTPHeaderField: @"Content-Length"]) {
        [myRequest addValue: [contentLength stringValue] forHTTPHeaderField: @"Content-Length"];
    } else {
        [myRequest setValue: [contentLength stringValue] forHTTPHeaderField: @"Content-Length"];
    }
    
    if (![myRequest valueForHTTPHeaderField: @"Accept"]) {
        [myRequest addValue: @"text/xml" forHTTPHeaderField: @"Accept"];
    } else {
        [myRequest setValue: @"text/xml" forHTTPHeaderField: @"Accept"];
    }
    
    if (![self userAgent]) {
      NSString *userAgent = [[NSUserDefaults standardUserDefaults] objectForKey:@"UserAgent"];
        
      if (userAgent) {
        [self setUserAgent: userAgent];
      }
    }
    
    [myRequest setHTTPBody: content];
    
    return (NSURLRequest *)myRequest;
}

#pragma mark -

- (void)setValue: (NSString *)value forHTTPHeaderField: (NSString *)header {
    [myRequest setValue: value forHTTPHeaderField: header];
}
#pragma mark -

- (id) extra {
    return extra;
}

- (void) setExtra:(id) extraObject {
#if ! __has_feature(objc_arc)
    [extra release];
    extra = [extraObject retain];
#else
    extra = extraObject;
#endif
}

#pragma mark -

- (void)dealloc {
#if ! __has_feature(objc_arc)
    [myRequest release];
    [myXMLEncoder release];
    
    [super dealloc];
#endif
}

@end
