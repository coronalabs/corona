//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
