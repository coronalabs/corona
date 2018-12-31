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

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneTemplate.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

// ----------------------------------------------------------------------------

@interface IPhoneTemplateDelegate : NSObject< UIAlertViewDelegate >

+ (id)sharedInstance;

@end

// ----------------------------------------------------------------------------

@implementation IPhoneTemplateDelegate

// Non-thread safe singletons in Obj-C: http://eschatologist.net/blog/?p=178
+ (id)sharedInstance
{
	static id sInstance = nil;

	if ( nil == sInstance )
	{
		sInstance = [[self alloc] init];
	}

	return sInstance;
}

static NSString * const kUrl = @"http://www.coronalabs.com/products/corona-sdk/?utm_source=corona-sdk&utm_medium=corona-sdk&utm_campaign=trial-popup";

-(void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if ( buttonIndex > 0 )
	{
		NSURL *url = [NSURL URLWithString:kUrl];
		[[UIApplication sharedApplication] openURL:url];
	}
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static NSString * const kTitle = @"Corona Trial";
static NSString * const kMessage = @"This message only appears in the trial version.";
static NSString * const kCancelButtonTitle = @"OK";
static NSString * const kOtherButtonTitle = @"Learn more";

void
IPhoneTemplate::WillLoadMain()
{
	UIAlertView *alertView = [[UIAlertView alloc]
								initWithTitle:kTitle
								message:kMessage
								delegate:[IPhoneTemplateDelegate sharedInstance]
								cancelButtonTitle:kCancelButtonTitle
								otherButtonTitles:kOtherButtonTitle, nil];
	[alertView show];
	[alertView release];
}

bool
IPhoneTemplate::IsProperty( Property key )
{
	return false;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

