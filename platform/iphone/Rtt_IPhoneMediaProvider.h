//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
