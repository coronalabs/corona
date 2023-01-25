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
#include "Corona/CoronaGraphics.h"

// ----------------------------------------------------------------------------

struct Rtt_Allocator;

namespace Rtt
{

class CommandBuffer;
class DisplayObject;
class MLuaUserdataAdapter;
class LuaUserdataProxy;
struct FormatExtensionList;

struct VertexAttributeSupport {
    U32 maxCount;
    bool hasInstancing;
    bool hasPerInstance;
    bool hasDivisors;
    const char * suffix;
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

        struct ExtensionAttribute {
            size_t nameHash;
            CoronaVertexExtensionAttributeType type;
            U16 offset : 13;
            U16 components : 2;
            U16 normalized : 1;
            
            U32 GetSize() const;
            bool IsFloat() const { return !!normalized || (CoronaVertexExtensionAttributeType)kAttributeType_Float == type; }
        };
    
        struct ExtensionGroup {
            U32 divisor;
            U16 count;
            U16 size;
            
            bool IsInstanceRate() const { return 0 != divisor; }
            bool IsWindowed() const { return 0 == size; }
            bool NeedsDivisor() const { return divisor > 1; }
            U32 GetWindowAttributeCount( U32 valueCount ) const;
            U32 GetValueCount( U32 instanceCount ) const;
            size_t GetDataSize( U32 instanceCount, const ExtensionAttribute * firstAttribute ) const;
            U32 GetVertexCount( U32 instanceCount, const ExtensionAttribute * firstAttribute ) const;
        };
    
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

// this is outside of Geometry to allow forward declarations, e.g. in ShaderResource
struct FormatExtensionList {
    FormatExtensionList();
    ~FormatExtensionList();
    
    class Iterator {
    public:
        enum GroupFilter { kAllGroups, kVertexRateGroups, kInstancedGroups };
        enum IterationPolicy { kIterateGroups, kIterateAttributes };
        
        Iterator( const FormatExtensionList* list, GroupFilter filter, IterationPolicy policy );
        
    public:
        void Advance();
        bool IsDone() const;
        U32 GetAttributeIndex() const;
        U32 GetGroupIndex() const;
        const Geometry::ExtensionAttribute* GetAttribute() const;
        const Geometry::ExtensionGroup* GetGroup() const;
        
    private:
        void AdvanceGroup();
        void UpdateGroup();

    private:
        GroupFilter fFilter;
        IterationPolicy fPolicy;
        const FormatExtensionList * fList;
        U32 fFirstInGroup;
        U32 fOffsetInGroup;
        U32 fGroupIndex;
    };
    
    static Iterator AllGroups( const FormatExtensionList* list );
    static Iterator AllAttributes( const FormatExtensionList* list );
    static Iterator InstancedGroups( const FormatExtensionList* list );
    
    U32 ExtraVertexCount() const;
    U32 InstanceGroupCount() const;
	bool IsInstanced() const { return instancedByID || HasInstanceRateData(); }
    bool HasInstanceRateData() const;
    bool HasVertexRateData() const;
    void SortNames() const;
    S32 FindHash( size_t hash ) const;
    S32 FindName( const char* name ) const;
    U32 FindGroup( U32 attributeIndex ) const;
    S32 FindCorrespondingInstanceGroup( const Geometry::ExtensionGroup* group, const Geometry::ExtensionAttribute* attribute, U32 * attributeIndex ) const;
    
    static size_t GetExtraVertexSize( const FormatExtensionList * list );
    static size_t GetVertexSize( const FormatExtensionList * list );
    static bool Compatible( const FormatExtensionList * shaderList, const FormatExtensionList * geometryList );
    static bool Match( const FormatExtensionList * list1, const FormatExtensionList * list2 );
    static void ReconcileFormats( CommandBuffer * buffer, const FormatExtensionList * shaderList, const FormatExtensionList * geometryList );
    
    void Build( const CoronaVertexExtension * extension );
    
    struct NamePair {
        const char* str;
        S32 index;
        
        bool operator<( const NamePair & other ) const { return index < other.index; }
    };
    
    Geometry::ExtensionAttribute * attributes;
    Geometry::ExtensionGroup * groups;
    mutable NamePair* names; // allow reordering
    U16 attributeCount;
    U16 groupCount;
	bool instancedByID;
    bool ownsData;
    mutable bool sorted;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Geometry_Renderer_H__
