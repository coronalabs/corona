//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaIOSCordovaLibrary.h"

#import "Cordova/CDVViewController.h"

#include "CoronaLua.h"
#include "CoronaLibrary.h"

#import "CoronaRuntime.h"
#import "CoronaNativeObjectAdapter.h"

// ----------------------------------------------------------------------------

CORONA_EXPORT int luaopen_cordova( lua_State *L )
{
	using namespace Corona;
	return LuaLibrary::OpenWrapper< IOSCordovaLibrary >( L );
}

// ----------------------------------------------------------------------------

@interface CordovaNativeObjectAdapter : NSObject< CoronaNativeObjectAdapter >
{
	CDVViewController *viewController;
}

@property (nonatomic, readonly, assign) CDVViewController *viewController;

@end

// ----------------------------------------------------------------------------

@implementation CordovaNativeObjectAdapter

@synthesize viewController;

- (id)init
{
	self = [super init];
	if ( self )
	{
		viewController = [CDVViewController new];
	}
	return self;
}

- (void)dealloc
{
	[viewController release];
	[super dealloc];
}

- (UIView *)view
{
	return viewController.view;
}

- (int)indexForState:(lua_State *)L key:(const char *)key
{
	return 0;
}

- (BOOL)newIndexForState:(lua_State *)L key:(const char *)key index:(int)index
{
	return false;
}

@end

// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------

IOSCordovaLibrary::IOSCordovaLibrary()
:	fRuntime( nil )
{
}

bool
IOSCordovaLibrary::Initialize( lua_State *L, void *platformContext )
{
	bool result = ( ! fRuntime );

	if ( result )
	{
		fRuntime = (id<CoronaRuntime>)platformContext;
	}

	return result;
}

/*
int
IOSCordovaLibrary::ValueForKey( lua_State *L )
{
	int result = 0;

	const char *key = lua_tostring( L, 2 );

	if ( 0 == strcmp( key, "isAvailable" ) )
	{
		bool isAvailable = [TWTweetComposeViewController canSendTweet];
		lua_pushboolean( L, isAvailable );
		result = 1;
	}

	return result;
}
*/

const luaL_Reg *
IOSCordovaLibrary::GetFunctions() const
{
	static const luaL_Reg kFunctions[] =
	{
		{ "newCleaver", newCleaver },

		{ NULL, NULL }
	};

	return kFunctions;
}

// ----------------------------------------------------------------------------

// cordova.newCleaver( x, y, w, h [, options] )
int
IOSCordovaLibrary::newCleaver( lua_State *L )
{
	using namespace Corona;

	int result = 0;

	float x = lua_tonumber( L, 1 );
	float y = lua_tonumber( L, 2 );
	float w = lua_tonumber( L, 3 );
	float h = lua_tonumber( L, 4 );

	if ( w > 0 && h > 0 )
	{
		CordovaNativeObjectAdapter *adapter = [[CordovaNativeObjectAdapter alloc] init];

		CGRect bounds = CGRectMake( x, y, w, h );

		result = CoronaPushNativeObject( L, bounds, adapter );

/*
		// TODO: Retain this somewhere
		CDVViewController *viewController = [CDVViewController new];

		// TODO: override defaults if specified in options:
		// viewController.wwwFolderName = ;
		// viewController.startPage = ;
		// viewController.useSplashScreen = ;
		// viewController.view.frame = CGRectMake( 0, 0, 320, 480 );

		UIView *cordovaView = viewController.view;
		cordovaView.frame = CGRectMake( 0, 0, 320, 480 );

		UIViewController *rootController = library.GetAppViewController();
		[rootController.view addSubview:cordovaView];
*/
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

