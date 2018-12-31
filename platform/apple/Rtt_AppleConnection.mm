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

#include "Core/Rtt_Build.h"

#import <XMLRPC/XMLRPC.h>

#import "NSString+Extensions.h"

#include "Rtt_AppleConnection.h"

#include "Rtt_AppleDictionaryWrapper.h"
#include "Rtt_MPlatformServices.h"

#if ! defined(Rtt_NO_GUI)
#import "AppDelegate.h"
#endif

#ifdef Rtt_DEBUG
#define NSDEBUG(...) // NSLog(__VA_ARGS__)
#else
#define NSDEBUG(...) // NSLog(__VA_ARGS__)
#endif

//
// Connection delegate for XMLRPCConnection
//
@interface XMLRPCConnectionDelegate : NSObject <XMLRPCConnectionDelegate>
{
	XMLRPCResponse *xmlrpcResponse;
	BOOL xmlrpcComplete;
	NSString *errorMesg;
}
@end

@implementation XMLRPCConnectionDelegate

- (id) init
{
	self = [super init];

	if ( self )
	{
		xmlrpcResponse = nil;
		xmlrpcComplete = NO;
		errorMesg = nil;
	}

	return self;
}

- (void) dealloc
{
	[xmlrpcResponse release];
	[errorMesg release];

	[super dealloc];
}

- (BOOL) complete
{
	return xmlrpcComplete;
}

- (XMLRPCResponse *) response
{
	return xmlrpcResponse;
}

- (NSString *) errorMesg
{
	return errorMesg;
}

- (void)request: (XMLRPCRequest *)request didReceiveResponse: (XMLRPCResponse *)response
{
	NSDEBUG(@"%s", __func__);
	xmlrpcResponse = [response retain];
	xmlrpcComplete = YES;
}

- (void)request: (XMLRPCRequest *)request didSendBodyData: (float)percent
{
	NSDEBUG(@"%s: %g%%", __func__, (percent * 100));

}

- (void)request: (XMLRPCRequest *)request didFailWithError: (NSError *)error
{
	NSDEBUG(@"%s", __func__);
	xmlrpcComplete = YES;

	errorMesg = [[error localizedDescription] copy];

	// Log the error to the Simulator Console
	Rtt_LogException("ERROR: XMLRPC: %s\n", [[error localizedDescription] UTF8String]);
}

- (void)request: (XMLRPCRequest *)request didReceiveData: (float)percent
{
	// NSDEBUG(@"%s: %g%%", __func__, (percent * 100));

#if ! defined(Rtt_NO_GUI)
	AppDelegate *appDelegate = (AppDelegate*)[NSApp delegate];

	// Don't move progress from 0 to 100% in one go (avoids progress bar for small protocol messages)
	if (percent < 0.99 || appDelegate.buildDownloadProgess != 0)
	{
		appDelegate.buildDownloadProgess = percent;
	}
#endif
}

- (BOOL)request: (XMLRPCRequest *)request canAuthenticateAgainstProtectionSpace: (NSURLProtectionSpace *)protectionSpace
{
	NSDEBUG(@"%s", __func__);
	return NO;
}

- (void)request: (XMLRPCRequest *)request didReceiveAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge
{
	NSDEBUG(@"%s", __func__);

}

- (void)request: (XMLRPCRequest *)request didCancelAuthenticationChallenge: (NSURLAuthenticationChallenge *)challenge
{
	NSDEBUG(@"%s", __func__);

}


@end

//
// Connection delegate for build downloads
//
@interface Rtt_DownloadConnectionDelegate : NSObject <NSURLConnectionDelegate>
{
	NSInteger totalBytesExpectedToReceive;
	NSInteger totalBytesReceived;
	NSURL *fileURL;
	NSFileHandle *outputFile;
	NSString *errorMesg;
	NSURLResponse *downloadResponse;
	BOOL downloadComplete;
}
@end

@implementation Rtt_DownloadConnectionDelegate

- (id) initWithFileURL:(NSURL *)aFileURL
{
	self = [super init];

	if ( self )
	{
		fileURL = aFileURL;
		totalBytesExpectedToReceive = 0;
		totalBytesReceived = 0;
		errorMesg = nil;
		downloadResponse = nil;
		downloadComplete = NO;
	}

	return self;
}

- (void) dealloc
{
	[errorMesg release];
	[downloadResponse release];

	[super dealloc];
}

- (BOOL) complete
{
	return downloadComplete;
}

- (NSURLResponse *) response
{
	return downloadResponse;
}

- (NSString *) errorMesg
{
	return errorMesg;
}

- (NSInteger) totalBytesReceived
{
	return totalBytesReceived;
}

- (NSCachedURLResponse *)connection:(NSURLConnection *)connection willCacheResponse:(NSCachedURLResponse *)cachedResponse
{
	NSDEBUG(@"%s", __func__);

	return nil;
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
	NSDEBUG(@"%s", __func__);
	downloadResponse = [response retain];

	if ([response respondsToSelector: @selector(expectedContentLength)])
	{
		totalBytesExpectedToReceive = [response expectedContentLength];
	}

	if ([response respondsToSelector: @selector(statusCode)])
	{
		NSInteger statusCode = [(NSHTTPURLResponse *)response statusCode];

		if (statusCode >= 400)
		{
			NSError *error = [NSError errorWithDomain: @"HTTP" code: statusCode userInfo: nil];

			errorMesg = [[error localizedDescription] retain];

			return;
		}
	}

	// Open the output file
	NSError *error = nil;
	if ([[NSFileManager defaultManager] createFileAtPath:[fileURL path] contents:nil attributes:nil])
	{
		outputFile = [[NSFileHandle fileHandleForWritingToURL:fileURL error:&error] retain];
	}
	else
	{
		Rtt_LogException("ERROR: download: failed to open '%s' for writing", [[fileURL path] UTF8String]);
	}
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
#if ! defined(Rtt_NO_GUI)
	float percent = ((float)totalBytesReceived/totalBytesExpectedToReceive);
#endif
	// NSDEBUG(@"%s: %g%%", __func__, (percent * 100));

	totalBytesReceived += [data length];

	[outputFile writeData:data];

#if ! defined(Rtt_NO_GUI)
	AppDelegate *appDelegate = (AppDelegate*)[NSApp delegate];

	// Don't move progress from 0 to 100% in one go (avoids progress bar for small protocol messages)
	if (percent < 0.99 || appDelegate.buildDownloadProgess != 0)
	{
		appDelegate.buildDownloadProgess = percent;
	}
#endif
}

- (NSURLRequest *)connection:(NSURLConnection *)connection willSendRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)redirectResponse
{
	NSDEBUG(@"%s", __func__);
	return request;
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	NSDEBUG(@"%s", __func__);

	// It seems we occasionally get a handful of bytes more than the expected Content-Length from S3 so
	// here we only error if the bytes received is less than the bytes expected (rather than not equal to)
	if (totalBytesExpectedToReceive != -1 && totalBytesReceived < totalBytesExpectedToReceive)
	{
		errorMesg = [[NSString stringWithFormat:@"expected %ld bytes but received %ld", totalBytesExpectedToReceive, totalBytesReceived] retain];
		Rtt_LogException("ERROR: download: %s\n", [errorMesg UTF8String]);
	}

	[outputFile closeFile];

	downloadComplete = YES;
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	NSDEBUG(@"%s", __func__);

	errorMesg = [[error localizedDescription] copy];

	// Log the error to the Simulator Console
	Rtt_LogException("ERROR: download: %s\n", [[error localizedDescription] UTF8String]);

	downloadComplete = YES;
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

bool
AppleConnection::Download(const char *urlStr, const char *filename)
{
	NSURL *downloadURL = [NSURL URLWithString:[NSString stringWithExternalString:urlStr]];
	NSURL *fileURL = [NSURL fileURLWithPath:[NSString stringWithExternalString:filename]];
	NSURLRequest *downloadRequest = [[[NSURLRequest alloc] initWithURL:downloadURL] autorelease];
	time_t startTime = time(NULL);

	// The server doesn't like certain cookies with its XMLRPC requests so zap 'em
	//[downloadRequest setValue:@"none" forHTTPHeaderField:@"Cookie"];

	//[downloadRequest setTimeoutInterval:0];  // disable timeout

	if (fDebugWebServices > 1)
	{
		Rtt_LogException("WebServices: download request: %s", [[downloadURL absoluteString] UTF8String]);
		Rtt_LogException("WebServices:          to file: %s", [[fileURL absoluteString] UTF8String]);
	}
	else if (fDebugWebServices > 0)
	{
		Rtt_LogException("WebServices: downloading build");
	}

	startTime = time(NULL);  // might be used in an error message below so we always need this

	NSURLResponse *downloadResponse = nil;
	Rtt_DownloadConnectionDelegate *downloadDelegate = [[[Rtt_DownloadConnectionDelegate alloc] initWithFileURL:fileURL] autorelease];
	NSURLConnection *downloadConnection = nil;

	downloadConnection = [[[NSURLConnection alloc] initWithRequest:downloadRequest delegate:downloadDelegate startImmediately:YES] autorelease];

	// Wait for the request to complete while pumping the run loop
	while ( ! [downloadDelegate complete])
	{
		@autoreleasepool {
#if ! defined(Rtt_NO_GUI)
			AppDelegate *appDelegate = (AppDelegate*)[NSApp delegate];

			if ([appDelegate stopRequested])
			{
				[downloadConnection cancel];

				break;
			}
	#endif

			[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.4]];
		}
	}

	downloadResponse = [downloadDelegate response];

	if (downloadResponse == nil)
	{
		// The error details should already have been reported by Rtt_DownloadConnectionDelegate
		Rtt_LogException("WebServices: download request failed (no response)");

#if ! defined(Rtt_NO_GUI)
		// When running in the GUI, the xmlDelegate runs on the main thread so we need to access it there when an error occurs
		dispatch_sync(dispatch_get_main_queue(), ^{
#endif

			fError = [[downloadDelegate errorMesg] retain];

#if ! defined(Rtt_NO_GUI)
		});
#endif

		return NO;
	}
	else if ([downloadDelegate errorMesg] != nil)
	{
		Rtt_LogException("WebServices: download request failed: %s", [[downloadDelegate errorMesg] UTF8String]);

		fError = [[downloadDelegate errorMesg] retain];

		return NO;
	}

	if (fDebugWebServices > 0)
	{
		time_t totalTime = time(NULL) - startTime;
		float MBps = ((float)[downloadDelegate totalBytesReceived] / totalTime) / (1024*1024);

		Rtt_LogException("WebServices: Download of %ld bytes complete in %ld second%s (%.2fMB/s)",
						 [downloadDelegate totalBytesReceived], totalTime, (totalTime == 1 ? "" : "s"), MBps);
	}

	return YES;
}


AppleConnection::AppleConnection( const MPlatformServices& platform, const char* url )
:	Super( platform ),
	fUrl( nil ),
	fError( nil )
{
//"http://web-a1.ath.cx/ansca-drupal-head/services/xmlrpc"

	NSString* str = [[NSString alloc] initWithUTF8String:url];
	fUrl = [[NSURL alloc] initWithString:str];
	[str release];

    String prefVal;
    fPlatform.GetPreference("debugWebServices", &prefVal);
    if (! prefVal.IsEmpty())
    {
        fDebugWebServices = (int) strtol(prefVal.GetString(), (char **)NULL, 10);
    }
    else
    {
        fDebugWebServices = 0;
    }
}

AppleConnection::~AppleConnection()
{
	[fError release];
	[fUrl release];
}

static NSArray *
CreateParamsArray( const KeyValuePair* pairs, int numPairs )
{
	Rtt_ASSERT( numPairs > 0 );

	NSMutableArray* values = [[NSMutableArray alloc] initWithCapacity:numPairs];

	for ( int i = 0; i < numPairs; i++ )
	{
		const KeyValuePair& item = pairs[i];
		const char *itemValue = (const char*) item.value;

		// These checks prevent us calling [NSString initWithUTF8String:] with NULL which can result
		// in an uncaught exception.
		if (item.key == NULL)
		{
			Rtt_TRACE(("CreateParamsArray: item.key unexpectedly NULL\n"));

			continue;
		}

		if (item.type == kStringValueType && item.value == NULL)
		{
			Rtt_TRACE(("CreateParamsArray: item.value unexpectedly NULL for item.key '%s'\n", item.key));

			itemValue = "";
		}

		// Value
		if ( item.type <= 0 )
		{
			NSObject* v = nil;
			switch( item.type )
			{
				case kStringValueType:
					v = [[NSString alloc] initWithUTF8String:itemValue]; Rtt_ASSERT( v );
					break;
				case kIntegerValueType:
					v = [[NSNumber alloc] initWithInteger:(NSInteger)item.value]; Rtt_ASSERT( v );
					break;
				default:
					Rtt_ASSERT_NOT_REACHED();
					v = [[NSString alloc] initWithString:@""];
					break;
			}

			[values addObject:v];
			[v release];
		}
		else
		{
			// Recurse ...
			NSArray *v = CreateParamsArray( (const KeyValuePair*)item.value, item.type );
			[values addObject:v];
			[v release];
		}
	}

	return values;
}

PlatformDictionaryWrapper*
AppleConnection::Call( const char* m, const KeyValuePair* pairs, int numPairs )
{
	XMLRPCRequest *xmlRequest = [[[XMLRPCRequest alloc] initWithURL:fUrl] autorelease];
    time_t startTime = 0;
	PlatformDictionaryWrapper* result = NULL;
	NSString* method = [[[NSString alloc] initWithUTF8String:m] autorelease];
    NSArray* params = nil;
	size_t maxDebugLength = ((fDebugWebServices > 3) ? 10000 :  0);

	if ( xmlRequest != nil )
	{
		if ( numPairs > 0 )
		{
			// Construct an ordered array of the parameters (XMLRPC only uses
			// the "value" half of the "key/value" pair)
			params = CreateParamsArray(pairs, numPairs);
		}

        if (fDebugWebServices > 0)
        {
			Rtt_LogException("WebServices: Call: %s %s", m, ((params == nil && fDebugWebServices > 1) ? "(no params)" : ""));
        }

        startTime = time(NULL);  // might be used in an error message below so we always need this

		// The server doesn't like certain cookies with its XMLRPC requests so zap 'em
		[xmlRequest setValue:@"none" forHTTPHeaderField:@"Cookie"];

		[xmlRequest setMethod:method withParameters:params];
		[xmlRequest setTimeoutInterval:0];  // disable timeout

		if (fDebugWebServices > 2)
		{
			if (maxDebugLength > 0 && [[xmlRequest body] length] < maxDebugLength)
			{
				Rtt_LogException("WebServices: XMLRPC request body: %s", [[xmlRequest body] UTF8String]);
			}
			else
			{
				Rtt_LogException("WebServices: XMLRPC request: %ld character body elided", [[xmlRequest body] length]);
			}
		}

		// XMLRPCResponse *xmlResponse = [XMLRPCConnection sendSynchronousXMLRPCRequest:xmlRequest error:&error];

		XMLRPCResponse *xmlResponse = nil;
		XMLRPCConnectionDelegate *xmlDelegate = [[[XMLRPCConnectionDelegate alloc] init] autorelease];
		XMLRPCConnectionManager *xmlConnectionManager = [XMLRPCConnectionManager sharedManager];
		XMLRPCConnection *xmlRPCConnection = nil;

		xmlRPCConnection = [[[XMLRPCConnection alloc] initWithXMLRPCRequest:xmlRequest delegate:xmlDelegate manager:xmlConnectionManager] autorelease];

		if (xmlRPCConnection == nil)
		{
			Rtt_LogException("WebServices: XMLRPC connection failed");

			return nil;
		}

		// Wait for the request to complete while pumping the run loop
		while ( ! [xmlDelegate complete])
		{
			@autoreleasepool {

#if ! defined(Rtt_NO_GUI)
				AppDelegate *appDelegate = (AppDelegate*)[NSApp delegate];

				if ([appDelegate stopRequested])
				{
					[xmlRPCConnection cancel];

					break;
				}
#endif

				[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
			}
		}

		xmlResponse = [xmlDelegate response];

		if (xmlResponse == nil)
		{
			// The error details should already have been reported by XMLRPCConnectionDelegate
			Rtt_LogException("WebServices: XMLRPC request failed");

#if ! defined(Rtt_NO_GUI)
			// When running in the GUI, the xmlDelegate runs on the main thread so we need to access it there when an error occurs
			dispatch_sync(dispatch_get_main_queue(), ^{
#endif

				fError = [[xmlDelegate errorMesg] retain];

#if ! defined(Rtt_NO_GUI)
			});
#endif

			return nil;
		}

		if (fDebugWebServices > 2)
		{
			if (maxDebugLength > 0 && [[xmlResponse body] length] < maxDebugLength)
			{
				Rtt_LogException("WebServices: XMLRPC response body: %s", [[xmlResponse body] UTF8String]);
			}
			else
			{
				Rtt_LogException("WebServices: XMLRPC response: %ld character body elided", [[xmlResponse body] length]);
			}
		}

		NSString *faultString = [xmlResponse faultString];
		if (faultString != nil)
		{
			[fError release];
			fError = [faultString retain];

			Rtt_LogException("WebServices: Call %s failed in %ld seconds: %s", m, (time(NULL) - startTime), [fError UTF8String]);
			Rtt_LogException("   response: %s", [[xmlResponse description] UTF8String]);
		}

		NSObject *responseObject = [xmlResponse object];

		if (responseObject != nil)
		{
			if (! [responseObject isKindOfClass:[NSDictionary class]])
			{
                // Make a placeholder dictionary for a (typically) string result
				responseObject = [NSDictionary dictionaryWithObject:responseObject forKey:@"result"];
			}

			NSDictionary* dict = (NSDictionary*)responseObject;

            if (fDebugWebServices > 0)
            {
                if ([[dict objectForKey:@"result"] isKindOfClass:[NSString class]])
                {
                    NSString *s = (NSString *)[dict objectForKey:@"result"];

                    if ([s length] > maxDebugLength)
                    {
                        Rtt_LogException("WebServices: Response received in %ld seconds: <%ld bytes of data elided>", (time(NULL) - startTime), [s length]);
                    }
                    else
                    {
                        Rtt_LogException("WebServices: Response received in %ld seconds: %s", (time(NULL) - startTime), [s UTF8String]);
                    }
                }
                else if (fDebugWebServices > 2)
                {
                    Rtt_LogException("WebServices: Response received in %ld seconds: %s", (time(NULL) - startTime), [[dict description] UTF8String]);
                }
                else
                {
                    Rtt_LogException("WebServices: Response received in %ld seconds", (time(NULL) - startTime));
                }
            }

			result = Rtt_NEW( & fPlatform.GetAllocator(), AppleDictionaryWrapper( dict ) );
		}
	}
	else
	{
		Rtt_LogException("WebServices: failed to init XMLRPCRequest failed for %s", [[method description] UTF8String]);
	}

	return result;
}

const char*
AppleConnection::Error() const
{
	return [fError UTF8String];
}

const char*
AppleConnection::Url() const
{
	return [[fUrl absoluteString] UTF8String];
}

void
AppleConnection::CloseConnection()
{
	// No-op on this platform currently
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

