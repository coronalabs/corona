//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_DisplayObject_H__
#define _Rtt_DisplayObject_H__

// ----------------------------------------------------------------------------

#include "Display/Rtt_MDrawable.h"
#include "Rtt_MLuaProxyable.h"
#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class BitmapMask;
class BitmapPaint;
class GroupObject;
class EmitterObject;
class ContainerObject;
class CPUResource;
class Display;
class DisplayObjectExtensions;
class LuaProxyVTable;
class LuaUserdataProxy;
class MEvent;
class Runtime;
class StageObject;
class Uniform;

struct RenderData;

// ----------------------------------------------------------------------------

// The DisplayObject is the high-level representation of an object on screen.
//
// It stores all the CPU-side data related to rendering. This data needs to get
// flattened into RenderData, which is what happens when Build() is called.
//
// The various parts of RenderData can be broken up into various categories:
// * Program (DisplayObject --- leaf nodes only?)
// * Geometry (DisplayPath)
// * Material (Paint)
//     + Color
//     + Color Offset
//     + Blend mode
//     + Optional:
//         - Texture
// * Bitmap Mask (DisplayObject)
//     + Texture
//     + Transform
// 
// Who owns the RenderData???
// 
class DisplayObject : public MDrawable, public MLuaProxyable
{
	Rtt_CLASS_NO_COPIES( DisplayObject )

	public:
		typedef DisplayObject Self;

		enum RenderFlag
		{
			kGeometryFlag = 0x01,
			kPaintFlag = 0x2,
			kColorFlag = 0x4,
			kTransformFlag = 0x08,
			kProgramFlag = 0x10,
			kProgramDataFlag = 0x20,
			kMaskFlag = 0x40,
			kStageBoundsFlag = 0x80, // TODO: Rename to kContentBoundsFlag
			kContainerFlag = 0x100,

			// NOTE: By default, do NOT dirty:
			// * mask
			// * program
			// * container
			kRenderDefault = (
				kGeometryFlag
				| kPaintFlag
				| kColorFlag
				| kTransformFlag
				| kProgramFlag ),

			kRenderDirty = (
				kRenderDefault
				| kProgramDataFlag
				| kMaskFlag
				| kContainerFlag ),

			kGroupPropagationMask = (
				kGeometryFlag
				| kColorFlag ),
		};
		typedef U16 DirtyFlags;

	protected:
		enum PropertyMask
		{
			// DisplayObject Properties
			kIsVisible = 0x01,
			kIsHitTestable = 0x02,
			kIsUsedByHitTest = 0x04,
			kIsForceDraw = 0x08,
			kIsHitTestMasked = 0x10,
			kIsOffScreen = 0x20,
			kIsExtensionsLocked = 0x40,
			kIsV1Compatibility = 0x80,
			kIsV1ReferencePointUsed = 0x100,
			kIsAnchorChildren = 0x200, // Group-specific property
			kIsRenderedOffscreen = 0x400,
			kIsRestricted = 0x800,

			// NOTE: Current maximum of 16 PropertyMasks!!!
		};
		typedef U16 Properties;

	public:
		enum ListenerMask
		{
			kUnknownListener = 0x0,
			kTouchListener = 0x1,
			kTapListener = 0x2,
			kSpriteListener = 0x4,
			kMouseListener = 0x8,
			kFinalizeListener = 0x10,
		};

		typedef U8 ListenerSet;

		static ListenerMask MaskForString( const char *name );

	public:
        static int KeysForProperties( const char **&keys );
		static GeometricProperty PropertyForKey( Rtt_Allocator *allocator, const char key[] );
		static void DidChangePaint( RenderData& data );

	public:
		DisplayObject();
		virtual ~DisplayObject();

	public:
		virtual void FinalizeSelf( lua_State *L );

	protected:
		// When hit test is masked, intersects rRect with fMask's self bounds and returns result in rRect.
		void UpdateSelfBounds( Rect& rRect ) const;

	public:
		void BuildStageBounds();

	protected:
		void CullOffscreen( const Rect& screenBounds );

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Prepare( const Display& display );
		virtual void Translate( Real deltaX, Real deltaY );
		virtual void GetSelfBoundsForAnchor( Rect& rect ) const;
		virtual bool GetTrimmedFrameOffset( Real& deltaX, Real& deltaY, bool force = false ) const { return false; }
		virtual bool GetTrimmedFrameOffsetForAnchor( Real& deltaX, Real& deltaY ) const { return GetTrimmedFrameOffset( deltaX, deltaY ); }

	public:
		virtual bool HitTest( Real contentX, Real contentY );
		virtual void DidUpdateTransform( Matrix& srcToDst );

	public:
		// Treats as assignment when isNewValue is true
		virtual void Scale( Real sx, Real sy, bool isNewValue );
		virtual void Rotate( Real deltaTheta );

		virtual void DidMoveOffscreen();
		virtual void WillMoveOnscreen();

	public:
		virtual bool CanCull() const;

	public:
		// MLuaProxyable
		virtual void InitProxy( lua_State *L );
		virtual LuaProxy* GetProxy() const;
		virtual void ReleaseProxy();

	public:
		virtual void AddedToParent( lua_State * L, GroupObject * parent );
		virtual void RemovedFromParent( lua_State * L, GroupObject * parent );

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;
		
		void QueueRelease( CPUResource *resource );
		void QueueRelease( LuaUserdataProxy *proxy );

		// Effectively calls o:addEventListener( eventName, listener ) where listener is object on stack at 'index'
		void AddEventListener( lua_State *L, int index, const char *eventName ) const;

		// Returns status from Lua call. No propagation.
		// Caller specifies the number of Lua results on the stack. Caller is responsible for inspecting and popping them.
		// At the Lua level, a 'target' property is present corresponding to the object being dispatched to.
		int DispatchEventWithTarget( lua_State *L, const MEvent& e, int nresults ) const;

		// Used by HitEvents. Returns true if listener handled event
		// Caller assumes that the listener returns a boolean. Defaults to false if none present.
		// At the Lua level, a 'target' property is present corresponding to the object being dispatched to
		bool DispatchEvent( lua_State *L, const MEvent& e ) const;

	public:
		virtual GroupObject* AsGroupObject();

	public:
		//! Local-space to Content-space.
		void LocalToContent( Vertex2& v ) const;

		//! Content-space to Local-space.
		void ContentToLocal( Vertex2& v ) const;

	protected:
		void PropagateImplicitSrcToDstInvalidation();
		void ApplyParent( Matrix& xform ) const;

	protected:
		static void ApplyParentTransform( const DisplayObject& object, Rect& rect );

	public:
		typedef enum _ReferencePoint
		{
			kReferenceCenter = 0,
			kReferenceTopLeft,
			kReferenceTopCenter,
			kReferenceTopRight,
			kReferenceCenterRight,
			kReferenceBottomRight,
			kReferenceBottomCenter,
			kReferenceBottomLeft,
			kReferenceCenterLeft,

			kNumReferencePoints
		}
		ReferencePoint;

		void SetReferencePoint( Rtt_Allocator* pAllocator, ReferencePoint location );
		void ResetReferencePoint();

	public:
		// Returns GetSelfBounds() transformed in dst space
		const Rect& StageBounds() const;
		bool Intersects( const DisplayObject& rhs ) const;

		//const String& StageName() const { return fStageName; }

		bool IsAbove( const DisplayObject& object ) const;

		bool IsReachable() const { return NULL != fLuaProxy; }

		// An object is an orphan when it's been moved outside off the scene graph.
		bool IsOrphan() const { return NULL == GetStage(); }

		const StageObject* GetStage() const;
		StageObject* GetStage() { return const_cast< StageObject* >( static_cast< const Self& >( * this ).GetStage() ); }
		GroupObject* GetParent() const { return ( IsRenderedOffScreen() ? NULL : fParent ); }

		void SetParent( GroupObject* parent );

		// If returns false, then object's Build() method will rebuild the vertices
		// If true, then the vertices are already built and are ready for submission
		bool IsValid( DirtyFlags flags ) const { return 0 == (fDirtyFlags & flags); }
		bool IsDirty() const { return ( !! (fDirtyFlags & kRenderDirty) ); }
//		bool CanDraw() const { return ! IsDirty(); }

	protected:
		void SetValid( DirtyFlags flags ) { fDirtyFlags &= (~flags); }
		void SetValid() { SetValid( kRenderDirty ); }
		DirtyFlags GetDirtyFlags() const { return fDirtyFlags; }

	public:
		// Invalidates canvas/display and fStageBounds 
		void Invalidate( DirtyFlags flags );
		void SetDirty( RenderFlag mask );

		// Unsets valid bit, so next call to Build() forces cache to be rebuilt
		void InvalidateStageBounds();
		bool IsStageBoundsValid() const;

		// Reblits display list to screen
		void InvalidateDisplay();

	protected:
		static void CalculateMaskMatrix( Matrix& dstToMask, const Matrix& srcToDst, const BitmapMask& mask );
		static void UpdateMaskUniform( Uniform& maskUniform, const Matrix& srcToDst, const BitmapMask& mask );
		void UpdateMask();
		Uniform *GetMaskUniform() { return fMaskUniform; }
		virtual void DidSetMask( BitmapMask *mask, Uniform *uniform );

	public:
		BitmapMask *GetMask() { return fMask; }
		const BitmapMask* GetMask() const { return fMask; }
		void SetMask( Rtt_Allocator *allocator, BitmapMask *mask );
		void SetMaskGeometricProperty( enum GeometricProperty p, Real newValue );
		Real GetMaskGeometricProperty( enum GeometricProperty p ) const;

	public:
		// Surround calls to Draw() with these
		virtual void WillDraw( Renderer& renderer ) const;
		virtual void DidDraw( Renderer& renderer ) const;

	public:
		void ResetTransform();
		virtual void SetSelfBounds( Real width, Real height );
		void SetGeometricProperty( enum GeometricProperty p, Real newValue );
		Real GetGeometricProperty( enum GeometricProperty p ) const;

	protected:
		Real GetInternalAnchorX() const { return fAnchorX; }
		Real GetInternalAnchorY() const { return fAnchorY; }

	public:
		// NOTE: Internal anchor uses range [-0.5,0.5]
		Real GetAnchorX() const { return fAnchorX + Rtt_REAL_HALF; }
		Real GetAnchorY() const { return fAnchorY + Rtt_REAL_HALF; }

		void SetAnchorX( Real newValue );
		void SetAnchorY( Real newValue );
		Vertex2 GetAnchorOffset() const;
		bool IsAnchorOffsetZero() const;
		
		// Controls whether the transform is offset by the anchor
		virtual bool ShouldOffsetWithAnchor() const;
		
		bool IsV1Compatibility() const { return (fProperties & kIsV1Compatibility) != 0; }
		virtual void SetV1Compatibility( bool newValue );

		bool IsByteColorRange() const { return IsV1Compatibility(); }

		bool IsV1ReferencePointUsed() const { return (fProperties & kIsV1ReferencePointUsed) != 0; }

		bool IsRestricted() const { return (fProperties & kIsRestricted) != 0; }
		void SetRestricted( bool newValue ) { SetProperty( kIsRestricted, newValue ); }

		Rtt_INLINE bool IsVisible() const { return (fProperties & kIsVisible) != 0; }
		void SetVisible( bool newValue );

		Rtt_INLINE bool IsHitTestable() const { return (fProperties & kIsHitTestable) != 0; }
		Rtt_INLINE void SetHitTestable( bool newValue ) { SetProperty( kIsHitTestable, newValue ); }

		Rtt_INLINE bool IsForceDraw() const { return (fProperties & kIsForceDraw) != 0; }
		Rtt_INLINE void SetForceDraw( bool newValue ) { SetProperty( kIsForceDraw, newValue ); }

		Rtt_INLINE bool IsOffScreen() const { return (fProperties & kIsOffScreen) != 0; }
		Rtt_INLINE void SetOffScreen( bool newValue ) { SetProperty( kIsOffScreen, newValue ); }

		Rtt_INLINE bool IsRenderedOffScreen() const { return (fProperties & kIsRenderedOffscreen) != 0; }
		Rtt_INLINE void SetRenderedOffScreen( bool newValue ) { SetProperty( kIsRenderedOffscreen, newValue ); }

		// Determines whether StageBounds() is clipped by mask.
		// NOTE: If no mask, always returns false.
		Rtt_INLINE bool IsHitTestMasked() const { return fMask && (fProperties & kIsHitTestMasked) != 0; }
		void SetHitTestMasked( bool newValue );

		void SetExtensionsLocked( bool newValue );

		void SetAnchorChildren( bool newValue );
		bool IsAnchorChildren() const { return IsProperty( kIsAnchorChildren ); }

		U8 Alpha() const { return fAlpha; }
		void SetAlpha( U8 newValue );

		U8 AlphaCumulative() const { return fAlphaCumulative; }
		void UpdateAlphaCumulative( U8 alphaCumulativeFromAncestors );

		Rtt_INLINE bool IsNotHidden() const { return IsVisible() && Alpha() > 0; }
		Rtt_INLINE bool ShouldHitTest() const { return IsNotHidden() || IsHitTestable(); }
		bool ShouldDraw() const
		{
			return ( ! IsDirty() && IsNotHidden() ) || IsForceDraw();
		}
		bool ShouldPrepare() const{ return IsDirty() && ShouldHitTest(); }

		void SetTransform( const Transform& newValue );
		const Transform& GetTransform() const { return fTransform; }

	protected:
		Transform& GetTransform() { return fTransform; }

		Matrix& GetSrcToDstMatrix()
		{
			return const_cast< Matrix& >( ((const Self*)this)->GetSrcToDstMatrix() );
		}

	public:
		const Matrix& GetSrcToDstMatrix() const;

		// There should never be a non-const version of GetMatrix().
		// Instead, manipulate fTransform
		const Matrix& GetMatrix() const;

	public:
		// TODO: Do we need this anymore?  Get rid of if we remove snapshot orphanage
		// When dispatching events that propagate between a chain of listeners
		// (e.g. TouchEvent), we need to mark the objects
		// for deferred removal. 
		// TODO: If we keep this, rename to IsUsedByHitTest
		bool IsUsedByHitTest() const { return (fProperties & kIsUsedByHitTest) != 0; }
		void SetUsedByHitTest( bool newValue ) { SetProperty( kIsUsedByHitTest, newValue ); }

	public:
		void SetFocusId( const void *newValue ) { fFocusId = newValue; }
		const void* GetFocusId() const { return fFocusId; }

	protected:
		// Use the PropertyMask constants
		Rtt_INLINE bool IsProperty( U32 mask ) const { return (fProperties & mask) != 0; }
		Rtt_INLINE void ToggleProperty( U32 mask ) { fProperties ^= mask; }
		void SetProperty( U32 mask, bool value );

	public:
		bool HasListener( ListenerMask mask ) const { return ( !! ( fListenerSet & mask ) ); }
		void SetHasListener( ListenerMask mask, bool value )
		{
			const ListenerSet p = fListenerSet;
			fListenerSet = ( value ? p | mask : p & ~mask );
		}

#ifdef Rtt_PHYSICS
	public:
		bool InitializeExtensions( Rtt_Allocator *allocator );
		void RemoveExtensions();
		DisplayObjectExtensions* GetExtensions() const { return fExtensions; }
#endif

    public:
        void SetObjectDesc( const char *objectDesc ) { fObjectDesc = objectDesc; }
        const char *GetObjectDesc() const { return fObjectDesc; }

	private:
		GroupObject* fParent;

		//! "Src" is Local-space.
		//! "Dst" is Content-space.
		//! This is an optimization. This Matrix contains the concatenation
		//! of all transforms between Local-space and Content-space.
		//! This must be kept in sync with any changes.
		Matrix fSrcToDst;

		//! This transform is ONLY relative to the parent (like a "model" transform).
		Transform fTransform;
		mutable Rect fStageBounds;
		mutable LuaProxy* fLuaProxy;
		mutable DisplayObjectExtensions *fExtensions;
		const void *fFocusId;
        const char *fObjectDesc;
        const char *fWhereDefined;
        const char *fWhereChanged;

	private:
		BitmapMask *fMask;
		Uniform *fMaskUniform;

	private:
		Real fAnchorX;
		Real fAnchorY;
		DirtyFlags fDirtyFlags;
		Properties fProperties;
		U8 fAlpha;
		U8 fAlphaCumulative;
		ListenerSet fListenerSet;
		U8 fUnused; // Alignment

		friend class DisplayObjectDrawGuard;
		friend class GroupObject; // Access to CullOffscreen
		friend class EmitterObject;
        friend class DisplayLibrary;
        friend class LuaDisplayObjectProxyVTable;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_DisplayObject_H__
