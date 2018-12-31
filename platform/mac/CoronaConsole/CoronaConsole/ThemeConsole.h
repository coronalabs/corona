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

#import <Foundation/Foundation.h>


@interface ThemeConsole : NSObject
{
} // End of Console

+ (NSArray*) ThemeConsoleWithArray: (NSArray*) array;
+ (ThemeConsole *)ThemeConsoleWithDictionary: (NSDictionary *) dictionary;
+ (ThemeConsole *)ThemeConsoleWithJSONString: (NSString *) jsonString usingEncoding: (NSStringEncoding) stringEncoding error: (NSError**) error;
- (id)initWithDictionary: (NSDictionary *) dictionary;
- (NSString*) description;

@property(nonatomic, retain) NSNumber * fontSize;
@property(nonatomic, retain) NSString * timestampBgColor;
@property(nonatomic, retain) NSString * textBgColor;
@property(nonatomic, retain) NSString * fontName;
@property(nonatomic, retain) NSString * errorFgColor;
@property(nonatomic, retain) NSString * timestampFgColor;
@property(nonatomic, retain) NSString * errorBgColor;
@property(nonatomic, retain) NSString * warningBgColor;
@property(nonatomic, retain) NSString * textFgColor;
@property(nonatomic, retain) NSString * warningFgColor;

@end
