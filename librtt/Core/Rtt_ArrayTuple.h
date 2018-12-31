//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
