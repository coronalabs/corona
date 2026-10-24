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
    U32 count = GetComponentCount();
    
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
FormatExtensionList::Group::GetWindowAttributeCount( U32 divisorChunks ) const
{
    return divisorChunks + count - 1;
}

U32
FormatExtensionList::Group::InstanceStreamSizeInDivisorChunks( U32 instanceCount ) const
{
    Rtt_ASSERT( 0 != divisor );
    
    return (instanceCount + divisor - 1) / divisor;
}

U32
FormatExtensionList::Group::InstanceStreamSizeInBytes( U32 instanceCount, const Attribute * firstAttribute ) const
{
    U32 divisorChunkCount = InstanceStreamSizeInDivisorChunks( instanceCount );
    
    if (IsWindowed())
    {
        Rtt_ASSERT( firstAttribute );
        
        return GetWindowAttributeCount( divisorChunkCount ) * firstAttribute->GetSize();
    }
    
    else
    {
        return size * divisorChunkCount;
    }
}

U32
FormatExtensionList::Group::InstanceStreamSizeInVertices( U32 instanceCount, const Attribute * firstAttribute ) const
{
    return Geometry::Vertex::SizeInVertices( InstanceStreamSizeInBytes( instanceCount, firstAttribute ) );
}

FormatExtensionList::FormatExtensionList(  Group* groups, U16 groupCount, Attribute* attributes, U16 attributeCount )
:   fAttributes( attributes ),
    fGroups( groups ),
    fLookupData( NULL ),
    fAttributeCount( attributeCount ),
    fGroupCount( groupCount ),
    fOwnsData( false )
{
}

FormatExtensionList::~FormatExtensionList()
{
    if (fOwnsData)
    {
        Rtt_DELETE( fAttributes );
        Rtt_DELETE( fGroups );
        Rtt_FREE( fLookupData );
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
HasVertexRateGroup( const FormatExtensionList::Group * groups, U32 groupCount )
{
    return ( groupCount > 0 ) && !groups[0].IsInstanceRate();
}

U32
FormatExtensionList::ExtraVertexRateSizeInVertices() const
{
    if ( HasVertexRateGroup( fGroups, fGroupCount ) )
    {
        return Geometry::Vertex::SizeInVertices( fGroups[0].size );
    }
    else
    {
		return 0;
    }
}

U32
FormatExtensionList::InstanceGroupCount() const
{
    return fGroupCount - HasVertexRateData();
}

bool
FormatExtensionList::HasInstanceRateData() const
{
    return InstanceGroupCount() > 0;
}

bool
FormatExtensionList::HasVertexRateData() const
{
    return HasVertexRateGroup( fGroups, fGroupCount );
}

U32
FormatExtensionList::FindGroup( U32 attributeIndex ) const
{
    Rtt_ASSERT( attributeIndex < fAttributeCount );

    S32 groupIndex = -1;
    
	for ( auto&& iter : FormatExtensionList::AllGroups( this ) )
    {
        if (iter.attributeIndex > attributeIndex)
        {
            break;
        }
        
        ++groupIndex;
    }
    
    Rtt_ASSERT( groupIndex >= 0 );
    
    return groupIndex;
}


S32
FormatExtensionList::FindCorrespondingInstanceGroup( const Group* group, const Attribute* attribute, const U8* nameData, U32 * attributeIndex ) const
{
	for ( auto&& iter : FormatExtensionList::InstancedGroups( this ) )
    {
        const Group* curGroup = iter.group;
        bool windowingMatches = curGroup->IsWindowed() == group->IsWindowed();
        bool divisorsMatch = curGroup->divisor == group->divisor;
        bool namesMatch = !group->IsWindowed() || FindMatchingAttribute( nameData, attribute->name_triples_minus_1 + 1 );
        
        // Matching groups must have the same divisor.
        // In the case of windowed groups, they must also both be such, and
        // also agree in names: however, if any name matches, they all do.
        if (windowingMatches && divisorsMatch && namesMatch)
        {
            if (attributeIndex)
            {
                *attributeIndex = iter.attributeIndex;
            }
        
            return (S32)iter.groupIndex;
        }
    }
            
    return -1;
}

U32
FormatExtensionList::ExtraVertexRateSizeInBytes( const FormatExtensionList * list )
{
    return list ? list->ExtraVertexRateSizeInVertices() * sizeof(Geometry::Vertex) : 0;
}

U32
FormatExtensionList::FullVertexRateSizeInBytes( const FormatExtensionList* list )
{
    return sizeof(Geometry::Vertex) + ExtraVertexRateSizeInBytes( list );
}

static bool
LogNameError( const char * formatStr, const FormatExtensionList * list, const FormatExtensionList::Attribute* attribute )
{
	char name[64 + 1];
	String::DecodeIdentifier( name, list->FindAttributeNameData( attribute ), attribute->name_triples_minus_1 + 1 );

    Rtt_TRACE_SIM(( formatStr, *name ? name : "??? (name set missing)" ));

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
    else if ( shaderList == geometryList )
    {
		return true;
    }
    else if ((shaderList->fGroupCount > geometryList->fGroupCount) || (shaderList->fAttributeCount > geometryList->fAttributeCount))
    {
        Rtt_TRACE_SIM(( "WARNING: shader list has more attributes or groups than geometry list." ));
        
        return false;
    }
    else
    {
		bool geometryIsInstanced = geometryList->fInstancedByID;
		
		for ( auto&& iter : FormatExtensionList::AllAttributes( shaderList ) )
        {
            Attribute shaderAttribute = *iter.attribute;
			S32 geometryAttributeIndex = geometryList->FindMatchingAttribute( shaderList, iter.attribute );
            if (-1 == geometryAttributeIndex)
            {
                return LogNameError( "WARNING: no attribute named `%s` in geometry list.", shaderList, iter.attribute );
            }
            
            Attribute geometryAttribute = geometryList->fAttributes[geometryAttributeIndex];
            
            if (shaderAttribute.type != geometryAttribute.type)
            {
                if (shaderAttribute.IsFloat() != geometryAttribute.IsFloat())
                {
                    return LogNameError( "WARNING: type clash with attribute named `%s`: float vs. non-float", shaderList, iter.attribute );
                }
                
                else
                {
                    return LogNameError( "WARNING: incompatible types with attribute named `%s`", shaderList, iter.attribute );
                    
                    // TODO: too strict? not enough? byte / int compatiblity, etc. signedness etc.
                }
            }
            
            U32 geometryGroupIndex = geometryList->FindGroup( geometryAttributeIndex );
            Group geometryGroup = geometryList->fGroups[geometryGroupIndex], shaderGroup = *iter.group;
        
			geometryIsInstanced |= geometryGroup.IsInstanceRate();
			
            if (shaderGroup.divisor != geometryGroup.divisor)
            {
                return LogNameError( "WARNING: instancing count clash with attribute named `%s`", shaderList, iter.attribute );
            }
            else if (shaderGroup.IsWindowed() != geometryGroup.IsWindowed())
            {
                return LogNameError( "WARNING: structuring clash with attribute named `%s`: windowed vs. not windowed", shaderList, iter.attribute );
            }
            else if (shaderGroup.IsWindowed() && shaderGroup.count != geometryGroup.count)
            {
                return LogNameError( "WARNING: structuring clash with attribute named `%s`: windows have different sizes", shaderList, iter.attribute );
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

bool
FormatExtensionList::NamedAttributeInfo::operator<( const NamedAttributeInfo & other ) const
{
	return length < other.length;
}

struct GroupInfo {
	U32 divisor;
	U16 index;
	U16 isWindow;
	
	bool operator<( const GroupInfo& other ) const
	{
		if ( divisor == other.divisor )
		{
			return isWindow < other.isWindow;
		}
		else
		{
			return divisor < other.divisor;
		}
	}
};

static U16
Max( U16 a, U16 b )
{
	return a > b ? a : b;
}

void
FormatExtensionList::Build( Rtt_Allocator* allocator, const CoronaVertexExtension * extension )
{
	GroupInfo groupInfo[kMaxAttribs];
    
    for ( int i = 0; i < extension->count; i++ )
    {
		groupInfo[i].index = i;
		groupInfo[i].isWindow = extension->attributes[i].windowSize > 1;
		groupInfo[i].divisor = extension->attributes[i].instancesToReplicate;
    }

    std::sort( groupInfo, groupInfo + extension->count );

	char windowNames[kMaxAttribs * ( 64 + 2 /* two-digit suffix */ + 1 /* NUL */ )], digits[3] = {};
	NamedAttributeInfo info[kMaxAttribs];
    Attribute attributes[kMaxAttribs];
    Group groups[kMaxAttribs], *curGroup;

	for ( U32 i = 0, windowNamePos = 0, numAttribs; i < extension->count; i++ )
	{
		const CoronaVertexExtensionAttribute &attribData = extension->attributes[ groupInfo[i].index ];
	
		if ( ( 0 == i ) || ( groupInfo[i - 1] < groupInfo[i] ) )
		{
			curGroup = &groups[fGroupCount++];
			curGroup->count = 0;
			curGroup->size = 0;
		
			if ( groupInfo[i].isWindow )
			{
				numAttribs = attribData.windowSize;
				curGroup->divisor = Max( groupInfo[i].divisor, 1 );
			}
			else
			{
				numAttribs = 1;
				curGroup->divisor = groupInfo[i].divisor;
			}
		}
		
		for ( int index = 1; index <= numAttribs; index++ )
		{
			U32 attribIndex = fAttributeCount++;
			Attribute &attrib = attributes[attribIndex];
			
			attrib.type = attribData.type;
			attrib.comp_minus_1 = attribData.components - 1;
			attrib.normalized = attribData.normalized;
			attrib.offset = curGroup->size;
			
			curGroup->count++;
			
			NamedAttributeInfo& ainfo = info[attribIndex];
			size_t nameLength = strlen( attribData.name ); // TODO: comes from Lua, known

			if ( numAttribs > 1 ) // window?
			{
				ainfo.name = windowNames + windowNamePos;
			
				strcpy( windowNames + windowNamePos, attribData.name );

				int onesPos = ( index >= 10 );
				int tensPos = 1 - onesPos;
				
				digits[tensPos] = ( '0' + ( index / 10 ) ) & -( onesPos );
				digits[onesPos] = '0' + ( index % 10 );
				
				strcpy( windowNames + ( windowNamePos + nameLength ), digits );
				
				nameLength += 1 + ( 0 != digits[1] );
				windowNamePos += nameLength + 1;
			}
			else
			{
				curGroup->size += attrib.GetSize();
				
				ainfo.name = attribData.name;
			}
			
			ainfo.attribute = &attrib;
			ainfo.length = nameLength;
        }
	}

    if ( fAttributeCount > 1 )
    {
		std::sort( info, info + fAttributeCount );
    }
    
	U16 triplesBits = 0, namesSize = 0, offsets[15];
	for ( int i = 0, j = 0, prevTriples = -1; i < fAttributeCount; i++ )
	{
        U16 numTriples = String::IdentifierLengthToTriples( info[i].length );
        
        triplesBits |= 1U << ( numTriples - 1 );

		if ( ( i > 0 ) && ( numTriples != prevTriples ) )
		{
			offsets[j++] = namesSize;
		}
		
        namesSize += numTriples * 3 + 1; // include attribute index / final flag
		
		prevTriples = numTriples;
	}
    
	// Tally, in layout order:
	// * 1 16-bit bitset
	// * #triples - 1 16-bit offsets (first one has impicit offset 0)
	// * namesSize bytes for attribute index + name pairs
	
	int triplesCount = Rtt_Pop16( triplesBits );
	
	fLookupData = (U8*)Rtt_MALLOC( L, triplesCount * sizeof(U16) + namesSize );

	memcpy( fLookupData, &triplesBits, sizeof(U16) );
	memcpy( fLookupData + sizeof(U16), offsets, ( triplesCount - 1 ) * sizeof(U16) );
    
    for ( int i = 0, wpos = triplesCount * sizeof(U16), prevTriples = -1; i < fAttributeCount; i++ )
    {
		U16 numTriples = String::IdentifierLengthToTriples( info[i].length );
		if (numTriples == prevTriples)
		{
			fLookupData[wpos - 1] &= ~kFinal; // bad guess: previous identifier was not final
		}
	
		prevTriples = numTriples;

		info[i].attribute->name_offset = wpos;
		info[i].attribute->name_triples_minus_1 = numTriples - 1;
		
		String::EncodeIdentifier( fLookupData + wpos, info[i].name, 16 );

		wpos += numTriples * 3;

		fLookupData[wpos++] = (U8)( info[i].attribute - attributes ) | kFinal; // provisionally make this identifier final one of size
    }
    
    fAttributes = Rtt_NEW( NULL, Attribute[fAttributeCount] );
    fGroups = Rtt_NEW( NULL, Group[fGroupCount] );
	fInstancedByID = !HasInstanceRateData() && !!extension->instanceByID;
    fOwnsData = true;
    
    memcpy( fAttributes, attributes, fAttributeCount * sizeof(Attribute) );
    memcpy( fGroups, groups, fGroupCount * sizeof(Group) );
}

void
FormatExtensionList::ReconcileFormats( Rtt_Allocator* allocator, CommandBuffer * buffer, const FormatExtensionList * shaderList, const FormatExtensionList * geometryList, U32 offset )
{
    Attribute attributes[kMaxAttribs];
    Group groups[kMaxAttribs];
    U32 groupIndices[kMaxAttribs];
	U16 attributeCount = 0, groupCount = 0;

    Rtt_ASSERT( geometryList || !shaderList );

	for ( auto&& iter : FormatExtensionList::AllAttributes( shaderList ) )
    {
		S32 geometryAttributeIndex = geometryList->FindMatchingAttribute( shaderList, iter.attribute );
        
        Rtt_ASSERT( -1 != geometryAttributeIndex );
        
        const Attribute& geometryAttribute = geometryList->fAttributes[geometryAttributeIndex];
        
        attributes[attributeCount++] = geometryAttribute;
        
        U32 groupIndex = geometryList->FindGroup( geometryAttributeIndex );
        
        bool indexFound = false;

        for (S32 i = 0, length = groupCount; i < length && !indexFound; i++)
        {
            indexFound = groupIndex == groupIndices[i];
        }

        if ( !indexFound )
        {
            groupIndices[groupCount] = groupIndex;
         
            Group group = geometryList->fGroups[groupIndex];
            
            group.count = 0;
            groupIndex = groupCount;
            
            groups[groupCount++] = group;
        }
        
        ++groups[groupIndex].count;
    }
    
    FormatExtensionList reconciledList( groups, groupCount, attributes, attributeCount );
    U32 geometryAttributeCount = geometryList ? geometryList->fAttributeCount : 0;
    
    buffer->BindVertexFormat( &reconciledList, geometryAttributeCount, FormatExtensionList::FullVertexRateSizeInBytes( geometryList ), offset );
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

FormatExtensionList::Iterator
FormatExtensionList::Iterator::begin()
{
	return *this;
}

FormatExtensionList::Iterator
FormatExtensionList::Iterator::end()
{
	return *this; // unused, cf. operator!=()
}

const FormatExtensionList::Iterator::CurrentState
FormatExtensionList::Iterator::operator*() const
{
	CurrentState cur;

	cur.groupIndex = fGroupIndex;
	cur.group = fList->fGroups + cur.groupIndex;
	cur.attributeIndex = fFirstInGroup + fOffsetInGroup;
	cur.attribute = fList->fAttributes + cur.attributeIndex;
	
	return cur;
}

FormatExtensionList::Iterator&
FormatExtensionList::Iterator::operator++()
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

	return *this;
}

bool
FormatExtensionList::Iterator::operator!=( const Iterator& ) const
{
	return ( NULL != fList );
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

FormatExtensionList::NamedAttributeIterator::NamedAttributeIterator()
:	fNameData( NULL)
{
}

FormatExtensionList::NamedAttributeIterator::NamedAttributeIterator( const U8* lookupData, U8 specificTriples )
{
	memcpy( &fTriplesBits, lookupData, sizeof(U16) );

	fNameData = lookupData + Rtt_Pop16( fTriplesBits ) * sizeof(U16);
	
	if ( 0 != specificTriples )
	{
		U16 mask = 1U << ( specificTriples - 1 );
		if ( fTriplesBits & mask )
		{
			U16 offsetIndex = Rtt_Pop16( fTriplesBits & ( mask - 1 ) );
			
			if ( offsetIndex > 0 )
			{
				U16 offset;
				
				// implicitly (offsetIndex - 1), but add 1 to skip `fTriplesBits`
				memcpy( &offset, lookupData + offsetIndex * sizeof(U16), sizeof(U16) );
				
				fNameData += offset;
			}
			
			fTriplesBits = mask;
		}
		else
		{
			fTriplesBits = 0;
		}
	}

	PrepareTripleCount();
}

void
FormatExtensionList::NamedAttributeIterator::PrepareTripleCount()
{
	U16 lsb = fTriplesBits & -fTriplesBits;
	
	fTriples = Rtt_Pop16( lsb - 1 ) + 1;
	fAttributeIndex = fTriplesBits ? fNameData[fTriples * 3] : 0;
}

FormatExtensionList::NamedAttributeIterator::CurrentState 
FormatExtensionList::NamedAttributeIterator::operator*() const
{
	CurrentState cur;
	cur.nameData = fNameData;
	cur.attributeIndex = fAttributeIndex & ~kFinal;
	cur.triplesCount = fTriples;

	return cur;
}

FormatExtensionList::NamedAttributeIterator& 
FormatExtensionList::NamedAttributeIterator::operator++()
{
	fNameData += fTriples * 3 + 1;
	
	if ( fAttributeIndex & kFinal )
	{
		fTriplesBits &= fTriplesBits - 1;
		
		PrepareTripleCount();
	}
	else
	{
		fAttributeIndex = fNameData[fTriples * 3];
	}
	
	return *this;
}

bool
FormatExtensionList::NamedAttributeIterator::operator!=( const NamedAttributeIterator& ) const
{
	return ( 0 != fTriplesBits );
}

int
FormatExtensionList::FindAttributeWithName( const char* name ) const
{
	if ( strlen( name ) <= 64 )
	{
		U8 packed[48];
		int n = String::EncodeIdentifier( packed, name, 16 );
		if ( n > 0 )
		{
			return FindMatchingAttribute( packed, n / 3 );
		}
	}
	
	return -1;
}

int
FormatExtensionList::FindMatchingAttribute( const U8* data, U8 triplesCount ) const
{
	int size = triplesCount * 3;
	for ( auto&& iter : NamedAttributesWithTriplesCount( triplesCount ) )
	{
		if ( 0 == memcmp( iter.nameData, data, size ) )
		{
			return iter.attributeIndex;
		}
	}
	
	return -1;
}

int
FormatExtensionList::FindMatchingAttribute( const FormatExtensionList* otherList, const Attribute* otherAttribute ) const
{
	Rtt_ASSERT( otherList );
	Rtt_ASSERT( otherAttribute );

	return FindMatchingAttribute( otherList->fLookupData + otherAttribute->name_offset, otherAttribute->name_triples_minus_1 + 1 );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
