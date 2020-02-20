//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaApplicationMain.h"

#import "MyCoronaDelegate.h"

int main(int argc, char *argv[])
{
	@autoreleasepool
	{
		CoronaApplicationMain( argc, argv, [MyCoronaDelegate class] );
	}

	return 0;
}
