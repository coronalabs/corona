//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _FontHelper_H_
#define _FontHelper_H_

#if 0 // Code currently disabled because we don't use it anymore

#ifdef __cplusplus
extern "C" {
#endif

@class NSError;
@class NSArray;
@class NSString;
@class NSTextField;

_Bool FontHelper_LoadLocalFonts(NSError** the_error, NSArray* font_names);

void FontHelper_ChangeFontOnLabel(NSString* font_name, NSTextField* text_label);


#ifdef __cplusplus
}
#endif

#endif // Code currently disabled because we don't use it anymore 

#endif /* _FontHelper_H_ */
