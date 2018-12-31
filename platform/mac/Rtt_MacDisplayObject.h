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

#ifndef _Rtt_MacDisplayObject_H__
#define _Rtt_MacDisplayObject_H__

#include "Rtt_PlatformDisplayObject.h"

#import <AppKit/NSText.h>
#import <Foundation/NSGeometry.h>

// ----------------------------------------------------------------------------

@class NSColor;
@class NSView;

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacDisplayObject : public PlatformDisplayObject
{
	public:
		typedef PlatformDisplayObject Super;

	public:
		MacDisplayObject( const Rect& bounds );
		virtual ~MacDisplayObject();

		void InitializeView( NSView *view );

		void SetFocus();

	public:
		// DisplayObject
		virtual void DidMoveOffscreen();
		virtual void WillMoveOnscreen();
		virtual bool CanCull() const;

	public:
		// MDrawable
		virtual void PreInitialize( const Display& display );
		virtual void Prepare( const Display& display );
		virtual void Translate( Real dx, Real dy );
//		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;
		virtual void SetSelfBounds( Real width, Real height );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	protected:
		NSView* GetView() const { return fView; }
		NSView* GetLayerHostSuperView();
		void AddSubviewToLayerHostView();
		void RecomputeNextKeyViews();
		bool IsInSimulator() const;
		float GetSimulatorScale() const;
		void CacheSimulatorScale();
		virtual void DidRescaleSimulator( float previousScale, float currentScale );
        static int setReturnKey( lua_State *L );

#if 0 // See TODO in .mm regarding Rtt_NATIVE_PROPERTIES_MAC
	protected:
		virtual id GetNativeTarget() const;
	
		virtual int GetNativeProperty( lua_State *L, const char key[] ) const;
		virtual bool SetNativeProperty( lua_State *L, const char key[], int valueIndex );
#endif

	protected:
		Rect fSelfBounds;
		Vertex2 fViewCenter;
		NSView *fView;
		// Don't retain this. I'm worried about circular references.
		NSView *fLayerHostSuperView;
		NSRect fNSViewFrame;
		bool fIsHidden; // I need a second flag because I need to hide objects when scaling/rotating
		float fCachedSimulatorScale;
};

// Common functions between text objects
class MacTextObject
{
	public:
		static NSColor* GetTextColor( lua_State *L, int index, bool isByteColorRange );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacDisplayObject_H__
