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

#ifndef _Rtt_PlatformWebPopup_H__
#define _Rtt_PlatformWebPopup_H__

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Geometry.h"

#include "Rtt_MLuaTableBridge.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

struct Rect;
class Display;
class LuaResource;
class MKeyValueIterable;
class MPlatform;
class PlatformSurface;
class Runtime;

// ----------------------------------------------------------------------------

class PlatformWebPopup : public MLuaTableBridge
{
	public:
		typedef PlatformWebPopup Self;

	public:
		typedef enum _Method
		{
			kGet = 0,
			kPost
		}
		Method;

	public:
		PlatformWebPopup();
		virtual ~PlatformWebPopup();

	public:
		static bool ShouldLoadUrl( const LuaResource& resource, const char *url, bool& inCallback );
		static bool DidFailLoadUrl( const LuaResource& resource, const char *url, const char *errorMsg, S32 errorCode );

	public:
		// Return true to leave popup open; false to close popup
		bool ShouldLoadUrl( const char *url ) const;
		bool DidFailLoadUrl( const char *url, const char *errorMsg, S32 errorCode ) const;

	public:
		virtual void SetStageBounds( const Rect& bounds, const Runtime *runtime );

		virtual const Rect & GetStageBounds() const;
		
	public:
		virtual void Show( const MPlatform& platform, const char *url ) = 0;
		virtual bool Close() = 0;

	public:
		virtual void Reset() = 0;
		virtual void SetPostParams( const MKeyValueIterable& params ) = 0;

	public:
		// Called when dialog is dismissed
		void SetCallback( LuaResource* callback );
		LuaResource* GetCallback() const { return fCallback; }
		bool GetInCallback() const { return fInCallback; }

	// TODO: Duplicate of PlatformDisplayObject.  This sucks.  We should convert 
	// this entire class as a subclass of PlatformDisplayObject
	public:
		virtual void Preinitialize( const Display& display );
		void GetScreenBounds( const Display& display, Rect& outBounds ) const;

	public:
		void SetContentToScreenSx( Real newValue ) { fContentToScreenSx = newValue; }
		void SetContentToScreenSy( Real newValue ) { fContentToScreenSy = newValue; }
		Real GetContentToScreenSx() const { return fContentToScreenSx; }
		Real GetContentToScreenSy() const { return fContentToScreenSy; }

	private:
		LuaResource *fCallback;
		mutable bool fInCallback;
		Real fContentToScreenSx;
		Real fContentToScreenSy;
		Rect fRect;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformWebPopup_H__
