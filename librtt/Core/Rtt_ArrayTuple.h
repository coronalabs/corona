//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ArrayTuple_H__
#define _Rtt_ArrayTuple_H__

#include "Core/Rtt_Array.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/**

# ArrayTuple

ArrayTuple is an array of tuples (e.g. position, color, etc.)

Each tuple is composed of one or more components of the same type, e.g. floats.
These types must be the same type as defined by the Type enum. 

 */
class ArrayTuple : public ArrayBuffer
{
	public:
		typedef ArrayBuffer Super;

	public:
		typedef enum _Type
		{
			kUnknown = -1,
			kUnsignedByte = 0,
			kFixed,
			kFloat,

			kNumTypes
		}
		Type;

		static size_t GetTypeSize( Type t );

	public:
		ArrayTuple( Rtt_Allocator* pAllocator, Type t );

	public:
		void Initialize( size_t numTypesPerTuple );

	public:
		// Number of bytes per tuple
		size_t GetTupleSize() const { return fTupleSize; }
		int GetNumComponentsPerTuple() const { return (int) fTupleSize / GetTypeSize(); }

		size_t GetTypeSize() const { return GetTypeSize( fType ); }
		Type GetType() const { return fType; }

	public:
		// Ensures internal allocation can handle 'minLength' tuples.
		// Returns true if allocation had to be grown; false otherwise.
		bool EnsureCapacity( int minLength );
		void SetLength( S32 newLength );

	public:
		// Returns false if buffer is not large enough; true otherwise
		void Set( int index, const void *src, int numSrcTuples );
		void Set( int index, const void *srcTuple );

	public:
		void Append( const void *src, int numSrcTuples );
		void Append( const void *srcTuple );

	public:
		void DuplicateLast();

	public:
		void Print( int numTabIndents ) const;

	protected:
		size_t fTupleSize;
		Type fType;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ArrayTuple_H__
