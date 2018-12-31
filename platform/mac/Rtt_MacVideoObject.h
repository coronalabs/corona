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

#ifndef _Rtt_MacVideoObject_H__
#define _Rtt_MacVideoObject_H__

#include "Rtt_MacDisplayObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacVideoObject : public MacDisplayObject
{
	public:
		typedef MacVideoObject Self;
		typedef MacDisplayObject Super;

	public:
		MacVideoObject( const Rect& bounds );
		virtual ~MacVideoObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

		// DisplayObject
		virtual const LuaProxyVTable& ProxyVTable() const;

	protected:
		static int Load( lua_State *L );
		static int Play( lua_State *L );
		static int Pause( lua_State *L );
		static int Seek( lua_State *L );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	protected:
		virtual id GetNativeTarget() const;
};

// ----------------------------------------------------------------------------

	
} // namespace Rtt

#import <AVFoundation/AVFoundation.h>

@interface Rtt_VideoObjectView : NSView
{
	Rtt::MacVideoObject *fOwner;
	NSURL *fUrl;
	BOOL touchTogglesPlay;
	BOOL fIsMuted;
	BOOL fIsReadyToPlay;
	BOOL fDidEnd;
	BOOL fWasPlaying;
}

@property(nonatomic, assign, getter=owner, setter=setOwner:) Rtt::MacVideoObject *fOwner;
@property(nonatomic, retain) AVPlayer *player;
@property(nonatomic, assign) BOOL touchTogglesPlay;

- (id)initWithFrame:(CGRect)rect;

- (void)load:(NSURL*)url;
- (void)unload;
- (void)play;
- (void)pause;
- (bool)isPaused;
- (void)seek:(CMTime)t;

- (NSString*)videoFillMode;
- (void)setVideoFillMode:(NSString *)fillMode;

- (BOOL)isMuted;
- (void)setMuted:(BOOL)newValue;

- (void)setSuspended:(BOOL)newValue;

@end

// ----------------------------------------------------------------------------

#endif // _Rtt_MacVideoObject_H__
