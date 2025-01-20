//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacWebPopup_H__
#define _Rtt_MacWebPopup_H__

#include "Rtt_PlatformWebPopup.h"
#include "Rtt_MPlatform.h"
#import <WebKit/WebKit.h>

// ----------------------------------------------------------------------------

@class NSDictionary;
@class NSString;
@class NSURL;
@class MacWebView;
@class MacWebViewContainer;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class MacWebPopup : public PlatformWebPopup
{
	public:
		typedef MacWebPopup Self;
		typedef PlatformWebPopup Super;

	public:
		MacWebPopup();
		virtual ~MacWebPopup();

	public:
		virtual void SetStageBounds( const Rect& bounds, const Runtime *runtime );

		virtual void Show( const MPlatform& platform, const char *url );
		virtual bool Close();

		virtual void Reset();
		virtual void SetPostParams( const MKeyValueIterable& params );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

		virtual void Preinitialize( const Display& display );
		void SetNeedsRecomputeFromRotationOrScale();
		void RecomputeFromRotationOrScale();
	
	private:
		MacWebViewContainer *fWebView;
		NSString *fMethod;
		NSString *fBaseUrl;
		MPlatform::Directory fBaseDirectory;
		bool fNeedsRecompute;
		const PlatformSurface* fPlatformSurface;
		const RenderingStream* fRenderingStream;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 101100
@protocol WebUIDelegate
@end
@protocol WebFrameLoadDelegate
@end
@protocol WebPolicyDelegate
@end
#endif

@interface MacWebView : WebView
{
	Rtt::MacWebPopup *owner;
	//	UIDeviceOrientation currentOrientation;
}
@property(nonatomic,readonly) Rtt::MacWebPopup* owner;
@end

// Callback/Notification glue code
@interface MacWebViewContainer : NSObject <WebUIDelegate, WebPolicyDelegate, WebFrameLoadDelegate>
{
	Rtt::MacWebPopup *owner;
	MacWebView *fWebView;
	NSRect viewFrame;
	//	UIDeviceOrientation currentOrientation;
}
@property(nonatomic,readonly) Rtt::MacWebPopup* owner;
@end

// ----------------------------------------------------------------------------

#endif // _Rtt_MacWebPopup_H__
