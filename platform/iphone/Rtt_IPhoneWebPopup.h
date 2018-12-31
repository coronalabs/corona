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

#ifndef _Rtt_IPhoneWebPopup_H__
#define _Rtt_IPhoneWebPopup_H__

#include "Rtt_PlatformWebPopup.h"
#include "Rtt_MPlatform.h"

#include <CoreGraphics/CGGeometry.h>

// ----------------------------------------------------------------------------

@class NSDictionary;
@class NSString;
@class NSURL;
@class UIView;
@class UIWebView;
@class IPhoneWebView;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class IPhoneWebPopup : public PlatformWebPopup
{
	public:
		typedef IPhoneWebPopup Self;
		typedef PlatformWebPopup Super;

	public:
		IPhoneWebPopup();
		virtual ~IPhoneWebPopup();

	public:
		virtual void Show( const MPlatform& platform, const char *url );
		virtual bool Close();

	public:
		virtual void Reset();
		virtual void SetPostParams( const MKeyValueIterable& params );

	protected:
//		NSURL* CreateUrl( NSString *url, NSString *method, NSDictionary *params );
		void Load( NSString *url );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	private:
		//UIView *fPopupView;
		IPhoneWebView *fWebView;
		NSString *fMethod;
		NSString *fBaseUrl;
		MPlatform::Directory fBaseDirectory;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneWebPopup_H__
