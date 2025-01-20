//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"

bool ValidationSupportMacUI_ValidateAndroidPackageName( const char* packagename, const char* filepath );
bool ValidationSupportMacUI_ValidateIOSAppName( const char* name, const char* filepath );

#ifdef __cplusplus
}
#endif

@interface ValidationSupportMacUI : NSObject <NSAlertDelegate>
{
	/// Window to attach alert sheet to
	NSWindow* parentWindow;
	lua_State* luaState;
}

@property(nonatomic, retain) NSWindow* parentWindow;

- (id) init;
/// Designated initializer
- (id) initWithParentWindow:(NSWindow*)window;
- (bool) runCommonFileValidationTestsInProjectPath:(NSString *)projectpath;
- (bool) runAndroidFileValidationTestsInProjectPath:(NSString*)projectpath;
- (bool) runIOSAppNameValidation:(NSString*)appname;
- (bool) runWebAppNameValidation:(NSString*)appname;
- (bool) runOSXAppNameValidation:(NSString*)appname;

@end
