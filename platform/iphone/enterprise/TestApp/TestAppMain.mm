//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaApplicationMain.h"

#import "TestAppCoronaDelegate.h"

int main(int argc, char *argv[])
{
	@autoreleasepool
	{
		CoronaApplicationMain( argc, argv, [TestAppCoronaDelegate class] );
	}

	return 0;
}
