//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShapePath_H__
#define _Rtt_ShapePath_H__

#include "Display/Rtt_ClosedPath.h"
#include "Display/Rtt_DisplayTypes.h"
#include "Display/Rtt_VertexCache.h"
#include "Display/Rtt_TesselatorShape.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MShapePathDelegate
{
    public:
        virtual void UpdateGeometry(
            Geometry& dst,
            const VertexCache& src,
            const Matrix& srcToDstSpace,
            U32 flags ) const = 0;
};

class ShapePath : public ClosedPath
{
    public:
        typedef ClosedPath Super;

    public:
        static ShapePath *NewRoundedRect( Rtt_Allocator *pAllocator, Real width, Real height, Real radius );
        static ShapePath *NewCircle( Rtt_Allocator *pAllocator, Real radius );
        static ShapePath *NewPolygon( Rtt_Allocator *pAllocator );
        static ShapePath *NewMesh( Rtt_Allocator *pAllocator, Geometry::PrimitiveType meshType );


    public:
        ShapePath( Rtt_Allocator *pAllocator, TesselatorShape *tesselator );
        virtual ~ShapePath();

        VertexCache & GetFillSource() { return fFillSource; }
        VertexCache & GetStrokeSource() { return fStrokeSource; }

	protected:
		void TesselateFill();
		void TesselateStroke();

        void UpdateFill( RenderData& data, const Matrix& srcToDstSpace );
        void UpdateStroke( const Matrix& srcToDstSpace );

        void CalculateUV( ArrayVertex2& texVertices, Paint *paint, bool canTransformTexture );
    public:
        // MDrawable
        virtual void Update( RenderData& data, const Matrix& srcToDstSpace );
        virtual void UpdateResources( Renderer& renderer ) const;
        virtual void Translate( Real dx, Real dy );
        virtual void GetSelfBounds( Rect& rect ) const;

        void GetTextureVertices( ArrayVertex2& texVertices);
        Rect GetTextureExtents( const ArrayVertex2& texVertices ) const;
    public:
        virtual bool SetSelfBounds( Real width, Real height );

    public:
        TesselatorShape *GetTesselator() { return fTesselator; }
        const TesselatorShape *GetTesselator() const { return fTesselator; }

        const MShapePathDelegate *GetDelegate() const { return fDelegate; }
        void SetDelegate( const MShapePathDelegate *delegate ) { fDelegate = delegate; }

    public:
        bool SetFillVertexColor( U32 index, U32 color );
        bool SetStrokeVertexColor( U32 index, U32 color );

		U32* GetFillVertexColors();
		U32* GetStrokeVertexColors();

		U32 GetFillVertexCount() const;
		U32 GetStrokeVertexCount() const;

        Geometry *GetFillGeometry() const { return fFillGeometry; }
        Geometry *GetStrokeGeometry() const { return fStrokeGeometry; }

    protected:
        Geometry *fFillGeometry;
        Geometry *fStrokeGeometry;
        ArrayIndex fIndexSource;
        VertexCache fFillSource;
        VertexCache fStrokeSource;
        TesselatorShape *fTesselator;
        const MShapePathDelegate *fDelegate;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShapePath_H__
