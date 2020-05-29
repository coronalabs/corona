//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacFont.h"

#import <AppKit/NSFont.h>
#import <AppKit/NSFontManager.h>

#import "Rtt_AppleFont.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacFont::MacFont( const char *fontName, Rtt_Real size, NSString *directory )
:	Super(),
	fFont( AppleFontHelper::AllocNativeFont(fontName, size, directory) )
{
}

MacFont::MacFont( PlatformFont::SystemFont fontType, Rtt_Real size )
:	Super(),
	fFont( nil )
{
	switch ( fontType )
	{
		case PlatformFont::kSystemFontBold:
			fFont = [NSFont boldSystemFontOfSize:size];
			break;
		default:	// case PlatformFont::kSystemFont:
			fFont = [NSFont labelFontOfSize:size];
			break;		
	}

	[fFont retain];
}

MacFont::MacFont( NSFont *font )
:	Super(),
	fFont( [font retain] )
{
}

MacFont::MacFont( const MacFont &font )
:	Super(),
	fFont( [font.fFont retain] )
{
}

MacFont::~MacFont()
{
	[fFont release];
}

PlatformFont*
MacFont::CloneUsing( Rtt_Allocator *allocator ) const
{
	if ( !allocator )
	{
		return NULL;
	}
	return Rtt_NEW( allocator, MacFont( *this ) );
}

const char*
MacFont::Name() const
{
	return [[fFont fontName] UTF8String];
}

void
MacFont::SetSize( Real newValue )
{
	if ( Size() != newValue )
	{
		NSFont *oldFont = fFont;
		fFont = [[[NSFontManager sharedFontManager] convertFont:oldFont toSize:newValue] retain];
		[oldFont release];
	}
}

Real
MacFont::Size() const
{
	return [fFont pointSize];
}

void*
MacFont::NativeObject() const
{
	return fFont;
}

//
// Enumerate the files in a project (aka directory) and activate any fonts that are found
//
void
MacFont::ActivateFontsForApplication(NSString* appDirStr, bool activate /* = true */)
{
    NSString* filename = nil;
    NSDirectoryEnumerator* enumerator =
        [[NSFileManager defaultManager] enumeratorAtPath:appDirStr];
	int count = 0;
	NSDate *startTime = [NSDate date];

	while (true)
	{
		// The [enumerator nextObject] call needs to be wrapped in an autorelease pool or
		// the app quickly exceeds available memory
		@autoreleasepool {
			filename = [enumerator nextObject];

			if (filename == nil)
			{
				break;
			}

			// check if it's a directory
			BOOL isDirectory = NO;
			[[NSFileManager defaultManager]
			 fileExistsAtPath:[NSString stringWithFormat:@"%@/%@", appDirStr, filename]
			 isDirectory:&isDirectory];

			// If the candidate is not a directory and ends with one of the font file formats then process it
			// (for other reasons, our documentation says the font file extensions must be lower case so we
			// just go with that as well)
			if (!isDirectory && [@[ @"ttf", @"otf", @"ttc" ] containsObject:[filename pathExtension]])
			{
				// This catches the case where we start scanning the whole disk for fonts because
				// the main.lua is in a stupid place (like the user's home directory)
				if (++count > Rtt_MAX_FONTS_IN_APPLICATION)
				{
					Rtt_TRACE_SIM(("Too many fonts in this application (Simulator limit is %d)", Rtt_MAX_FONTS_IN_APPLICATION));

					break;
				}

				NSURL* fontFileURL = [NSURL fileURLWithPathComponents:@[ appDirStr, filename ]];
				CFErrorRef error = NULL;

				if (activate)
				{
					if (CTFontManagerRegisterFontsForURL((__bridge CFURLRef)fontFileURL,
														 kCTFontManagerScopeProcess,
														 &error))
					{
						NSData *fontData = [NSData dataWithContentsOfURL:fontFileURL];
						NSString *postScriptName = nil;

						if (fontData != nil)
						{
							// create font from loaded data and get it's postscript name
							CGDataProviderRef fontDataProvider = CGDataProviderCreateWithCFData((CFDataRef)fontData);
							CGFontRef loadedFont = CGFontCreateWithDataProvider(fontDataProvider);

							postScriptName = (NSString *)CGFontCopyPostScriptName(loadedFont);

							CGFontRelease(loadedFont);
							CGDataProviderRelease(fontDataProvider);
						}

						if (postScriptName != nil)
						{
							Rtt_TRACE(("Activated fonts in '%s' (%s)", [filename UTF8String], [postScriptName UTF8String]));
						}
						else
						{
							Rtt_TRACE(("Activated fonts in '%s'", [filename UTF8String]));
						}

					}
					else
					{
						Rtt_TRACE_SIM(("Failed to activate fonts in file '%s'", [[fontFileURL path] UTF8String]));
					}
				}
				else
				{
					if (CTFontManagerUnregisterFontsForURL((__bridge CFURLRef)fontFileURL,
														   kCTFontManagerScopeProcess,
														   &error))
					{
						Rtt_TRACE(("Deactivated fonts in '%s'", [filename UTF8String]));
					}
					else
					{
						Rtt_TRACE_SIM(("Failed to deactivate fonts in file '%s'", [[fontFileURL path] UTF8String]));
					}
				}
			}
		}

		// If we've been scanning files for more than 15 seconds, give up (we were probably
		// started in the user's home directory or some other degenerate case)
		NSTimeInterval interval = [[NSDate date] timeIntervalSinceDate:startTime];

		if (interval > 15)
		{
			break;
		}
    }

	if( ! activate )
	{
		AppleFontHelper::ClearCacheAndLoadedFonts();
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

