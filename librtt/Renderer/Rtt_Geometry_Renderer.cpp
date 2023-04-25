//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_String.h"
#include "Core/Rtt_Assert.h"

#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Display/Rtt_DisplayTypes.h"
#include "Renderer/Rtt_RenderTypes.h"

#include "CoronaGraphics.h"

#include <cstring>
#include <stddef.h>
#include <string.h>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void Geometry::Vertex::Zero()
{
    memset(this, 0, sizeof(*this));
}

void Geometry::Vertex::Set(Real x_,
    Real y_,
    Real u_,
    Real v_,
    U8* optionalColorScale,
    Real* optionalUserData)
{
    x = x_;
    y = y_;
    u = u_;
    v = v_;

    if (optionalColorScale)
    {
        rs = optionalColorScale[0];
        gs = optionalColorScale[1];
        bs = optionalColorScale[2];
        as = optionalColorScale[3];
    }

    if (optionalUserData)
    {
        ux = optionalUserData[0];
        uy = optionalUserData[1];
        uz = optionalUserData[2];
        uw = optionalUserData[3];
    }
}

void Geometry::Vertex::SetPos(Real x_,
    Real y_)
{
    x = x_;
    y = y_;
}

void Geometry::Vertex::SetColor4ub(U32 vertexCount, Vertex* vertices,
    U8 r, U8 g, U8 b, U8 a)
{
    for (U32 i = 0;
        i < vertexCount;
        ++i)
    {
        Vertex& vert = vertices[i];

        vert.rs = r;
        vert.gs = g;
        vert.bs = b;
        vert.as = a;
    }
}

void Geometry::Vertex::SetColor(U32 vertexCount, Vertex* vertices,
    Real red, Real green, Real blue, Real alpha)
{
    U8 r = (U8)(red * 255.0f);
    U8 g = (U8)(green * 255.0f);
    U8 b = (U8)(blue * 255.0f);
    U8 a = (U8)(alpha * 255.0f);

    SetColor4ub(vertexCount, vertices, r, g, b, a);
}

U32
Geometry::Vertex::SizeInVertices( U32 size )
{
    size += sizeof(Geometry::Vertex) - 1; // round up

    return size / sizeof(Geometry::Vertex);
}

U32
Geometry::ExtensionAttribute::GetSize() const
{
    U32 count = components;
    
    if (kAttributeType_Byte != type)
    {
        count *= 4; // TODO: double, etc.
    }
        
    return count;
}

bool
Geometry::ExtensionAttribute::IsFloat() const
{
    return !!normalized || (CoronaVertexExtensionAttributeType)kAttributeType_Float == type;
}

U32
Geometry::ExtensionGroup::GetWindowAttributeCount( U32 valueCount ) const
{
    return valueCount + count - 1;
}

U32
Geometry::ExtensionGroup::GetValueCount( U32 instanceCount ) const
{
    Rtt_ASSERT( 0 != divisor );
    
    return (instanceCount + divisor - 1) / divisor;
}

size_t
Geometry::ExtensionGroup::GetDataSize( U32 instanceCount, const ExtensionAttribute * firstAttribute ) const
{
    U32 valueCount = GetValueCount( instanceCount );
    
    if (IsWindowed())
    {
        Rtt_ASSERT( firstAttribute );
        
        return GetWindowAttributeCount( valueCount ) * firstAttribute->GetSize();
    }
    
    else
    {
        return size * valueCount;
    }
}

U32
Geometry::ExtensionGroup::GetVertexCount( U32 instanceCount, const ExtensionAttribute * firstAttribute ) const
{
    return Geometry::Vertex::SizeInVertices( GetDataSize( instanceCount, firstAttribute ) );
}

FormatExtensionList::FormatExtensionList()
:   attributes(NULL),
    groups(NULL),
    names(NULL),
    attributeCount(0),
    groupCount(0),
    ownsData(false),
    sorted(false)
{
}

FormatExtensionList::~FormatExtensionList()
{
    if (ownsData)
    {
        Rtt_DELETE( attributes );
        Rtt_DELETE( groups );
        
        if ( names )
        {
            for (U32 i = 0; i < attributeCount; ++i)
            {
                Rtt_DELETE( names[i].str );
            }
            
            Rtt_DELETE( names );
        }
    }
}

FormatExtensionList::Iterator
FormatExtensionList::AllGroups( const FormatExtensionList* list )
{
    return Iterator( list, Iterator::kAllGroups, Iterator::kIterateGroups );
}

FormatExtensionList::Iterator
FormatExtensionList::AllAttributes( const FormatExtensionList* list )
{
    return Iterator( list, Iterator::kAllGroups, Iterator::kIterateAttributes );
}

FormatExtensionList::Iterator
FormatExtensionList::InstancedGroups( const FormatExtensionList* list )
{
    return Iterator( list, Iterator::kInstancedGroups, Iterator::kIterateGroups );
}

static bool
HasVertexRateGroup( const Geometry::ExtensionGroup * groups, U32 groupCount )
{
    return groupCount > 0 && !groups[0].IsInstanceRate();
}

U32
FormatExtensionList::ExtraVertexCount() const
{
    U32 extra = 0;
    
    if (HasVertexRateGroup( groups, groupCount ))
    {
        extra = Geometry::Vertex::SizeInVertices( groups[0].size );
    }
    
    return extra;
}

U32
FormatExtensionList::InstanceGroupCount() const
{
    U32 count = groupCount;
    
    if (HasVertexRateData())
    {
        --count;
    }
    
    return count;
}

bool
FormatExtensionList::HasInstanceRateData() const
{
    if (1 == groupCount)
    {
        return groups[0].IsInstanceRate();
    }
    
    else
    {
        return groupCount > 1;
    }
}

bool
FormatExtensionList::HasVertexRateData() const
{
    return HasVertexRateGroup( groups, groupCount );
}

void
FormatExtensionList::SortNames() const
{
    if (names && !sorted)
    {
        std::sort( names, names + attributeCount );
        
        sorted = true;
    }
}


S32
FormatExtensionList::FindHash( size_t hash ) const
{
    for (U32 i = 0; i < attributeCount; ++i)
    {
        if (hash == attributes[i].nameHash)
        {
            return i;
        }
    }
    
    return -1;
}

S32
FormatExtensionList::FindName( const char* name ) const
{
    if (names)
    {
        for (U32 i = 0; i < attributeCount; ++i)
        {
            if (0 == Rtt_StringCompare( name, names[i].str ))
            {
                FormatExtensionList::NamePair temp = names[i]; // move to front
                
                names[i] = names[0];
                names[0] = temp;
                sorted = false;
                
                return temp.index;
            }
        }
    }
    
    return -1;
}

U32
FormatExtensionList::FindGroup( U32 attributeIndex ) const
{
    Rtt_ASSERT( attributeIndex < attributeCount );

    S32 groupIndex = -1;
    
    for (auto iter = FormatExtensionList::AllGroups( this ); !iter.IsDone(); iter.Advance())
    {
        if (iter.GetAttributeIndex() > attributeIndex)
        {
            break;
        }
        
        ++groupIndex;
    }
    
    Rtt_ASSERT( groupIndex >= 0 );
    
    return groupIndex;
}


S32
FormatExtensionList::FindCorrespondingInstanceGroup( const Geometry::ExtensionGroup* group, const Geometry::ExtensionAttribute* attribute, U32 * attributeIndex ) const
{
    for (auto iter = FormatExtensionList::InstancedGroups( this ); !iter.IsDone(); iter.Advance())
    {
        const Geometry::ExtensionGroup* curGroup = iter.GetGroup();
        bool windowingMatches = curGroup->IsWindowed() == group->IsWindowed();
        bool divisorsMatch = curGroup->divisor == group->divisor;
        bool namesMatch = !group->IsWindowed() || attribute->nameHash == iter.GetAttribute()->nameHash;
    
        // Matching groups must have the same divisor.
        // In the case of windowed groups, they must also both be such, and
        // also agree in names: however, if any name matches, they all do.
        if (windowingMatches && divisorsMatch && namesMatch)
        {
            if (attributeIndex)
            {
                *attributeIndex = iter.GetAttributeIndex();
            }
        
            return (S32)iter.GetGroupIndex();
        }
    }
            
    return -1;
}

size_t
FormatExtensionList::GetExtraVertexSize( const FormatExtensionList * list )
{
    if (list)
    {
        return list->ExtraVertexCount() * sizeof(Geometry::Vertex);
    }
    
    else
    {
        return 0;
    }
}

size_t
FormatExtensionList::GetVertexSize( const FormatExtensionList* list )
{
    return sizeof(Geometry::Vertex) + GetExtraVertexSize( list );
}

static bool
LogNameError( const char * formatStr, const FormatExtensionList * list, const FormatExtensionList::Iterator & iter )
{
    list->SortNames();

    const char* name = "??? (name set missing)";
    
    if (list->names)
    {
        name = list->names[iter.GetAttributeIndex()].str;
    }
    
    Rtt_Log( formatStr, name );

    return false;
}

bool
FormatExtensionList::Compatible( const FormatExtensionList * shaderList, const FormatExtensionList * geometryList )
{
    if (NULL == shaderList) // implicitly the reduced case, below...
    {
        return true;
    }
    
    else if (NULL == geometryList) // ...whereas this is the opposite
    {
        Rtt_Log( "WARNING: no geometry list, so shader list implicitly not a subset." );
        
        return false;
    }
    
    else if ((shaderList->groupCount > geometryList->groupCount) || (shaderList->attributeCount > geometryList->attributeCount))
    {
        Rtt_Log( "WARNING: shader list has more attributes or groups than geometry list." );
        
        return false;
    }
	
    else
    {
		bool geometryIsInstanced = geometryList->instancedByID;
		
        for (auto iter = FormatExtensionList::AllAttributes( shaderList ); !iter.IsDone(); iter.Advance())
        {
            Geometry::ExtensionAttribute shaderAttribute = *iter.GetAttribute();
            S32 geometryAttributeIndex = geometryList->FindHash( shaderAttribute.nameHash );
            
            if (-1 == geometryAttributeIndex)
            {
                return LogNameError( "WARNING: no attribute named `%s` in geometry list.", shaderList, iter );
            }
            
            Geometry::ExtensionAttribute geometryAttribute = geometryList->attributes[geometryAttributeIndex];
            
            if (shaderAttribute.type != geometryAttribute.type)
            {
                if (shaderAttribute.IsFloat() != geometryAttribute.IsFloat())
                {
                    return LogNameError( "WARNING: type clash with attribute named `%s`: float vs. non-float", shaderList, iter );
                }
                
                else
                {
                    return LogNameError( "WARNING: incompatible types with attribute named `%s`", shaderList, iter );
                    
                    // TODO: too strict? not enough? byte / int compatiblity, etc. signedness etc.
                }
            }
            
            U32 geometryGroupIndex = geometryList->FindGroup( geometryAttributeIndex );
            Geometry::ExtensionGroup geometryGroup = geometryList->groups[geometryGroupIndex], shaderGroup = *iter.GetGroup();
        
			geometryIsInstanced |= geometryGroup.IsInstanceRate();
			
            if (shaderGroup.divisor != geometryGroup.divisor)
            {
                return LogNameError( "WARNING: instancing count clash with attribute named `%s`", shaderList, iter );
            }
            
            if (shaderGroup.IsWindowed() != geometryGroup.IsWindowed())
            {
                return LogNameError( "WARNING: structuring clash with attribute named `%s`: windowed vs. not windowed", shaderList, iter );
            }
        
            if (shaderGroup.IsWindowed() && shaderGroup.count != geometryGroup.count)
            {
                return LogNameError( "WARNING: structuring clash with attribute named `%s`: windows have different sizes", shaderList, iter );
            }
        }
		
		if (geometryIsInstanced && !shaderList->IsInstanced())
		{
			Rtt_Log( "WARNING: geometry list instantiated, but not shader list." );
			
			return false;
		}

        return true;
    }
}

bool
FormatExtensionList::Match( const FormatExtensionList * list1, const FormatExtensionList * list2 )
{
    if (NULL == list1 || NULL == list2)
    {
        return list1 == list2;
    }
    
    else if ((list1->groupCount != list2->groupCount) || (list1->attributeCount != list2->attributeCount))
    {
        return false;
    }
    
    else
    {
        return 0 == memcmp( list1->groups, list2->groups, sizeof(Geometry::ExtensionGroup) * list1->groupCount )
        && 0 == memcmp( list1->attributes, list2->attributes, sizeof(Geometry::ExtensionAttribute) * list1->attributeCount );
        // TODO? strictly speaking, the components need not agree
    }
}

void
FormatExtensionList::Build( const CoronaVertexExtension * extension )
{
    std::vector< Geometry::ExtensionAttribute > vattributes;
    std::vector< Geometry::ExtensionGroup > vgroups;
    std::vector< std::string > vnames;
    
    for (int i = 0; i < extension->count; ++i)
    {
        const CoronaVertexExtensionAttribute & attributeData = extension->attributes[i];

        Geometry::ExtensionAttribute attribute = {};
        
        attribute.type = attributeData.type;
        attribute.components = attributeData.components;
        attribute.normalized = attributeData.normalized;
        
        // Any window gets its own group.
        if (attributeData.windowSize > 1)
        {
            Geometry::ExtensionGroup group = {};
            
            group.count = attributeData.windowSize;
            group.divisor = attributeData.instancesToReplicate;
            
            if (0 == group.divisor)
            {
                ++group.divisor;
            }
            
            const unsigned int kWindowSizeLimit = 100; // should be more than enough, i.e. max attribs far less
            
            Rtt_ASSERT( attributeData.windowSize < kWindowSizeLimit );
            
            for (int j = 0; j < attributeData.windowSize; ++j)
            {
                vnames.push_back( attributeData.name );
               
                char buf[3] = {}; // two digits, cf. kWindowSizeLimit
                
                sprintf( buf, "%i", j + 1 );
                
                vnames.back() += buf;
                
                attribute.nameHash = std::hash<std::string>{}( vnames.back() );
                
                vattributes.push_back( attribute );
                
                attribute.offset += attribute.GetSize();
            }
            
            vgroups.push_back( group );
        }
        
        // Otherwise, merge desciptors with common divisors.
        else
        {
            S32 attributeIndex = 0, groupIndex = -1;
            
            if (!attributeData.instancesToReplicate) // not instanced?
            {
                if (!HasVertexRateGroup( vgroups.data(), vgroups.size() )) // assumed to be first
                {
                    vgroups.insert( vgroups.begin(), Geometry::ExtensionGroup{} );
                }
                
                groupIndex = 0;
                attributeIndex = vgroups[0].count;
            }
            
            else
            {
                for (U32 j = 0; j < vgroups.size(); ++j) // group exists?
                {
                    attributeIndex += vgroups[j].count; // skip over group, if windowed, or to end otherwise
                    
                    if (!vgroups[j].IsWindowed() && vgroups[j].divisor == attributeData.instancesToReplicate)
                    {
                        groupIndex = j;
                        
                        break;
                    }
                }
                
                if (-1 == groupIndex)
                {
                    groupIndex = vgroups.size();
                    
                    Geometry::ExtensionGroup group = {};
                    
                    group.divisor = attributeData.instancesToReplicate;

                    vgroups.push_back( group );
                }
            }
            
            Geometry::ExtensionGroup & group = vgroups[groupIndex];
            
            attribute.offset = group.size;

            group.size += attribute.GetSize();
            
            ++group.count;
     
            attribute.nameHash = std::hash<std::string>{}( attributeData.name );
            
            vattributes.insert( vattributes.begin() + attributeIndex, attribute );
            vnames.insert( vnames.begin() + attributeIndex, attributeData.name );
        }
    }
    
    attributeCount = vattributes.size();
    attributes = Rtt_NEW( NULL, Geometry::ExtensionAttribute[attributeCount] );

    memcpy( attributes, vattributes.data(), attributeCount * sizeof(Geometry::ExtensionAttribute) );
    
    groupCount = vgroups.size();
    groups = Rtt_NEW( NULL, Geometry::ExtensionGroup[groupCount] );
    
    memcpy( groups, vgroups.data(), groupCount * sizeof(Geometry::ExtensionGroup) );
    
    names = Rtt_NEW( NULL, FormatExtensionList::NamePair[vnames.size()] );

    for (U32 i = 0; i < attributeCount; ++i)
    {
        char* name = Rtt_NEW( NULL, char[vnames[i].length() + 1] );
        
        strcpy( name, vnames[i].c_str() );
        
        names[i].str = name;
        names[i].index = (S32)i; // preserve attribute index against reordering
    }
    
	instancedByID = !HasInstanceRateData() && !!extension->instanceByID;
    ownsData = true;
    sorted = true;
}

void
FormatExtensionList::ReconcileFormats( CommandBuffer * buffer, const FormatExtensionList * shaderList, const FormatExtensionList * geometryList, U32 offset )
{
    FormatExtensionList reconciledList = {};
    
    std::vector<Geometry::ExtensionAttribute> attributes;
    std::vector<Geometry::ExtensionGroup> groups;
    std::vector<U32> groupIndices;

    Rtt_ASSERT( geometryList || !shaderList );

    for (auto iter = FormatExtensionList::AllAttributes( shaderList ); !iter.IsDone(); iter.Advance())
    {
        S32 geometryAttributeIndex = geometryList->FindHash( iter.GetAttribute()->nameHash );
        
        Rtt_ASSERT( -1 != geometryAttributeIndex );
        
        const Geometry::ExtensionAttribute& geometryAttribute = geometryList->attributes[geometryAttributeIndex];
        
        attributes.push_back( geometryAttribute );
        
        U32 groupIndex = geometryList->FindGroup( geometryAttributeIndex );
        
        if (groupIndices.end() == std::find( groupIndices.begin(), groupIndices.end(), groupIndex ))
        {
            groupIndices.push_back( groupIndex );
         
            Geometry::ExtensionGroup group = geometryList->groups[groupIndex];
            
            group.count = 0;
            groupIndex = groups.size();
            
            groups.push_back( group );
        }
        
        ++groups[groupIndex].count;
    }
    
    reconciledList.attributes = attributes.data();
    reconciledList.groups = groups.data();
    reconciledList.attributeCount = attributes.size();
    reconciledList.groupCount = groups.size();
    
    U32 geometryAttributeCount = geometryList ? geometryList->attributeCount : 0;
    
    buffer->BindVertexFormat( &reconciledList, geometryAttributeCount, FormatExtensionList::GetVertexSize( geometryList ), offset );
}

FormatExtensionList::Iterator::Iterator( const FormatExtensionList* list, GroupFilter filter, IterationPolicy policy )
:   fList(NULL),
    fFilter(filter),
    fPolicy(policy),
    fFirstInGroup(0),
    fOffsetInGroup(0),
    fGroupIndex(0)
{
    if (list && list->groupCount > 0)
    {
        fList = list;
        
        if (kInstancedGroups == fFilter && list->HasVertexRateData())
        {
            AdvanceGroup();
        }
        
        UpdateGroup();
    }
}

void
FormatExtensionList::Iterator::Advance()
{
    if (!IsDone())
    {
        bool advanceGroup = kIterateGroups == fPolicy;
        
        if (!advanceGroup)
        {
            Rtt_ASSERT( kIterateAttributes == fPolicy );

            ++fOffsetInGroup;
            
            advanceGroup = fOffsetInGroup == fList->groups[fGroupIndex].count;
        }
        
        if (advanceGroup)
        {
            if (kVertexRateGroups == fFilter)
            {
                fList = NULL;
            }
            
            else
            {
                AdvanceGroup();
                UpdateGroup();
            }
        }
    }
}

bool
FormatExtensionList::Iterator::IsDone() const
{
    return NULL == fList;
}

U32
FormatExtensionList::Iterator::GetAttributeIndex() const
{
    return fFirstInGroup + fOffsetInGroup;
}

U32
FormatExtensionList::Iterator::GetGroupIndex() const
{
    return fGroupIndex;
}

const Geometry::ExtensionAttribute*
FormatExtensionList::Iterator::GetAttribute() const
{
    return fList ? &fList->attributes[GetAttributeIndex()] : NULL;
}

const Geometry::ExtensionGroup*
FormatExtensionList::Iterator::GetGroup() const
{
    return fList ? &fList->groups[fGroupIndex] : NULL;
}

void
FormatExtensionList::Iterator::AdvanceGroup()
{
    fFirstInGroup += fList->groups[fGroupIndex].count;
    
    ++fGroupIndex;
}

void
FormatExtensionList::Iterator::UpdateGroup()
{
    fOffsetInGroup = 0;

    if (fGroupIndex == fList->groupCount)
    {
        fList = NULL;
    }
}

Geometry::ExtensionBlock::ExtensionBlock( Rtt_Allocator* allocator )
:   fVertexData( allocator ),
    fInstanceData( NULL ),
    fCount( 0 ),
    fProxy( NULL )
{
}
 
Geometry::ExtensionBlock::ExtensionBlock( ExtensionBlock & block )
:   fVertexData( block.fVertexData.Allocator() ),
    fInstanceData( NULL ),
    fCount( block.fCount ),
    fProxy( NULL )
{
    fVertexData.Reserve( block.fVertexData.Length() );

    memcpy( fVertexData.WriteAccess(), block.fVertexData.ReadAccess(), block.fVertexData.Length() * sizeof(Geometry::Vertex) );
 
    if (block.fList.NotNull())
    {
        SetExtensionList( block.fList ); // will allocate any instance data too
    }

    if (block.fInstanceData)
    {
        Rtt_ASSERT( fList.NotNull() );
        Rtt_ASSERT( fInstanceData );

        for (U32 i = 0, count = fList->InstanceGroupCount(); i < count; ++i)
        {
            Rtt_ASSERT( fInstanceData[i] );
            
            fInstanceData[i]->Reserve( block.fInstanceData[i]->Length() );

            memcpy( fInstanceData[i]->WriteAccess(), block.fInstanceData[i]->ReadAccess(), block.fInstanceData[i]->Length() );
        }
    }
}

Geometry::ExtensionBlock::~ExtensionBlock()
{
    if (fInstanceData)
    {
        Rtt_ASSERT( fList.NotNull() );
        
        for (U32 i = 0, iMax = fList->InstanceGroupCount(); i < iMax; ++i)
        {
            Rtt_DELETE( fInstanceData[i] );
        }
        
        Rtt_FREE( fInstanceData );
    }
}

void
Geometry::ExtensionBlock::SetExtensionList( SharedPtr<FormatExtensionList> &list )
{
    Rtt_ASSERT( fList.IsNull() && list.NotNull() );
    
    fList = list;

    if (list->HasInstanceRateData())
    {
        Rtt_Allocator* allocator = fVertexData.Allocator();

        fInstanceData = (Array<U8>**)Rtt_MALLOC( allocator, sizeof(Array<U8>*) * fList->InstanceGroupCount() );

        for (U32 i = 0, count = fList->InstanceGroupCount(); i < count; ++i)
        {
            fInstanceData[i] = Rtt_NEW( allocator, Array<U8>( allocator ) );
        }
    }
}

void
Geometry::ExtensionBlock::UpdateData( bool storedOnGPU, U32 count )
{
    Rtt_ASSERT( fList.NotNull() );
    
    if (count > 0)
    {
        U32 extraCount = fList->ExtraVertexCount();
        
        // For (vertex-rate) GPU data, we consolidate the original + extended
        // vertex data before uploading it. To avoid too much complexity, the
        // original streams are left intact, whereas the extended GPU streams
        // are laid out thus: "stub" vertex #1, extended components #1; "stub"
        // vertex #2, extended part #2; etc. The stubs are populated from the
        // original stream just before uploading.
        // (TODO? The extended part comprises one or more Vertex values; this
        // might waste some space, but lets us ignore alignment concerns when
        // splicing. However, consider using a U8 stream instead.)
        if (storedOnGPU)
        {
            ++extraCount;
        }
        
        fVertexData.PadToSize( count * extraCount, Geometry::Vertex{} );
    }
}

Geometry::Geometry(Rtt_Allocator* allocator, PrimitiveType type, U32 vertexCount, U32 indexCount, bool storeOnGPU)
    : CPUResource(allocator),
    fPrimitiveType(type),
    fVerticesAllocated(vertexCount),
    fIndicesAllocated(indexCount),
#if Rtt_OPENGL_CLIENT_SIDE_ARRAYS
    fStoredOnGPU(storeOnGPU),
#else
    fStoredOnGPU(true), // Force this to be true if client side arrays are NOT supported
#endif
    fPerVertexColors(NULL),
    fVertexData(NULL),
    fIndexData(NULL),
    fVerticesUsed(0),
    fIndicesUsed(0),
    fExtension(NULL)
{
    // Indexed triangles are only supported for the VBO path
    Rtt_ASSERT(!(type == Geometry::kIndexedTriangles && storeOnGPU == false));

    Allocate();
}

Geometry::Geometry(const Geometry& geometry)
    : CPUResource(geometry.GetAllocator()),
    fPrimitiveType(geometry.fPrimitiveType),
    fVerticesAllocated(geometry.fVerticesAllocated),
    fIndicesAllocated(geometry.fIndicesAllocated),
    fStoredOnGPU(geometry.fStoredOnGPU),
    fVertexData(NULL),
    fIndexData(NULL),
    fVerticesUsed(geometry.fVerticesUsed),
    fIndicesUsed(geometry.fIndicesUsed),
    fExtension(NULL)
{
    // Indexed triangles are only supported for the VBO path
    Rtt_ASSERT(!(fPrimitiveType == Geometry::kIndexedTriangles && fStoredOnGPU == false));
    // Copy the given geometry's vertex and index arrays.
    Allocate();
    if (fVertexData)
    {
        memcpy(fVertexData, geometry.fVertexData, fVerticesAllocated * sizeof(Vertex));
    }
    if (fIndexData)
    {
        memcpy(fIndexData, geometry.fIndexData, fIndicesAllocated * sizeof(Index));
    }
    if (geometry.fExtension)
    {
        fExtension = Rtt_NEW( geometry.GetAllocator(), ExtensionBlock( *geometry.fExtension ) );
    }
}

Geometry::~Geometry()
{
    Deallocate();
}

CPUResource::ResourceType Geometry::GetType() const
{
    return CPUResource::kGeometry;
}

void
    Geometry::Allocate()
{
    Deallocate();

    fVertexData = fVerticesAllocated > 0 ? new Vertex[fVerticesAllocated] : NULL;
    fIndexData = fIndicesAllocated > 0 ? new Index[fIndicesAllocated] : NULL;
}

void
    Geometry::Deallocate()
{
    if (fVertexData)
    {
        delete[] fVertexData;
        fVertexData = NULL;
    }

    if (fIndexData)
    {
        delete[] fIndexData;
        fIndexData = NULL;
    }

    if (fExtension)
    {
        delete fExtension;
        fExtension = NULL;
    }
}

bool
    Geometry::RequiresCopy() const
{
    return true;
}

void
    Geometry::SetPrimitiveType(Geometry::PrimitiveType primitive_type)
{
    /**/ //THIS WILL ONLY WORK IF NOT GPU-SIDE?????????
    fPrimitiveType = primitive_type;
}

Geometry::PrimitiveType
    Geometry::GetPrimitiveType() const
{
    return fPrimitiveType;
}

U32
    Geometry::GetVerticesAllocated() const
{
    return fVerticesAllocated;
}

U32
    Geometry::GetIndicesAllocated() const
{
    return fIndicesAllocated;
}

bool
    Geometry::GetStoredOnGPU() const
{
    return fStoredOnGPU;
}

U32
    Geometry::GetVerticesUsed() const
{
    return fVerticesUsed;
}

U32
    Geometry::GetIndicesUsed() const
{
    return fIndicesUsed;
}

void
    Geometry::SetVerticesUsed(U32 count)
{
    Rtt_ASSERT(count <= fVerticesAllocated);
    fVerticesUsed = count;
}

void
    Geometry::SetIndicesUsed(U32 count)
{
    Rtt_ASSERT(count <= fIndicesAllocated);
    fIndicesUsed = count;
}

Geometry::Vertex*
    Geometry::GetVertexData()
{
    return fVertexData;
}

Geometry::Index*
    Geometry::GetIndexData()
{
    return fIndexData;
}

// TODO: Remove. Deprecated
void
    Geometry::Resize(U32 vertexCount, bool copyData)
{
    Resize(vertexCount, 0, copyData);
}

static void SyncPerVertexColors(ArrayU32* perVertexColors, U32 size)
{
    Rtt_ASSERT(perVertexColors);

    perVertexColors->PadToSize(size, ColorWhite());
}

void
    Geometry::Resize(U32 vertexCount, U32 indexCount, bool copyData)
{
    /**/
    //WHY NOT PREVENT new/delete IF WE ALREADY HAVE ENOUGH DATA ALLOCATED???????
    //IN ITS CURRENT STATE, IT'S THE CALLER OF THIS FUNCTION THAT HAS TO PREVENT
    //ITSELF FROM CALLING THIS FUNCTION WHEN IT'S UNNECESSARY!!!!!!!!

    fVerticesAllocated = vertexCount;
    fVerticesUsed = Min(fVerticesUsed, fVerticesAllocated);

    fIndicesAllocated = indexCount;
    fIndicesUsed = Min(fIndicesUsed, fIndicesAllocated);

    // Save the currently allocated data.
    Vertex* existingVertexData = fVertexData;
    Index* existingIndexData = fIndexData;

    // Allocate new data.
    fVertexData = fVerticesAllocated > 0 ? new Vertex[fVerticesAllocated] : NULL;
    fIndexData = fIndicesAllocated > 0 ? new Index[fIndicesAllocated] : NULL;

    // Copy and free the old data.
    if (existingVertexData)
    {
        if (copyData)
        {
            memcpy(fVertexData, existingVertexData, fVerticesUsed * sizeof(Vertex));
        }
        delete[] existingVertexData;
    }

    // Copy and free the old data.
    if (existingIndexData)
    {
        if (copyData)
        {
            memcpy(fIndexData, existingIndexData, fIndicesUsed * sizeof(Index));
        }
        delete[] existingIndexData;
    }

    if (fPerVertexColors && fVerticesAllocated > 0)
    {
        SyncPerVertexColors(fPerVertexColors, fVerticesAllocated);
    }
}

void
    Geometry::Append(const Vertex& vertex)
{
    if (fVerticesUsed == fVerticesAllocated)
    {
        const U32 expansionFactor = 2;
        Resize(expansionFactor * fVerticesAllocated, fIndicesAllocated, true);
    }

    fVertexData[fVerticesUsed] = vertex;
    fVerticesUsed++;
}

// ----------------------------------------------------------------------------

static inline bool
    PointInTriangle(const Real& x, Real const& y, const Geometry::Vertex& p0, const Geometry::Vertex& p1, const Geometry::Vertex& p2)
{
    Real e0x = x - p0.x;
    Real e0y = y - p0.y;
    Real e1x = p1.x - p0.x;
    Real e2x = p2.x - p0.x;
    Real e1y = p1.y - p0.y;
    Real e2y = p2.y - p0.y;

    Real u, v;
    if (Rtt_RealIsZero(e1x))
    {
        if (Rtt_RealIsZero(e2x)) { return false; }
        u = Rtt_RealDiv(e0x, e2x);
        if (u < Rtt_REAL_0 || u > Rtt_REAL_1) { return false; }
        if (Rtt_RealIsZero(e1y)) { return false; }
        v = Rtt_RealDiv((e0y - Rtt_RealMul(e2y, u)), e1y);
        if (v < Rtt_REAL_0) { return false; }
    }
    else
    {
        Real d = Rtt_RealMul(e2y, e1x) - Rtt_RealMul(e2x, e1y);
        if (Rtt_RealIsZero(d)) { return false; }
        u = Rtt_RealDiv((Rtt_RealMul(e0y, e1x) - Rtt_RealMul(e0x, e1y)), d);
        if (u < Rtt_REAL_0 || u > Rtt_REAL_1) { return false; }
        v = Rtt_RealDiv((e0x - Rtt_RealMul(e2x, u)), e1x);
        if (v < Rtt_REAL_0) { return false; }
    }

    if ((u + v) <= Rtt_REAL_1)
    {
        return true;
    }
    else
    {
        return false;
    }
}


static bool
    PointInTriangles(Real x, Real y, const Geometry::Vertex* vertices, int numVertices)
{
    for (int i = 0, iMax = (numVertices / 3) * 3; i < iMax; i += 3)
    {
        if (PointInTriangle(x, y, vertices[i], vertices[i + 1], vertices[i + 2]))
            return true;
    }
    return false;
}


static bool
    PointInIndexedTriangles(Real x, Real y, const Geometry::Vertex* vertices, int numVertices, const Geometry::Index* indices, int numIndices)
{
    if (!indices || !numIndices)
        return false;
    for (int i = 0, iMax = (numIndices / 3) * 3; i < iMax; i += 3)
    {
        const Geometry::Index i1 = indices[i];
        const Geometry::Index i2 = indices[i + 1];
        const Geometry::Index i3 = indices[i + 2];
        if ((i1 < numVertices && i2 < numVertices && i3 < numVertices) && PointInTriangle(x, y, vertices[i1], vertices[i2], vertices[i3]))
            return true;
    }
    return false;
}


static bool
    PointInTriangleStrip(Real x, Real y, const Geometry::Vertex* vertices, int numVertices)
{
    for (int i = 0, iMax = numVertices - 2; i < iMax; i++)
    {
        if (PointInTriangle(x, y, vertices[i], vertices[i + 1], vertices[i + 2]))
            return true;
    }
    return false;
}

static bool
    PointInTriangleFan(Real x, Real y, const Geometry::Vertex* vertices, int numVertices)
{
    for (int i = 0, iMax = numVertices - 1; i < iMax; i++)
    {
        if (PointInTriangle(x, y, vertices[0], vertices[i], vertices[i + 1]))
            return true;
    }
    return false;
}

// ----------------------------------------------------------------------------

bool
    Geometry::HitTest(Real x, Real y) const
{
    switch (GetPrimitiveType()) {
    case kIndexedTriangles:
        return PointInIndexedTriangles(x, y, fVertexData, fVerticesUsed, fIndexData, fIndicesUsed);
    case kTriangles:
        return PointInTriangles(x, y, fVertexData, fVerticesUsed);
    case kTriangleFan:
        return PointInTriangleFan(x, y, fVertexData, fVerticesUsed);
    case kTriangleStrip:
    default:
        return PointInTriangleStrip(x, y, fVertexData, fVerticesUsed);

    }

}

// ----------------------------------------------------------------------------

void
    Geometry::AttachPerVertexColors(ArrayU32* colors, U32 size)
{
    Rtt_ASSERT(!fPerVertexColors || fPerVertexColors == colors);

    bool justBound = !fPerVertexColors;

    fPerVertexColors = colors;

    if (justBound)
    {
        SyncPerVertexColors(fPerVertexColors, size);
    }
}

const U32*
    Geometry::GetPerVertexColorData() const
{
    return fPerVertexColors ? fPerVertexColors->ReadAccess() : NULL;
}

U32*
	Geometry::GetWriteablePerVertexColorData()
{
	return fPerVertexColors ? fPerVertexColors->WriteAccess() : NULL;
}

bool 
	Geometry::SetVertexColor(U32 index, U32 color)
{
    if (!fPerVertexColors || index >= fPerVertexColors->Length())
    {
        return false;
    }

    U32& entry = fPerVertexColors->WriteAccess()[index];
    bool changed = entry != color;
        
    entry = color;

    return changed;
}

const FormatExtensionList*
Geometry::GetExtensionList() const
{
    if (!fExtension)
    {
        return NULL;
    }

    return &*fExtension->fList;
}

const Geometry::Vertex*
    Geometry::GetExtendedVertexData() const
{
    if (!fExtension)
    {
        return NULL;
    }
    
    return fExtension->fVertexData.ReadAccess();
}

Geometry::Vertex*
Geometry::GetWritableExtendedVertexData( S32 * length )
{
    if (!fExtension)
    {
        return NULL;
    }
    
    if (NULL != length)
    {
        *length = fExtension->fVertexData.Length();
    }
    
    return fExtension->fVertexData.WriteAccess();
}

Geometry::ExtensionBlock*
Geometry::EnsureExtension()
{
    if (!fExtension)
    {
        fExtension = Rtt_NEW( GetAllocator(), ExtensionBlock( GetAllocator() ) );
    }
    
    return fExtension;
}
 
bool
Geometry::UsesInstancing( const ExtensionBlock* block, const FormatExtensionList* list )
{
    bool instancedByID = list && list->instancedByID;

    return block && (block->fInstanceData || instancedByID);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
