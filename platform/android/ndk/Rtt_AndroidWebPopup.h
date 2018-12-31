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


#ifndef _Rtt_AndroidWebPopup_H__
#define _Rtt_AndroidWebPopup_H__

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Geometry.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_Allocator.h"

#include "Rtt_PlatformWebPopup.h"
#include "Rtt_MPlatform.h"

class AndroidDisplayObjectRegistry;
class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidWebViewObject;


class AndroidWebPopup : public PlatformWebPopup
{
	public:
		typedef AndroidWebPopup Self;
		typedef PlatformWebPopup Super;

	public:
		AndroidWebPopup(AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb);
		virtual ~AndroidWebPopup();

	public:
		virtual void Show( const MPlatform& platform, const char *url );
		virtual bool Close();

	public:
		int GetWebViewId() const;
		virtual void Reset();
		virtual void SetPostParams( const MKeyValueIterable& params );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	private:
		AndroidDisplayObjectRegistry *fDisplayObjectRegistry;
		AndroidWebViewObject *fWebView;
		MPlatform::Directory fBaseDirectory;
		String * fBaseUrl;
		bool fHasBackground;
		bool fAutoCancelEnabled;
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidWebPopup_H__
