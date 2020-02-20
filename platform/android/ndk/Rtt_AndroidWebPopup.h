//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
