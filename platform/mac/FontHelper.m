//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "FontHelper.h"
#import <Cocoa/Cocoa.h>

#if 0 // Code currently disabled because we don't use it anymore

// Adapted from http://www.cocoadev.com/index.pl?UsingCustomFontsInYourCocoaApplications
_Bool FontHelper_LoadLocalFonts(NSError** err, NSArray* fontnames)
{

	NSString *resourcePath, *fontsFolder,*errorMessage;    
	NSURL *fontsURL;
	resourcePath = [[NSBundle mainBundle] resourcePath];
	if (!resourcePath) 
	{
		errorMessage = @"Failed to load fonts! no resource path...";
		goto error;
	}
	fontsFolder = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"/fonts"];
	
	NSFileManager *fm = [NSFileManager defaultManager];
	
	if (![fm fileExistsAtPath:fontsFolder])
	{
		errorMessage = @"Failed to load fonts! Font folder not found...";
		goto error;
	}
	if(fontsURL = [NSURL fileURLWithPath:fontsFolder])
	{
		OSStatus status;
		FSRef fsRef;
		CFURLGetFSRef((CFURLRef)fontsURL, &fsRef);
		status = ATSFontActivateFromFileReference(&fsRef, kATSFontContextLocal, kATSFontFormatUnspecified, 
												  NULL, kATSOptionFlagsDefault, NULL);
		if (status != noErr)
		{
			errorMessage = @"Failed to acivate fonts!";
			goto error;
		}
	}
	if (fontnames != nil)
	{
		NSFontManager *fontManager = [NSFontManager sharedFontManager];
		for (NSString *fontname in fontnames)
		{
			BOOL fontFound = [[fontManager availableFonts] containsObject:fontname]; 
			if (!fontFound)
			{
				errorMessage = [NSString stringWithFormat:@"Required font not found:%@",fontname];
				goto error;
			}
		}
	}
	return true;
error:
	
	if (err != NULL) {
		NSString *localizedMessage = NSLocalizedString(errorMessage, @"");
		NSDictionary *userInfo = [NSDictionary dictionaryWithObject:localizedMessage forKey:NSLocalizedDescriptionKey];
		*err = [[NSError alloc] initWithDomain:NSCocoaErrorDomain code:0 userInfo:userInfo];
	}
	
	return false;
	
}


// For a given NSTextField, change the font to the specified font
// Will preserve the existing font size.
void FontHelper_ChangeFontOnLabel(NSString* font_name, NSTextField* text_label)
{
	NSFont* old_label_font = [text_label font];
	NSFont* new_label_font = [NSFont fontWithName:font_name size:[old_label_font pointSize]];
	[text_label setFont:new_label_font];
	
}

#endif // Code currently disabled because we don't use it anymore 
