//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "NativeFileCreate.h"
#import <Foundation/Foundation.h>
#include <stdio.h>


bool NativeFileCreate(const char* input_file, const char* target_dir, int file_name_sanitation_mode)
{
	NSAutoreleasePool* autorelease_pool = [[NSAutoreleasePool alloc] init];
/*
 // fileHandleForReadingFromURL isn't working for me. It doesn't return an error, but I can't get any data.
 // Static analysis shows data will always be 0. That suggests I'm using the API wrong.
 
	NSURL* input_url = [NSURL fileURLWithPath:[[NSString stringWithUTF8String:input_file] stringByStandardizingPath]];
	NSLog(@"input_url %@", input_url);
	NSError* the_error = nil;
    NSFileHandle* file_handle = [NSFileHandle fileHandleForReadingFromURL:input_url error:&the_error];
    if(nil != file_handle || nil != the_error)
    {
        NSLog(@"Error opening input file: %@", [the_error localizedDescription]);
        return false;
    }
*/


    NSFileHandle* file_handle = [NSFileHandle fileHandleForReadingAtPath:[[NSString stringWithUTF8String:input_file] stringByStandardizingPath]];

	// slurp all the data in the file. (Should be small enough for desktops to handle.)
    NSData* file_data = [file_handle readDataToEndOfFile];

	// 1 byte file is just an EOF
	if( (nil == file_data) || ([file_data length] <= 1) )
	{
		return true;
	}
/*
	NSLog(@"file_data %@", file_data);
	NSLog(@"file_data length %lu", [file_data length]);
*/
	// The format is a giant string with : separators.
    NSString* file_string = [[[NSString alloc] initWithBytes:[file_data bytes] length:[file_data length] encoding:NSUTF8StringEncoding] autorelease];

	if(nil == file_string)
	{
		return true;
	}
	
    NSArray* list_of_files = [file_string componentsSeparatedByString:@":"];
//	NSLog(@"list_of_files %@", list_of_files);

	NSString* target_dir_nsstring = [[NSString stringWithUTF8String:target_dir] stringByStandardizingPath];
	
	[[NSFileManager defaultManager] createDirectoryAtPath:target_dir_nsstring withIntermediateDirectories:YES attributes:nil error:nil];
	 
	
    [list_of_files enumerateObjectsWithOptions:NSEnumerationConcurrent usingBlock:
		^(id the_object, NSUInteger the_index, BOOL* bad_stop)
		{
			NSString* base_file_name = [the_object lastPathComponent];
			switch(file_name_sanitation_mode)
			{
				// For some reason, only for Android sound files, we make names lowercase and replace strings with underscores.
				case 1:
				{
					base_file_name = [base_file_name lowercaseString];
					base_file_name = [base_file_name stringByReplacingOccurrencesOfString:@" " withString:@""];
					break;
				}
				default:
				{
				}
			}
			NSString* touch_file = [target_dir_nsstring stringByAppendingPathComponent:base_file_name];
//			NSLog(@"%@", touch_file);
			BOOL did_succeed = [[NSFileManager defaultManager] createFileAtPath:touch_file contents:nil attributes:nil];
			
			if(NO == did_succeed)
			{
				NSLog(@"Error in creating file: %@", touch_file);
				*bad_stop = YES;
				return;
			}
		}

     ];
	[autorelease_pool drain];
	return true;
}
