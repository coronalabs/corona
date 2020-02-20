//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_ArrayTupleStruct.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ArrayTupleStruct::ArrayTupleStruct(
	Rtt_Allocator* pAllocator,
	Type t,
	const U8 *attributeSizes,
	int numAttributes )
:	Super( pAllocator, t ),
	fAttributeSizes( pAllocator ),
	fAttributeOffsets( pAllocator )
{
	Rtt_ASSERT( numAttributes > 0 );

	U8 offset = 0;

	for ( int i = 0; i < numAttributes; i++ )
	{
		U8 value = attributeSizes[i];

		fAttributeSizes.Append( value );
		fAttributeOffsets.Append( offset );

		offset += value;
	}

	Super::Initialize( offset );
}

int
ArrayTupleStruct::GetNumAttributes() const
{
	Rtt_ASSERT( fAttributeSizes.Length() == fAttributeOffsets.Length() );

	return fAttributeSizes.Length();
}

void
ArrayTupleStruct::Invalidate()
{
	Super::Empty();
	fAttributeSizes.Empty();
	fAttributeOffsets.Empty();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

