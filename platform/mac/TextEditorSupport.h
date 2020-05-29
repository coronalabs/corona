//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _TextEditorSupport_H_
#define _TextEditorSupport_H_

#import <Cocoa/Cocoa.h>

#ifdef __cplusplus
extern "C" {
#endif

void TextEditorSupport_LaunchTextEditorWithFile(NSString* file_name, NSInteger line_number);

#ifdef __cplusplus
}
#endif

#endif //_TextEditorSupport_H_