//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaLibDisplay_H__
#define _Rtt_LuaLibDisplay_H__

#include "Renderer/Rtt_RenderTypes.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class BitmapPaint;
class CameraPaint;
class CompositePaint;
class GradientPaint;
class GroupObject;
class Display;
class DisplayObject;
class Paint;
class Runtime;
class ShapeObject;
struct Vertex2;

// ----------------------------------------------------------------------------

class LuaLibDisplay
{
	public:
		typedef LuaLibDisplay Self;

	public:
		static const char* ReferencePoints();

	public:
		static ShapeObject* PushImage(
			lua_State *L,
			Vertex2* topLeft,
			BitmapPaint* paint,
			Display& display,
			GroupObject *parent,
			Real w,
			Real h );
		static ShapeObject* PushImage(
			lua_State *L,
			Vertex2* topLeft,
			BitmapPaint* paint,
			Display& display,
			GroupObject *parent );
		static void Initialize( lua_State *L, Display& display );

	public:
		static int AssignParentAndPushResult(
			lua_State *L,
			Display& display,
			DisplayObject* o,
			GroupObject *pParent );

	protected:
		static Color toColorFloat( lua_State *L, int index );
		static Color toColorByte( lua_State *L, int index );

	public:
		static int PushColorChannels( lua_State *L, Color c, bool isBytes );
		static Color toColor( lua_State *L, int index, bool isBytes );
		static void ArrayToColor( lua_State *L, int index, Color& outColor, bool isBytes );
		static Paint *LuaNewColor( lua_State *L, int index, bool isBytes );
		static BitmapPaint *LuaNewBitmapPaint( lua_State *L, int index );
		static GradientPaint *LuaNewGradientPaint( lua_State *L, int index );
		static CompositePaint *LuaNewCompositePaint( lua_State *L, int index );
		static CameraPaint *LuaNewCameraPaint( lua_State *L, int index );
		static Paint *LuaNewPaint( lua_State *L, int index );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaLibDisplay_H__
