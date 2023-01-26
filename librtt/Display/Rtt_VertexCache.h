//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_VertexCache_H__
#define _Rtt_VertexCache_H__

#include "Display/Rtt_DisplayTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class VertexCache
{
	public:
		VertexCache(Rtt_Allocator* pAllocator);
        ~VertexCache();

	public:
		void Invalidate();
		bool IsValid() const { return fVertices.Length() > 0; }
    
        bool AddExtraFloatArray( const void * key );
        U32 ExtraFloatArrayCount() const;

        bool AddExtraIndexArray( const void * key );
        U32 ExtraIndexArrayCount() const;

	public:
		ArrayVertex2& Vertices() { return fVertices; }
		ArrayVertex2& TexVertices() { return fTexVertices; }
		Array<U32>& Colors() { return fColors; }
		ArrayS32& Counts() { return fCounts; }
    
        ArrayFloat * ExtraFloatArray( const void * key, bool addIfAbsent = false );
        ArrayFloat * FindFloatArray( const void * key ) const;
        const ArrayFloat * ExtraFloatArray( const void * key ) const;

        ArrayIndex * ExtraIndexArray( const void * key, bool addIfAbsent = false );
        ArrayIndex * FindIndexArray( const void * key ) const;
        const ArrayIndex * ExtraIndexArray( const void * key ) const;

		const ArrayVertex2& Vertices() const { return fVertices; }
		const ArrayVertex2& TexVertices() const { return fTexVertices; }
		const Array<U32>& Colors() const { return fColors; }
		const ArrayS32& Counts() const { return fCounts; }

	private:
		ArrayVertex2 fVertices;
		ArrayVertex2 fTexVertices;
		Array<U32> fColors;
		ArrayS32 fCounts;

        struct ArrayFloatBox {
            ArrayFloatBox( Rtt_Allocator * allocator );

            ArrayFloatBox * fNext;
            ArrayFloat fArray;
            void * fKey;
        };

        struct ArrayIndexBox {
            ArrayIndexBox( Rtt_Allocator * allocator );

            ArrayIndexBox * fNext;
            ArrayIndex fArray;
            void * fKey;
        };
        ArrayFloatBox * fExtraFloatArrays;
        ArrayIndexBox * fExtraIndexArrays;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VertexCache_H__
