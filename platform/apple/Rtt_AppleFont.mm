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
#include "Rtt_AppleFont.h"


#ifdef Rtt_MAC_ENV
	#import <AppKit/NSFont.h>
	#import <AppKit/NSFontManager.h>
#elif defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
	#import <UIKit/UIFont.h>
	#import <CoreText/CoreText.h>
#else
	#error Unsupported platform
#endif


namespace Rtt
{

NSMutableDictionary<NSString*, NSString*>* AppleFontHelper::sFontCache = nil;
NSMutableSet<NSURL*>* AppleFontHelper::sActivatedFonts = nil;
	
AppleFont* AppleFontHelper::AllocNativeFont( const char *fontName, Rtt_Real size, NSString *directory )
{
	AppleFont *fFont = [AppleFont fontWithName:[NSString stringWithUTF8String:fontName] size:size];
	
	NSString* dirWithFont = nil;
	if ( ! fFont )
	{
		if ( ! directory )
		{
			// Default font location to main app bundle location
			directory = [[NSBundle mainBundle] resourcePath];
		}
		dirWithFont = [NSString pathWithComponents:@[directory, [NSString stringWithUTF8String:fontName]]];
		
		NSString *postScriptName = [sFontCache objectForKey:dirWithFont];
		if ( postScriptName )
		{
			fFont = [AppleFont fontWithName:postScriptName size:size];
		}
	}
	
	if( ! fFont )
	{
		NSString *filePath = nil;
		NSString *postScriptName = nil;
		
		// iterate though possible files and attempt to register and load fonts from them. First past the post wins.
		for (NSString* extension in @[ @"", @".ttf", @".otf", @".ttc" ])
		{
			filePath = [dirWithFont stringByAppendingString:extension];
			BOOL isDirectory = NO;
			if ( ! [[NSFileManager defaultManager] fileExistsAtPath:filePath isDirectory:&isDirectory] )
			{
				filePath = nil;
			}
			if ( isDirectory )
			{
				filePath = nil;
			}
			postScriptName = nil;
			// file exists, attempt to obtain font face from it and load it if necessary
			if ( filePath )
			{
				NSData *inData = [NSData dataWithContentsOfFile:filePath];
				if (inData)
				{
					CGDataProviderRef provider = CGDataProviderCreateWithCFData((CFDataRef)inData);
					if ( provider )
					{
						CGFontRef cgFont = CGFontCreateWithDataProvider(provider);
						if ( cgFont )
						{
							postScriptName = (NSString *)CGFontCopyPostScriptName(cgFont);
							CFRelease(cgFont);
						}
						CFRelease(provider);
					}
				}
				
				// file has a face name. It is a font.
				if ( postScriptName )
				{
					// if font was already registered, attempt to access it by face name
					fFont = [AppleFont fontWithName:postScriptName size:size];
					
					// font was not registered yet. Attempt to register it and try to get reference again.
					if ( ! fFont )
					{
						CFErrorRef error;
						NSURL * fontFileUrl = [NSURL fileURLWithPath:filePath];
						if ( CTFontManagerRegisterFontsForURL((__bridge CFURLRef)fontFileUrl, kCTFontManagerScopeProcess, &error) )
						{
							if( ! sActivatedFonts )
							{
								sActivatedFonts = [[NSMutableSet alloc] init];
							}
							[sActivatedFonts addObject:fontFileUrl];
						}
						else
						{
							CFStringRef errorDescription = CFErrorCopyDescription(error);
							Rtt_LogException("Failed to register font: %s (%s)", fontName, [(NSString*)errorDescription UTF8String]);
							CFRelease(errorDescription);
						}
						fFont = [AppleFont fontWithName:postScriptName size:size];
					}
				}
			}
			
			// Stop looking for a font file if we already found one
			if ( fFont )
			{
				if ( ! sFontCache )
				{
					sFontCache = [[NSMutableDictionary alloc] init];
				}
				[sFontCache setObject:postScriptName forKey:dirWithFont];
			}
			[postScriptName release];
			if ( fFont )
			{
				break;
			}
		}
	}
	
	if ( ! fFont )
	{
		Rtt_LogException( "WARNING: The font (%s) could not be found. Using system font instead.\n", (fontName ? fontName : "null") );
		fFont = [AppleFont systemFontOfSize:size];
	}
	
	[fFont retain];
	return fFont;
}

void AppleFontHelper::ClearCacheAndLoadedFonts()
{
	CFErrorRef error;
	for (NSURL *fontFileURL in sActivatedFonts)
	{
		if (CTFontManagerUnregisterFontsForURL((__bridge CFURLRef)fontFileURL,
											   kCTFontManagerScopeProcess,
											   &error))
		{
			Rtt_TRACE(("Deactivated fonts in '%s'", [[fontFileURL path] UTF8String]));
		}
		else
		{
			Rtt_TRACE_SIM(("Failed to deactivate fonts in file '%s'", [[fontFileURL path] UTF8String]));
		}
	}
	
	[sActivatedFonts release];
	[sFontCache release];
	
	sActivatedFonts = nil;
	sFontCache = nil;
}
	
}
