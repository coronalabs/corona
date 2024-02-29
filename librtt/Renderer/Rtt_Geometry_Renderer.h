//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Geometry_Renderer_H__
#define _Rtt_Geometry_Renderer_H__

#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Types.h"
#include "Display/Rtt_DisplayTypes.h"
#include "Core/Rtt_Real.h" // TODO: Rtt_Real.h depends on Rtt_Types being included before it
#include "Core/Rtt_SharedPtr.h"

// ----------------------------------------------------------------------------

struct Rtt_Allocator;
struct CoronaGeometryMappingLayout;

namespace Rtt
{

class DisplayObject;
class MLuaUserdataAdapter;
class LuaUserdataProxy;
class FormatExtensionList;

struct VertexAttributeSupport {
    U32 maxCount;
    bool hasInstancing;
    bool hasPerInstance;
    bool hasDivisors;
    const char * suffix;
};

struct GeometryWriter {
    enum _MaskBits
    {
        kX = 1 << 0,
        kY = 1 << 1,
        kZ = 1 << 2,
	    kPosition = kX | kY | kZ,
        kU = 1 << 3,
        kV = 1 << 4,
        kQ = 1 << 5,
	    kTexcoord = kU | kV | kQ,
        kRS = 1 << 6,
        kGS = 1 << 7,
        kBS = 1 << 8,
        kAS = 1 << 9,
        kColor = kRS | kGS | kBS | kAS,
        kUX = 1 << 10,
        kUY = 1 << 11,
        kUZ = 1 << 12,
        kUW = 1 << 13,
	    kUserdata = kUX | kUY | kUZ | kUW,
	    kMain = kPosition | kTexcoord | kColor | kUserdata,
	    kExtra = 1 << 14,
	    kAll = kMain | kExtra,
        kIsUpdate = 1 << 15
    };
    typedef U16 MaskBits;

	// n.b. if we do these two conditions as a couple asserts in a row, Clang
	// seems to treat the two conditions as being on the same line and breaks
	// the macro
    Rtt_STATIC_ASSERT( ( kExtra == kMain + 1 ) && ( kIsUpdate == kAll + 1 ) );

    static const GeometryWriter& CopyGeometryWriter();

	const void* fContext;
	void (*fWriter)( void*, const void*, const CoronaGeometryMappingLayout*, U32, U32 );
	MaskBits fMask;
	MaskBits fSaved;

	// relevant to user-supplied writers:
	U16 fOffset;
	U8 fComponents;
	U8 fType;
};

// ----------------------------------------------------------------------------

class Geometry : public CPUResource
{
    public:
        typedef CPUResource Super;
        typedef CPUResource Self;

        typedef enum _Mode
        {
            kTriangleStrip,
            kTriangleFan,
            kTriangles,
            kIndexedTriangles,
            kLineLoop,
            kLines
        }
        Mode;
        typedef Mode PrimitiveType; // TODO: Rename Mode to PrimitiveType

        struct Vertex
        {
            void Zero();

            void Set( Real x_,
                Real y_,
                Real u_,
                Real v_,
                U8* optionalColorScale, /* This is an array of length 4. */
                Real* optionalUserData /* This is an array of length 4. */ );

            void SetPos( Real x_,
                Real y_ );

            // 'vertices' is an array of length "vertexCount"
            static void SetColor4ub( U32 vertexCount, Vertex* vertices,
                U8 r, U8 g, U8 b, U8 a );
            static void SetColor( U32 vertexCount, Vertex* vertices,
                Real red, Real green, Real blue, Real alpha );

            static U32 SizeInVertices( U32 size );
            
            Real x, y, z;         // 12 bytes
            Real u, v, q;         // 12 bytes
            U8 rs, gs, bs, as;     // 4 bytes
            Real ux, uy, uz, uw; // 16 bytes
        };

        typedef U16 Index;

        struct ExtensionBlock {
            ExtensionBlock( Rtt_Allocator* allocator );
            ExtensionBlock( ExtensionBlock & block );
            ~ExtensionBlock();
            
            void SetExtensionList( SharedPtr<FormatExtensionList>& list );
            void UpdateData( bool storedOnGPU, U32 count );
            
            SharedPtr<FormatExtensionList> fList;
            Array<Vertex> fVertexData;
            Array<U8>** fInstanceData;
            U32 fCount;
            mutable LuaUserdataProxy *fProxy;
        };
    
        // Generic vertex attribute indices
#ifdef Rtt_WIN_PHONE_ENV
        // Note: These are the indexes that the pre-compiled shaders have assigned to these attributes on Windows Phone.
        //       This is not a good solution. These should be assigned when compiling the shaders or fetched at runtime.
        static const U32 kVertexPositionAttribute = 1;
        static const U32 kVertexTexCoordAttribute = 2;
        static const U32 kVertexColorScaleAttribute = 0;
        static const U32 kVertexUserDataAttribute = 3;
#else
        static const U32 kVertexPositionAttribute = 0;
        static const U32 kVertexTexCoordAttribute = 1;
        static const U32 kVertexColorScaleAttribute = 2;
        static const U32 kVertexUserDataAttribute = 3;
#endif
	
        static U32 FirstExtraAttribute() { return kVertexUserDataAttribute + 1; }

    public:
        // If storeOnGPU is true, a copy of the vertex data will be stored
        // in GPU memory. For large, infrequently changing data, this can
        // improve performance by avoiding the per-frame copy of data from
        // main memory to GPU memory. For smaller, frequently changing data
        // this can actually reduce performance.
        Geometry( Rtt_Allocator* allocator, PrimitiveType type, U32 vertexCount, U32 indexCount, bool storeOnGPU );
        Geometry( const Geometry& geometry );
        ~Geometry();

        virtual ResourceType GetType() const;
        virtual void Allocate();
        virtual void Deallocate();
        virtual bool RequiresCopy() const;

        void SetPrimitiveType( PrimitiveType primitive_type );
        PrimitiveType GetPrimitiveType() const;

        U32 GetVerticesAllocated() const;
        U32 GetIndicesAllocated() const;
        bool GetStoredOnGPU() const;

        void AttachPerVertexColors( ArrayU32* colors, U32 size );

		const U32* GetPerVertexColorData() const;
		U32* GetWriteablePerVertexColorData();
		bool SetVertexColor( U32 index, U32 color );

        const FormatExtensionList * GetExtensionList() const;
        const Vertex* GetExtendedVertexData() const;
        Vertex* GetWritableExtendedVertexData( S32 * length = NULL );
 
        ExtensionBlock * GetExtensionBlock() const { return fExtension; }
        ExtensionBlock * EnsureExtension();
     
        static bool UsesInstancing( const ExtensionBlock* block, const FormatExtensionList* list );
    
        // More space may be allocated than is initially needed. By default,
        // the use count is zero and must be set for Geometry to be useful.
        U32 GetVerticesUsed() const;
        U32 GetIndicesUsed() const;

        void SetVerticesUsed( U32 count );
        void SetIndicesUsed( U32 count );

        // To avoid excess copying, vertex data may be manipulated directly.
        // Invalidate() will result in the data being subloaded to the GPU.
        Vertex* GetVertexData();
        Index* GetIndexData();

        void Resize( U32 vertexCount, bool copyData ); // TODO: Deprecated. Remove.

        // Resize this Geometry's data store. The original data, or as much
        // of it as possible, will be copied to the newly allocated memory.
        void Resize( U32 vertexCount, U32 indexCount, bool copyData );

        // A convenience function which will insert the given Vertex into the
        // data buffer immediately following the last "used" Vertex, resizing
        // as necessary. It is the caller's responsibility to Invalidate().
        void Append( const Vertex& vertex );

    public:
        bool HitTest( Real x, Real y ) const;

    private:
        // Assignment operator made private until we add copy support.
        void operator=( const Geometry& geometry ) { };

        PrimitiveType fPrimitiveType;
        U32 fVerticesAllocated;
        U32 fIndicesAllocated;
        bool fStoredOnGPU;
        ArrayU32* fPerVertexColors;
        Vertex* fVertexData;
        Index* fIndexData;
        U32 fVerticesUsed;
        U32 fIndicesUsed;
        ExtensionBlock* fExtension;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Geometry_Renderer_H__
