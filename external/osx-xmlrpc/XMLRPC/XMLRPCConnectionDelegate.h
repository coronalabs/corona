#import <Foundation/Foundation.h>

@class XMLRPCConnection, XMLRPCRequest, XMLRPCResponse;

@protocol XMLRPCConnectionDelegate<NSObject>

@required
- (void)request: (XMLRPCRequest *)request didReceiveResponse: (XMLRPCResponse *)response;

@optional
- (void)request: (XMLRPCRequest *)request didSendBodyData: (float)percent;

@required
- (void)request: (XMLRPCRequest *)request didFailWithError: (NSError *)error;

@optional
- (void)request: (XMLRPCRequest *)request didReceiveData: (float)percent;

#pragma mark -

@required
- (BOOL)request: (XMLRPCRequest *)request canAuthenticateAgainstProtectionSpace: (NSURLProtectionSpace *)protectionSpace;

@required
- (void)request: (XMLRPCRequest *)request didReceiveAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge;

@required
- (void)request: (XMLRPCRequest *)request didCancelAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge;

@end
