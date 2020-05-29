//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>

@class MouseOverHighlightButtonCell;

@interface PassiveHitView : NSButton
{
	id theTarget;
	SEL theSelector;
	NSTrackingArea* trackingArea;
	BOOL mouseOverDisabled;
	MouseOverHighlightButtonCell* mouseOverHighlightButtonCell;

}
@property(nonatomic, assign) BOOL mouseOverDisabled;

@end
