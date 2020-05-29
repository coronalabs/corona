//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_SnapshotObject_H__
#define _Rtt_SnapshotObject_H__

#include "Core/Rtt_Geometry.h"
#include "Display/Rtt_RectObject.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class FrameBufferObject;
class GroupObject;
class Renderer;

// ----------------------------------------------------------------------------

class SnapshotObject : public RectObject
{
	Rtt_CLASS_NO_COPIES( SnapshotObject )

	public:
		typedef RectObject Super;
		typedef SnapshotObject Self;

	public:
		enum RenderFlag
		{
			kGroupFlag = 0x1,
			kCanvasFlag = 0x2,
			
			kDefaultFlag = kGroupFlag
		};
		typedef U8 DirtyFlags;

		static RenderFlag RenderFlagForString( const char *str );
//		static const char *StringForRenderFlag( RenderFlag flag );
		
	public:
		typedef enum _CanvasMode
		{
			kAppendMode = 0,
			kDiscardMode,
			
			kNumCanvasModes,
			kDefaultMode = kAppendMode // Preset value
		}
		CanvasMode;
		
		static CanvasMode CanvasModeForString( const char *str );
		static const char *StringForCanvasMode( CanvasMode mode );

	public:
		SnapshotObject(
			Rtt_Allocator *pAllocator,
			Display& display,
			Real contentW, 
			Real contentH );
		virtual ~SnapshotObject();

	public:
		virtual void FinalizeSelf( lua_State *L );

	public:
		void Initialize( lua_State *L, Display& display, Real contentW, Real contentH );

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Prepare( const Display& display );
		virtual void Draw( Renderer& renderer ) const;

	public:
		static void RenderToFBO(
			Renderer& renderer,
			FrameBufferObject *dstFBO, const Rect& bounds,
			const DisplayObject& object, const Color *clearColor );

	protected:
		void DrawGroup( Renderer& renderer, const GroupObject& group, const Color *clearColor ) const;

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

	protected:
		bool ShouldRenderGroup() const { return ( !! ( fDirtyFlags & kGroupFlag ) ); }
		bool ShouldRenderCanvas() const { return ( !! ( fDirtyFlags & kCanvasFlag ) ); }

		void MoveChildren( GroupObject& src );
		void ClearFlag( DirtyFlags flags );

	public:
		void SetDirty( DirtyFlags flags );
		bool IsDirty() const { return 0 != fDirtyFlags; }

		GroupObject& GetGroup() { return * fGroup; }
		const GroupObject& GetGroup() const { return * fGroup; }

		GroupObject& GetCanvas() { return * fCanvas; }
		const GroupObject& GetCanvas() const { return * fCanvas; }

		Color GetClearColor() const { return fClearColor; }
		void SetClearColor( Color newValue ) { fClearColor = newValue; }

		void SetCanvasMode( CanvasMode newValue ) { fCanvasMode = newValue; }
		CanvasMode GetCanvasMode() const { return (CanvasMode)fCanvasMode; }

	public:
		RenderTypes::TextureFilter GetTextureFilter() const;
		RenderTypes::TextureWrap GetTextureWrapX() const;
		RenderTypes::TextureWrap GetTextureWrapY() const;
		void SetTextureFilter( RenderTypes::TextureFilter newValue );
		void SetTextureWrapX( RenderTypes::TextureWrap newValue );
		void SetTextureWrapY( RenderTypes::TextureWrap newValue );

	private:
		GroupObject *fGroup; // Master group
		GroupObject *fCanvas;
		FrameBufferObject *fFrameBufferObject;
		Color fClearColor;
		GroupObject& fOrphanage;
		mutable DirtyFlags fDirtyFlags;
		U8 fCanvasMode;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_SnapshotObject_H__
