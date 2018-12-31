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

#ifndef _Rtt_IPhoneMediaProvider_H__
#define _Rtt_IPhoneMediaProvider_H__

// ----------------------------------------------------------------------------

@class UIImagePickerControllerDelegate;
@class NSDictionary;
@class NSString;
@class UIImage;
@class UIImagePickerController;
@class UIPopoverController;
@class IPhoneMediaPickerControllerDelegate;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class IPhoneMediaProvider
{
	public:
		typedef IPhoneMediaProvider Self;

	public:
		IPhoneMediaProvider();
		virtual ~IPhoneMediaProvider();

	protected:
		void Initialize();

	public:
		virtual void Show( UIImagePickerControllerSourceType source, NSString* mediaTypes, NSObject<UIImagePickerControllerDelegate>* delegate, lua_State* L, int tableIndex, NSTimeInterval maxTime, UIImagePickerControllerQualityType quality );
		UIImagePickerControllerSourceType MediaProviderTypeToImagePickerSourceType( int source );
		bool Internal_IsOS5_0();

	public:
		void DidDismiss( UIImage* image, NSDictionary* editingInfo );
		// Needed internally for Obj-C callback to dismiss popover on iPad
		void DismissPopoverController();
		void ReleaseVariables();

	public:
		IPhoneMediaPickerControllerDelegate* fDelegate;
		UIImagePickerController* fImagePicker;
		UIPopoverController* fPopoverController;
		NSString *fDstPath;
		bool iOS5statusBarHidden; // workaround for statusbar coming back in iOS 5.0 when hidden
};

// ----------------------------------------------------------------------------

}; // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneMediaProvider_H__
