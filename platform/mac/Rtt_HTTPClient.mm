//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_HTTPClient.h"
#include <Foundation/Foundation.h>
#include "NSString+Extensions.h"

namespace Rtt {
  
bool HTTPClient::HttpDownload( const char* url, Rtt::String &resultData, String& errorMesg, const std::map<std::string, std::string>& headers )
{
	resultData.Set("");
	errorMesg.Set("");
	dispatch_semaphore_t    sem;

	__block bool result = false;

	sem = dispatch_semaphore_create(0);

	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithExternalString:url]]];

	for ( const auto &pair : headers )
	{
		[request setValue:[NSString stringWithExternalString:pair.second.c_str()] forHTTPHeaderField:[NSString stringWithExternalString:pair.first.c_str()]];
	}

	[[[NSURLSession sharedSession] dataTaskWithRequest:request
									 completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {

											 bool successful = true;

											 if ([response isKindOfClass:[NSHTTPURLResponse class]] &&
												 [(NSHTTPURLResponse *) response statusCode] != 200)
											 {
												 errorMesg.Set("HttpDownload: download failed with status: ");
												 errorMesg.Append([[NSString stringWithFormat:@"%ld", [(NSHTTPURLResponse *) response statusCode]] UTF8String]);

												 successful = false;
											 }
											 if (error != nil)
											 {
												 NSString *errorStr = [error localizedDescription];

												 errorMesg.Set([errorStr UTF8String]);

												 successful = false;
											 }

											 if (successful)
											 {
												 if (data)
												 {
													 NSString* str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
													 if(str)
													 {
														 resultData.Set([str UTF8String]);
														 [str release];
														 result = true;
													 }
													 else
													 {
														 errorMesg.Set("HttpDownload: unable to decode data");
														 result = false;
													 }
												 }
												 else
												 {
													 errorMesg.Set("HttpDownload: could not open downloaded data");
												 }
											 }

											 dispatch_semaphore_signal(sem);
											 
										 }] resume];
	
	dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);

	return result;
}

bool HTTPClient::HttpDownloadFile( const char* url, const char *filename, String& errorMesg, const std::map<std::string, std::string>& headers )
{
	dispatch_semaphore_t    sem;
	__block bool result = false;

	errorMesg.Set("");

	sem = dispatch_semaphore_create(0);

	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithExternalString:url]]];

	for (const auto &pair : headers)
	{
		[request setValue:[NSString stringWithExternalString:pair.second.c_str()] forHTTPHeaderField:[NSString stringWithExternalString:pair.first.c_str()]];
	}

	[[[NSURLSession sharedSession] downloadTaskWithRequest:request
									 completionHandler:^(NSURL *location, NSURLResponse *response, NSError *error) {

										 bool successful = true;

										 if ([response isKindOfClass:[NSHTTPURLResponse class]] &&
											  [(NSHTTPURLResponse *) response statusCode] != 200)
										 {
											 errorMesg.Set("HttpDownloadFile: download failed with status: ");
											 errorMesg.Append([[NSString stringWithFormat:@"%ld", [(NSHTTPURLResponse *) response statusCode]] UTF8String]);

											 successful = false;
										 }
										 if (error != nil)
										 {
											 NSString *errorStr = [[[error userInfo] objectForKey:NSUnderlyingErrorKey] localizedDescription];

											 errorMesg.Set([errorStr UTF8String]);

											 successful = false;
										 }

										 if (successful)
										 {
											 NSURL *dest = [NSURL fileURLWithPath:[NSString stringWithExternalString:filename]];
											 NSError *err = nil;
											 if ([[NSFileManager defaultManager] replaceItemAtURL:dest
																					withItemAtURL:location
																				   backupItemName:nil
																						  options:0
																				 resultingItemURL:nil
																							error:&err])
											 {
												 result = true;
											 }
											 else
											 {
												 errorMesg.Set("HttpDownloadFile: could not copy download to '");
												 errorMesg.Append(filename);
												 errorMesg.Append("'");
											 }
										 }

										 dispatch_semaphore_signal(sem);

									 }] resume];

	dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);

	return result;
}

}


