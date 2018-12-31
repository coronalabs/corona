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

#ifndef _Rtt_IPhoneVideoProvider_H__
#define _Rtt_IPhoneVideoProvider_H__

#include "Rtt_PlatformVideoProvider.h"
#include "Rtt_IPhoneMediaProvider.h"
// ----------------------------------------------------------------------------

@class IPhoneMediaProvider;
@class IPhoneVideoPickerControllerDelegate;
@class NSDictionary;
@class NSURL;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class IPhoneVideoProvider : public PlatformVideoProvider
{
	public:
		typedef IPhoneVideoProvider Self;
		typedef PlatformVideoProvider Super;

	public:
		IPhoneVideoProvider( const ResourceHandle<lua_State> & handle );
		virtual ~IPhoneVideoProvider();

	protected:
		void Initialize();

	public:
		virtual bool Supports( int source ) const;
		virtual bool Show( int source, lua_State* L, int maxTime, int quality );
//		virtual void SetProperty( U32 mask, bool newValue );

	public:
		void DidDismiss( NSURL* movie, int duration, long size );
		// Needed internally for Obj-C callback to dismiss popover on iPad
		void DismissPopoverController();

	private:
		IPhoneMediaProvider* fMediaProvider;
		IPhoneVideoPickerControllerDelegate* fDelegate;
		bool iOS5statusBarHidden; // workaround for statusbar coming back in iOS 5.0 when hidden
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneVideoProvider_H__
