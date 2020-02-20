//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_DisplayPath_H__
#define _Rtt_DisplayPath_H__

#include "Rtt_DisplayTypes.h"

struct lua_State;

namespace Rtt
{

class DisplayObject;
class Geometry;
class LuaUserdataProxy;
class MLuaUserdataAdapter;
class Matrix;
struct Rect;
struct RenderData;
class Renderer;
class VertexCache;

// ----------------------------------------------------------------------------

// NOTE: DisplayPath instances have no notion of transform
// Only DisplayObjects have that concept!
// 
// Therefore the semantics of Translate in a Path are different than that of an
// Object.  In a path, translate means to translate all cached vertices, or
// if that's not possible, to invalidate any cached vertices.
// In an object, translate means to update its transform and also translate
// any paths it might own.

class DisplayPath
{
	public:
		typedef DisplayPath Self;

	public:
		enum {
			kVerticesMask = 0x1,
			kTexVerticesMask = 0x2,
			kIndicesMask = 0x4,
		};

		static void UpdateGeometry(
			Geometry& dst,
			const VertexCache& src,
			const Matrix& srcToDstSpace,
			U32 flags,
		    ArrayIndex *indices );

	public:
		DisplayPath();
		virtual ~DisplayPath();

	public:
		virtual void Update( RenderData& data, const Matrix& srcToDstSpace ) = 0;
		virtual void UpdateResources( Renderer& renderer ) const = 0;
		virtual void Translate( Real deltaX, Real deltaY ) = 0;
		virtual void GetSelfBounds( Rect& rect ) const = 0;

	public:
		DisplayObject *GetObserver() const { return fObserver; }
		void SetObserver( DisplayObject *newValue ) { fObserver = newValue; }

	public:
		const MLuaUserdataAdapter *GetAdapter() const { return fAdapter; }
		void SetAdapter( const MLuaUserdataAdapter *newValue ) { fAdapter = newValue; }
		void PushProxy( lua_State *L ) const;
		void DetachProxy() { fAdapter = NULL; fProxy = NULL; }

	private:
		DisplayObject *fObserver; // weak ptr
		const MLuaUserdataAdapter *fAdapter; // weak ptr
		mutable LuaUserdataProxy *fProxy;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_DisplayPath_H__
