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

#import <Cocoa/Cocoa.h>

// ----------------------------------------------------------------------------

namespace Rtt
{
	class MSimulatorServices;
}

// ----------------------------------------------------------------------------

@interface NewProjectController : NSObject< NSOpenSavePanelDelegate >
{
	@private
		IBOutlet NSWindow *fWindow;
		NSString *fProjectName;
		NSInteger fTemplateIndex;
		IBOutlet NSPopUpButton *fScreenSize;
		NSInteger fScreenSizeIndex;
		NSNumber *fScreenWidth;
		NSNumber *fScreenHeight;
		NSInteger fOrientationIndex;
		//IBOutlet NSMatrix *fOrientation;

		NSString *fResourcePath;

		NSWindow *fParent;
		NSString *fProjectPath;

		const Rtt::MSimulatorServices *fServices;
}

@property (nonatomic, readonly, assign, getter=window) NSWindow *fWindow;
@property (nonatomic, readwrite, assign, getter=projectName, setter=setProjectName:) NSString *fProjectName;
@property (nonatomic, readwrite, assign, getter=templateIndex, setter=setTemplateIndex:) NSInteger fTemplateIndex;
@property (nonatomic, readwrite, assign, getter=screenSizeIndex, setter=setScreenSizeIndex:) NSInteger fScreenSizeIndex;
@property (nonatomic, readwrite, assign, getter=screenWidth, setter=setScreenWidth:) NSNumber *fScreenWidth;
@property (nonatomic, readwrite, assign, getter=screenHeight, setter=setScreenHeight:) NSNumber *fScreenHeight;
@property (nonatomic, readwrite, assign, getter=orientationIndex, setter=setOrientationIndex:) NSInteger fOrientationIndex;
@property (nonatomic, readwrite, retain, getter=projectPath, setter=setProjectPath:) NSString *fProjectPath;

@property (nonatomic, readwrite, assign, getter=services, setter=setServices:) const Rtt::MSimulatorServices *fServices;


- (id)initWithWindowNibName:(NSString*)windowNibName resourcePath:(NSString*)path;

- (void)beginSheetModalForWindow:(NSWindow*)parent;

- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo;

- (BOOL)formComplete;

@end

// ----------------------------------------------------------------------------
