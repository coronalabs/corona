//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneWebPopup_H__
#define _Rtt_IPhoneWebPopup_H__

#include "Rtt_PlatformWebPopup.h"
#include "Rtt_MPlatform.h"

#include <CoreGraphics/CGGeometry.h>

// ----------------------------------------------------------------------------

@class NSString;
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
