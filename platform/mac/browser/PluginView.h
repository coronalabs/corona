//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaView.h"

@interface PluginView : CoronaView
{
	NSDictionary *fArguments;
	NSMutableData *fReceivedData;
}

+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments;

- (void)setArguments:(NSDictionary *)arguments;

@end
