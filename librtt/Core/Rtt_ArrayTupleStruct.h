//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ArrayTupleStruct_H__
#define _Rtt_ArrayTupleStruct_H__

#include "Core/Rtt_ArrayTuple.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/**

# ArrayTupleStruct

ArrayTupleStruct is an array of tuples in which each tuple is organized into
a structure of attribute-tuples (e.g. position, color, etc.)

Every attribute-tuple is made up of components.

Each component is the same type as defined by the Type enum. 

# Example

If we have two attribute-tuples, say position and texture coordinates in 2D, then the
array would look something like the following:

-- -- -- -- ---- -- -- -- --
x0 y0 s0 t0 ...  xN yN sN tN
-- -- -- -- ---- -- -- -- --

Each struct is composed of a position coordinate (x,y) and a texture coordinate (s,t).

The corresponding contructor call would look like:

``````
const int kPositionAttribute = 0;
const int kTextureCoordAttribute = 1;

// Both attributes have 2 components
const U8 kAttributeSizes[] = { 2, 2 };
const int kNumAttributes = sizeof( kAttributeSizes ) / sizeof( * kAttributeSizes );

ArrayTupleStruct vertexArray(
	pAllocator, ArrayTupleStruct::kFloat, kAttributeSizes, kNumAttributes );

// 32x64 rectangle filled with image of same aspect ratio
float rectangle[] =
{
	// Position		// Texture
	0.f, 0.f,		0.f, 0.f,
	0.f, 64.f,		0.f, 1.f,
	32.f, 64.f,		1.f, 1.f,
	32.f, 0.f,		1.f, 0.f	
};
int numStructs = sizeof( rectangle ) / vertexArray.GetStructSize();

vertexArray.Append( 0, rectangle, numStructs );

Rtt_ASSERT( 2 == vertexArray.GetAttributeSize( kPositionAttribute ) );
Rtt_ASSERT( 2 == vertexArray.GetAttributeSize( kTextureCoordAttribute ) );

Rtt_ASSERT( 0 == vertexArray.GetAttributeOffset( kPositionAttribute ) );
Rtt_ASSERT( 2 == vertexArray.GetAttributeOffset( kTextureCoordAttribute ) );

``````

 */
class ArrayTupleStruct : public ArrayTuple
{
	public:
		typedef ArrayTuple Super;

	public:
		ArrayTupleStruct(
			Rtt_Allocator* pAllocator,
			Type t,
			const U8 *attributeSizes,
			int numAttributes );

	public:
		// TODO: rename.  Size suggests numBytes.  This is really num Type components
		// Returns number of fType primitives making up the attribute
		size_t GetAttributeSize( int attributeIndex ) const { return fAttributeSizes[attributeIndex]; }

		// Returns the number of fType primitives from the base of a struct element 
		// to where that attribute begins
		size_t GetAttributeOffset( int attributeIndex ) const { return fAttributeOffsets[attributeIndex]; }
		int GetNumAttributes() const;

	public:
		void Invalidate();

	private:
		Array< U8 > fAttributeSizes; // TODO: rename
		Array< U8 > fAttributeOffsets;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ArrayTupleStruct_H__
