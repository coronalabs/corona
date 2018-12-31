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

#import "MyLibraryModule.h"

#import "CoronaRuntime.h"

#import <UIKit/UIKit.h>
#import <Twitter/Twitter.h>
#import <Accounts/Accounts.h>

// ----------------------------------------------------------------------------

static bool
IsTwitterFrameworkAvailable()
{
	return nil != [TWTweetComposeViewController class];
}

// local value = mylibrary.canSendTweet()
static int
canSendTweet( lua_State *L )
{
	// No need to do IsTwitterFrameworkAvailable() check. If it's not available,
	// the default value is false anyway.
	lua_pushboolean( L, [TWTweetComposeViewController canSendTweet] );
	return 1;
}

// mylibrary.sendTweet( "Hi" )
static int
sendTweet( lua_State *L )
{
	if ( IsTwitterFrameworkAvailable() )
	{
		id<CoronaRuntime> runtime = (id<CoronaRuntime>)lua_touserdata( L, lua_upvalueindex( 1 ) );

		// First argument contains the message
		const char *msg = lua_tostring( L, 1 );

		// Assign default msg if none
		const char kEmptyMsg[] = "";
		if ( ! msg ) { msg = kEmptyMsg; }

		NSString *message = [NSString stringWithUTF8String:msg];

		// Set up the built-in twitter composition view controller.
		TWTweetComposeViewController *controller = [[TWTweetComposeViewController alloc] init];

		// Set the initial tweet text
		[controller setInitialText:message];

		// Create the completion handler block.
		[controller setCompletionHandler:^(TWTweetComposeViewControllerResult result)
		{
			NSString *output;
			
			switch (result)
			{
				case TWTweetComposeViewControllerResultCancelled:
					// The cancel button was tapped.
					output = @"Tweet cancelled.";
					break;
				case TWTweetComposeViewControllerResultDone:
					// The tweet was sent.
					output = @"Tweet done.";
					break;
				default:
					break;
			}
			
			// Dismiss the tweet composition view controller.
			[runtime.appViewController dismissModalViewControllerAnimated:YES];
		}];

		// Present the tweet composition view controller modally.
		[runtime.appViewController presentModalViewController:controller animated:YES];
	}

	return 0;
}

// ----------------------------------------------------------------------------

const char *
MyLibraryModule::Name()
{
	static const char sName[] = "mylibrary";
	return sName;
}

int
MyLibraryModule::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "canSendTweet", canSendTweet },
		{ "sendTweet", sendTweet },

		{ NULL, NULL }
	};

	// Ensure upvalue is available to library
	void *context = lua_touserdata( L, lua_upvalueindex( 1 ) );
	lua_pushlightuserdata( L, context );

	luaL_openlib( L, Name(), kVTable, 1 ); // leave "mylibrary" on top of stack

	return 1;
}
