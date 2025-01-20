//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneImageProvider_H__
#define _Rtt_IPhoneImageProvider_H__

#include "Rtt_IPhoneConstants.h"
#include "Rtt_PlatformImageProvider.h"
#include "Rtt_IPhoneMediaProvider.h"
// ----------------------------------------------------------------------------

@class IPhoneMediaProvider;
@class IPhoneImagePickerControllerDelegate;
@class NSDictionary;
@class NSString;
@class UIImage;
@class UIPopoverController;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class IPhoneImageProvider : public PlatformImageProvider
{
	public:
		typedef IPhoneImageProvider Self;
		typedef PlatformImageProvider Super;

	public:
		IPhoneImageProvider( const ResourceHandle<lua_State> & handle );
		virtual ~IPhoneImageProvider();

	protected:
		void Initialize();

	public:
		virtual bool Supports( int source ) const;
		virtual bool HasAccessTo( int source ) const;
		virtual bool Show( int source, const char* filePath, lua_State* L );
//		virtual void SetProperty( U32 mask, bool newValue );

	public:
		void DidDismiss( UIImage* image, NSDictionary* editingInfo );
		// Needed internally for Obj-C callback to dismiss popover on iPad
		void DismissPopoverController();

	private:
		static AuthorizationStatus getAuthorizationStatusForSourceType( int source );

	private:
		IPhoneMediaProvider* fMediaProvider;
		IPhoneImagePickerControllerDelegate* fDelegate;
		NSString *fDstPath;
		bool iOS5statusBarHidden; // workaround for statusbar coming back in iOS 5.0 when hidden
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneImageProvider_H__
