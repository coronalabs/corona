//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformDisplayObject_H__
#define _Rtt_PlatformDisplayObject_H__

#include "Display/Rtt_DisplayObject.h"

#include "Core/Rtt_ResourceHandle.h"
#include "Rtt_MLuaTableBridge.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Display;
class LuaResource;

// ----------------------------------------------------------------------------

class PlatformDisplayObject : public DisplayObject, public MLuaTableBridge
{
	public:
		typedef PlatformDisplayObject Self;
		typedef DisplayObject Super;

	public:
		static const char kUserInputEvent[];

	public:
		// Derived class should return these when implementing the ProxyVTable() method
		static const LuaProxyVTable& GetTextFieldObjectProxyVTable();
		static const LuaProxyVTable& GetTextBoxObjectProxyVTable();
		static const LuaProxyVTable& GetMapViewObjectProxyVTable();
		static const LuaProxyVTable& GetWebViewObjectProxyVTable();
		static const LuaProxyVTable& GetVideoObjectProxyVTable();

	public:
		typedef enum _TextAlignment
		{
			kTextAlignmentLeft = 0,
			kTextAlignmentCenter = 0,
			kTextAlignmentRight = 0,
		}
		TextAlignment;

	public:
		PlatformDisplayObject();
		virtual ~PlatformDisplayObject();

		virtual void Preinitialize( const Display& display );
		virtual bool Initialize() = 0;

	public:
		// You'll need to implement the following from MDrawable:
		// virtual void Build( const Matrix& parentToDstSpace );
		// virtual void Translate( Real dx, Real dy );
		// virtual void Draw( Renderer& renderer ) const;
		// virtual void GetSelfBounds( Rect& rect ) const;
		
		virtual bool HitTest( Real contentX, Real contentY );

	public:
		// Derived classes will need to implement this using one of the static accessors above.
		// virtual const LuaProxyVTable& ProxyVTable() const;

	public:
		int DispatchEventWithTarget( const MEvent& e );
		void SetHandle( Rtt_Allocator *allocator, const ResourceHandle< lua_State >& handle );
		lua_State* GetL() const;

	public:
		static void CalculateScreenBounds(
						const Display& display,
						Real contentToScreenSx,
						Real contentToScreenSy,
						Rect& inOutBounds );

		void GetScreenBounds( Rect& outBounds ) const;

	public:
		void SetContentToScreenSx( Real newValue ) { fContentToScreenSx = newValue; }
		void SetContentToScreenSy( Real newValue ) { fContentToScreenSy = newValue; }
		Real GetContentToScreenSx() const { return fContentToScreenSx; }
		Real GetContentToScreenSy() const { return fContentToScreenSy; }

		void GetContentOffsets( Real& outX, Real& outY ) const;

	protected:
		virtual int GetNativeProperty( lua_State *L, const char key[] ) const;
		virtual bool SetNativeProperty( lua_State *L, const char key[], int valueIndex );

	public:
		static int getNativeProperty( lua_State *L );
		static int setNativeProperty( lua_State *L );

	protected:
		ResourceHandle< lua_State > *fHandle;
		Real fContentToScreenSx;
		Real fContentToScreenSy;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformDisplayObject_H__
