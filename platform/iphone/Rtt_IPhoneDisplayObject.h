//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneDisplayObject_H__
#define _Rtt_IPhoneDisplayObject_H__

#include "Rtt_PlatformDisplayObject.h"

#import <UIKit/UIStringDrawing.h>
#import <UIKit/UITextInputTraits.h>

// ----------------------------------------------------------------------------

@class CoronaView;
@class NSString;
@class UIColor;
@class UIView;

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneDisplayObject : public PlatformDisplayObject
{
	public:
		typedef IPhoneDisplayObject Self;
		typedef PlatformDisplayObject Super;

	public:
		static IPhoneDisplayObject* ToObject( lua_State *L, int index, const LuaProxyVTable& table );

	public:
		IPhoneDisplayObject( const Rect& bounds );
		virtual ~IPhoneDisplayObject();

		virtual void Preinitialize( const Display& display );
		void InitializeView( UIView *view );

		void SetFocus();

	public:
		// DisplayObject
		virtual void DidMoveOffscreen();
		virtual void WillMoveOnscreen();

	public:
		// MDrawable
		virtual void Prepare( const Display& display );
		virtual void Translate( Real dx, Real dy );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;
		virtual void SetSelfBounds( Real width, Real height );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	protected:
		virtual id GetNativeTarget() const;
	
		virtual int GetNativeProperty( lua_State *L, const char key[] ) const;
		virtual bool SetNativeProperty( lua_State *L, const char key[], int valueIndex );

	protected:
		UIView* GetView() const { return fView; }
		CoronaView* GetCoronaView() const;

	private:
		Rect fSelfBounds;
		Vertex2 fViewCenter;
		UIView *fView;
		CoronaView *fCoronaView;
		BOOL fHidden; // used by DidMoveOffscreen/WillMoveOnscreen
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneDisplayObject_H__
