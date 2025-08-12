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
#include "Core/Rtt_Array.h"

#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_FormatExtensionList.h"

#include "Corona/CoronaGraphics.h"
#include <algorithm>

#include <stdio.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

U32
FormatExtensionList::Attribute::GetSize() const
{
    U32 count = components;
    
    if (kAttributeType_Byte != type)
    {
        count *= 4; // TODO: double, etc.
    }
        
    return count;
}

bool
FormatExtensionList::Attribute::IsFloat() const
{
    return !!normalized || (CoronaVertexExtensionAttributeType)kAttributeType_Float == type;
}

U32
FormatExtensionList::Group::GetWindowAttributeCount( U32 valueCount ) const
{
    return valueCount + count - 1;
}

U32
FormatExtensionList::Group::GetValueCount( U32 instanceCount ) const
{
    Rtt_ASSERT( 0 != divisor );
    
    return (instanceCount + divisor - 1) / divisor;
}

size_t
FormatExtensionList::Group::GetDataSize( U32 instanceCount, const Attribute * firstAttribute ) const
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
FormatExtensionList::Group::GetVertexCount( U32 instanceCount, const Attribute * firstAttribute ) const
{
    return Geometry::Vertex::SizeInVertices( GetDataSize( instanceCount, firstAttribute ) );
}

FormatExtensionList::FormatExtensionList()
:   fAttributes( NULL ),
    fGroups( NULL ),
    fNames( NULL ),
    fAttributeCount( 0 ),
    fGroupCount( 0 ),
    fOwnsData( false ),
    fSorted( false )
{
}

FormatExtensionList::~FormatExtensionList()
{
    if (fOwnsData)
    {
        Rtt_DELETE( fAttributes );
        Rtt_DELETE( fGroups );
        
        if ( fNames )
        {
            for (U32 i = 0; i < fAttributeCount; ++i)
            {
                Rtt_DELETE( fNames[i].str );
            }
            
            Rtt_DELETE( fNames );
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

FormatExtensionList FormatExtensionList::FromArrays( const Array<Group>& groups, const Array<Attribute>& attributes )
{
    FormatExtensionList list;

    list.fGroups = const_cast<Group*>( groups.ReadAccess() );
    list.fGroupCount = (U16)groups.Length();
    list.fAttributes = const_cast<Attribute*>( attributes.ReadAccess() );
    list.fAttributeCount = (U16)attributes.Length();

    return list;
}

static bool
HasVertexRateGroup( const FormatExtensionList::Group * groups, U32 groupCount )
{
    return groupCount > 0 && !groups[0].IsInstanceRate();
}

U32
FormatExtensionList::ExtraVertexCount() const
{
    U32 extra = 0;
    
    if (HasVertexRateGroup( fGroups, fGroupCount ))
    {
        extra = Geometry::Vertex::SizeInVertices( fGroups[0].size );
    }
    
    return extra;
}

U32
FormatExtensionList::InstanceGroupCount() const
{
    U32 count = fGroupCount;
    
    if (HasVertexRateData())
    {
        --count;
    }
    
    return count;
}

bool
FormatExtensionList::HasInstanceRateData() const
{
    if (1 == fGroupCount)
    {
        return fGroups[0].IsInstanceRate();
    }
    
    else
    {
        return fGroupCount > 1;
    }
}

bool
FormatExtensionList::HasVertexRateData() const
{
    return HasVertexRateGroup( fGroups, fGroupCount );
}

void
FormatExtensionList::SortNames() const
{
    if (fNames && !fSorted)
    {
        std::sort( fNames, fNames + fAttributeCount );
        
        fSorted = true;
    }
}

const char*
FormatExtensionList::FindNameByAttribute( U32 attributeIndex, S32* index ) const
{
    if ( fNames && attributeIndex < fAttributeCount )
    {
        NamePair& pair = fNames[attributeIndex];

        if ( index )
        {
            *index = pair.index;
        }

        return pair.str->GetString();
    }

    else
    {
        return NULL;
    }
}

S32
FormatExtensionList::FindHash( size_t hash ) const
{
    for (U32 i = 0; i < fAttributeCount; ++i)
    {
        if (hash == fAttributes[i].nameHash)
        {
            return i;
        }
    }
    
    return -1;
}

S32
FormatExtensionList::FindName( const char* name ) const
{
    if (fNames)
    {
        for (U32 i = 0; i < fAttributeCount; ++i)
        {
            if (0 == Rtt_StringCompare( name, fNames[i].str->GetString() ))
            {
                FormatExtensionList::NamePair temp = fNames[i]; // move to front
                
                fNames[i] = fNames[0];
                fNames[0] = temp;
                fSorted = false;
                
                return temp.index;
            }
        }
    }
    
    return -1;
}

U32
FormatExtensionList::FindGroup( U32 attributeIndex ) const
{
    Rtt_ASSERT( attributeIndex < fAttributeCount );

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
FormatExtensionList::FindCorrespondingInstanceGroup( const Group* group, const Attribute* attribute, U32 * attributeIndex ) const
{
    for (auto iter = FormatExtensionList::InstancedGroups( this ); !iter.IsDone(); iter.Advance())
    {
        const Group* curGroup = iter.GetGroup();
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

    const char* name = list->FindNameByAttribute( iter.GetAttributeIndex() );
    
    if ( NULL == name )
    {
        name = "??? (name set missing)";
    }
    
    Rtt_TRACE_SIM(( formatStr, name ));

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
        Rtt_TRACE_SIM(( "WARNING: no geometry list, so shader list implicitly not a subset." ));
        
        return false;
    }
    
    else if ((shaderList->fGroupCount > geometryList->fGroupCount) || (shaderList->fAttributeCount > geometryList->fAttributeCount))
    {
        Rtt_TRACE_SIM(( "WARNING: shader list has more attributes or groups than geometry list." ));
        
        return false;
    }
	
    else
    {
		bool geometryIsInstanced = geometryList->fInstancedByID;
		
        for (auto iter = FormatExtensionList::AllAttributes( shaderList ); !iter.IsDone(); iter.Advance())
        {
            Attribute shaderAttribute = *iter.GetAttribute();
            S32 geometryAttributeIndex = geometryList->FindHash( shaderAttribute.nameHash );
            
            if (-1 == geometryAttributeIndex)
            {
                return LogNameError( "WARNING: no attribute named `%s` in geometry list.", shaderList, iter );
            }
            
            Attribute geometryAttribute = geometryList->fAttributes[geometryAttributeIndex];
            
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
            Group geometryGroup = geometryList->fGroups[geometryGroupIndex], shaderGroup = *iter.GetGroup();
        
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
    
    else if ((list1->fGroupCount != list2->fGroupCount) || (list1->fAttributeCount != list2->fAttributeCount))
    {
        return false;
    }
    
    else
    {
        return 0 == memcmp( list1->fGroups, list2->fGroups, sizeof(Group) * list1->fGroupCount )
        && 0 == memcmp( list1->fAttributes, list2->fAttributes, sizeof(Attribute) * list1->fAttributeCount );
        // TODO? strictly speaking, the components need not agree
    }
}

void
FormatExtensionList::Build( Rtt_Allocator* allocator, const CoronaVertexExtension * extension )
{
    Array< Attribute > attributes( allocator );
    Array< Group > groups( allocator );
    LightPtrArray< String > names( allocator );
    
    for (int i = 0; i < extension->count; ++i)
    {
        const CoronaVertexExtensionAttribute & attributeData = extension->attributes[i];

        Attribute attribute = {};
        
        attribute.type = attributeData.type;
        attribute.components = attributeData.components;
        attribute.normalized = attributeData.normalized;
        
        // Any window gets its own group.
        if (attributeData.windowSize > 1)
        {
            Group group = {};
            
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
                String* str = Rtt_NEW( allocator, String( allocator, attributeData.name ) );

                names.Append( str );
               
                char buf[3] = {}; // two digits, cf. kWindowSizeLimit
                
                sprintf( buf, "%i", j + 1 );
                
                str->Append( buf );
                
                attribute.nameHash = str->GetHash32();
                
                attributes.Append( attribute );
                
                attribute.offset += attribute.GetSize();
            }
            
            groups.Append( group );
        }
        
        // Otherwise, merge desciptors with common divisors.
        else
        {
            S32 attributeIndex = 0, groupIndex = -1;
            
            if (!attributeData.instancesToReplicate) // not instanced?
            {
                if (!HasVertexRateGroup( groups.ReadAccess(), groups.Length() )) // assumed to be first
                {
                    groups.Insert( 0, Group{} );
                }
                
                groupIndex = 0;
                attributeIndex = groups[0].count;
            }
            
            else
            {
                for (S32 j = 0, length = groups.Length(); j < length; ++j) // group exists?
                {
                    attributeIndex += groups[j].count; // skip over group, if windowed, or to end otherwise
                    
                    if (!groups[j].IsWindowed() && groups[j].divisor == attributeData.instancesToReplicate)
                    {
                        groupIndex = j;
                        
                        break;
                    }
                }
                
                if (-1 == groupIndex)
                {
                    groupIndex = groups.Length();
                    
                    Group group = {};
                    
                    group.divisor = attributeData.instancesToReplicate;

                    groups.Append( group );
                }
            }
            
            Group & group = groups[groupIndex];
            
            attribute.offset = group.size;

            group.size += attribute.GetSize();
            
            ++group.count;
     
            String* str = Rtt_NEW( allocator, String( allocator, attributeData.name ) );
            
            attribute.nameHash = str->GetHash32();
            
            attributes.Insert( attributeIndex, attribute );
            names.Insert( attributeIndex, str );
        }
    }
    
    fAttributeCount = (U16)attributes.Length();
    fAttributes = Rtt_NEW( NULL, Attribute[fAttributeCount] );

    memcpy( fAttributes, attributes.ReadAccess(), fAttributeCount * sizeof(Attribute) );
    
    fGroupCount = (U16)groups.Length();
    fGroups = Rtt_NEW( NULL, Group[fGroupCount] );
    
    memcpy( fGroups, groups.ReadAccess(), fGroupCount * sizeof(Group) );
    
    fNames = Rtt_NEW( NULL, FormatExtensionList::NamePair[names.Length()] );

    for (U32 i = 0; i < fAttributeCount; ++i)
    {
        fNames[i].str = names[i];
        fNames[i].index = (S32)i; // preserve attribute index against reordering
    }
    
	fInstancedByID = !HasInstanceRateData() && !!extension->instanceByID;
    fOwnsData = true;
    fSorted = true;
}

void
FormatExtensionList::ReconcileFormats( Rtt_Allocator* allocator, CommandBuffer * buffer, const FormatExtensionList * shaderList, const FormatExtensionList * geometryList, U32 offset )
{
    Array<Attribute> attributes( allocator );
    Array<Group> groups( allocator );
    Array<U32> groupIndices( allocator );

    Rtt_ASSERT( geometryList || !shaderList );

    for (auto iter = FormatExtensionList::AllAttributes( shaderList ); !iter.IsDone(); iter.Advance())
    {
        S32 geometryAttributeIndex = geometryList->FindHash( iter.GetAttribute()->nameHash );
        
        Rtt_ASSERT( -1 != geometryAttributeIndex );
        
        const Attribute& geometryAttribute = geometryList->fAttributes[geometryAttributeIndex];
        
        attributes.Append( geometryAttribute );
        
        U32 groupIndex = geometryList->FindGroup( geometryAttributeIndex );
        
        bool indexFound = false;

        for (S32 i = 0, length = groupIndices.Length(); i < length && !indexFound; i++)
        {
            indexFound = groupIndex == groupIndices[i];
        }

        if ( !indexFound )
        {
            groupIndices.Append( groupIndex );
         
            Group group = geometryList->fGroups[groupIndex];
            
            group.count = 0;
            groupIndex = groups.Length();
            
            groups.Append( group );
        }
        
        ++groups[groupIndex].count;
    }
    
    FormatExtensionList reconciledList = FormatExtensionList::FromArrays( groups, attributes );
    
    U32 geometryAttributeCount = geometryList ? geometryList->fAttributeCount : 0;
    
    buffer->BindVertexFormat( &reconciledList, geometryAttributeCount, FormatExtensionList::GetVertexSize( geometryList ), offset );
}

FormatExtensionList::Iterator::Iterator( const FormatExtensionList* list, GroupFilter filter, IterationPolicy policy )
:   fList( NULL ),
    fFilter( filter ),
    fPolicy( policy ),
    fFirstInGroup( 0 ),
    fOffsetInGroup( 0 ),
    fGroupIndex( 0 )
{
    if (list && list->fGroupCount > 0)
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
            
            advanceGroup = fOffsetInGroup == fList->fGroups[fGroupIndex].count;
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

const FormatExtensionList::Attribute*
FormatExtensionList::Iterator::GetAttribute() const
{
    return fList ? &fList->fAttributes[GetAttributeIndex()] : NULL;
}

const FormatExtensionList::Group*
FormatExtensionList::Iterator::GetGroup() const
{
    return fList ? &fList->fGroups[fGroupIndex] : NULL;
}

void
FormatExtensionList::Iterator::AdvanceGroup()
{
    fFirstInGroup += fList->fGroups[fGroupIndex].count;
    
    ++fGroupIndex;
}

void
FormatExtensionList::Iterator::UpdateGroup()
{
    fOffsetInGroup = 0;

    if (fGroupIndex == fList->fGroupCount)
    {
        fList = NULL;
    }
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
