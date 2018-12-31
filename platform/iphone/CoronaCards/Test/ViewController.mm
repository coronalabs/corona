// ----------------------------------------------------------------------------
// 
// ViewController.mm
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

#import "CoronaCards/Test/ViewController.h"
#import <Foundation/Foundation.h>

@interface ViewController ()
{
}
@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

	CoronaView *view = (CoronaView *)self.view;
	
	NSDictionary *params = nil;

//	params = [NSDictionary dictionaryWithObjectsAndKeys:
//		[NSNumber numberWithInt:640], @"contentWidth",
//		[NSNumber numberWithInt:480], @"contentHeight",
//		nil];

#ifdef CK_TEST_CORONAVIEWDELEGATE
	view.coronaViewDelegate = self;
#endif

	[view runWithPath:nil parameters:params];
	
#ifdef CK_TEST_CORONAVIEWDELEGATE
	[self performSelector:@selector(foo:) withObject:self afterDelay:1.f];
#endif
}

#ifdef CK_TEST_CORONAVIEWDELEGATE
// Add the following somewhere in Lua:
/*
Runtime:addEventListener( "myEvent", function( event )
	print( "Handling '[coronaView sendEvent:]'" )
	for k,v in pairs( event ) do
		print( "\t", k, v )
	end

	return { abc=123, you="me", truth=true, address={street="Embarcadero", number=1900} }
end )
*/
- (void)foo:(id)sender
{
	CoronaView *view = (CoronaView *)self.view;
	id result = [view sendEvent:[NSDictionary dictionaryWithObjectsAndKeys:
		@"fooValue", @"fooKey",
		@"myEvent", @"name",
		nil]];
		
	NSLog( @"result of sendEvent is (%@)", result );
}

// Add the following somewhere in Lua:
/*
local result = Runtime:dispatchEvent( { name="coronaView", a="b", c=3, d=false, company={name="CoronaLabs", uptime=99.999} })
print( "Result of 'Runtime:dispatchEvent()':" )
for k,v in pairs( result ) do
	print( "\t", k, v )
end
*/
- (id)coronaView:(CoronaView *)view receiveEvent:(NSDictionary *)event
{
	NSLog( @"receiveEvent: %@", event );
	
	// NOTE: values come first, then keys
	return [NSDictionary dictionaryWithObjectsAndKeys:
		@"superfragile", @"fooKey",
		@"blah", @"bar",
		@(3.14), @"pie",
		@(3), @"count",
		@true, @"not-not",
		nil];
}
#endif

@end
