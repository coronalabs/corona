//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_OpenPath_H__
#define _Rtt_OpenPath_H__

#include "Display/Rtt_DisplayPath.h"
#include "Renderer/Rtt_RenderData.h"
#include "Rtt_VertexCache.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Paint;

// ----------------------------------------------------------------------------

class OpenPath : public DisplayPath
{
    Rtt_CLASS_NO_COPIES( OpenPath )

    protected:
        enum _Constants
        {
            kIsValid = 0x1,
            kIsStrokeCacheAllocated = 0x2,

            kNumConstants
        };

    public:
        enum _DirtyMask
        {
            kStroke = 0x1,                    // Stroke vertices in renderdata
            kStrokeTexture = 0x2,            // Stroke tex coords in renderdata
            kStrokeSource = 0x4,            // Stroke tesselation
            kStrokeSourceTexture = 0x8,        // Stroke tex tesselation

            kDefault = kStrokeSource,
        };
        typedef U8 DirtyFlags;

    public:
        OpenPath( Rtt_Allocator* pAllocator );
        virtual ~OpenPath();

    protected:
        void Tesselate();

    public:
        virtual void Update( RenderData& data, const Matrix& srcToDstSpace );
        virtual void UpdateResources( Renderer& renderer ) const;
        virtual void Translate( Real dx, Real dy );
        virtual void GetSelfBounds( Rect& rect ) const;

    public:
        void UpdatePaint( RenderData& data );
        void UpdateColor( RenderData& data, U8 objectAlpha );

    public:
        Rtt_INLINE Paint* GetStroke() const { return fStroke; }
        void SetStroke( Paint* newValue );

        Geometry *GetStrokeGeometry() const { return fStrokeGeometry; }
    
        Rtt_INLINE Real GetWidth() const { return fWidth; }
        void SetWidth( Real newValue );

        Rtt_INLINE bool HasStroke() const { return fStroke && GetWidth() > Rtt_REAL_0; }

    public:
        void Insert( S32 index, const Vertex2& p );
        void Remove( S32 index );
        Rtt_INLINE void Append( const Vertex2& p ) { Insert( -1, p ); }
        Rtt_INLINE S32 NumVertices() const { return fStrokePoints.Length(); }
        ArrayVertex2 &GetVertices(){ return fStrokePoints; }

    public:
        void Invalidate( DirtyFlags flags ) { fDirtyFlags |= flags; }
        bool IsValid( DirtyFlags flags ) const { return 0 == (fDirtyFlags & flags); }

    protected:
        void SetValid( DirtyFlags flags ) { fDirtyFlags &= (~flags); }

    public:
        // Use the PropertyMask constants
        // Make properties only read-only to the public
        Rtt_INLINE bool IsProperty( U32 mask ) const { return (fProperties & mask) != 0; }

    protected:
        Rtt_INLINE void ToggleProperty( U32 mask ) { fProperties ^= mask; }
        Rtt_INLINE void SetProperty( U16 mask, bool value )
        {
            const U16 p = fProperties;
            fProperties = ( value ? p | mask : p & ~mask );
        }

    public:
        bool IsStrokeVisible() const;

    public:
        bool SetStrokeVertexColor(U32 index, U32 color);

        U32 GetStrokeVertexCount() const;

    private:
        Paint* fStroke;
        Geometry *fStrokeGeometry;
        VertexCache fStrokeSource;
        ArrayVertex2 fStrokePoints;
        Rect fBounds; // self bounds
        Real fWidth;
        U8 fProperties;
        U8 fDirtyFlags;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_OpenPath_H__
