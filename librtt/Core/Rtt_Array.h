//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_Array_H
#define Rtt_Array_H

#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_Finalizer.h"
#include "Core/Rtt_New.h"
#include "Core/Rtt_Traits.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_Math.h"

#include <string.h>
#include <type_traits>

// ----------------------------------------------------------------------------

namespace Rtt
{

	// ----------------------------------------------------------------------------

	class ArrayStorage
	{
		Rtt_CLASS_NO_COPIES(ArrayStorage);

	protected:
		ArrayStorage(Rtt_Allocator* pAllocator);
		~ArrayStorage();

	public:
		Rtt_INLINE S32 Length() const { return fLength; }
		Rtt_INLINE Rtt_Allocator* Allocator() const { return fAllocator; }

	protected:
		size_t Copy(const ArrayStorage& rhs, size_t elementSize);

		//		void* ElementAt( S32 index, size_t elementSize );
		//		bool Realloc( S32 newLength, size_t elementSize );

		Rtt_FORCE_INLINE void* Storage() const { return fStorage; }
		Rtt_FORCE_INLINE S32 LengthMax() const { return fLengthMax; }

		void* Expand(size_t elementSize, S32 minLength = 0);

		// TODO: Implement Shrink()

		void ShiftMem(S32 index, S32 shiftAmount, size_t elementSize);

#ifdef Rtt_DEBUG
		Rtt_INLINE bool VerifyIndex(S32 index) const { return index >= 0 && index < Length(); }
#endif

		void Preallocate(U32 length, size_t elementSize);

	protected:
		void* fStorage;
		S32 fLength;
		S32 fLengthMax;
		Rtt_Allocator* fAllocator;
	};

	// ----------------------------------------------------------------------------
	class ArrayBuffer : public ArrayStorage
	{
		Rtt_CLASS_NO_COPIES(ArrayBuffer);

	public:
		typedef ArrayStorage Super;

	public:
		ArrayBuffer(Rtt_Allocator* pAllocator);

	public:
		void Empty() { fLength = 0; }

	public:
		const void* ReadAccess() const { return Storage(); }
		void* WriteAccess() { return Storage(); }

	protected:
		bool Grow(size_t elementSize, S32 newLength);
	};

	// ----------------------------------------------------------------------------

	template < typename T >
	class Array : public ArrayStorage
	{
		Rtt_CLASS_NO_COPIES(Array);

	public:
		typedef ArrayStorage Super;
		typedef Array< T > Self;

	public:
		Array(Rtt_Allocator* pAllocator) : Super(pAllocator) {}
		Rtt_INLINE ~Array() { Empty(); }

		// This class' copy constructor, copy operator, and Copy() method can leak memory
		// Uncomment on your own risk
//	protected:
//		void Copy( const Self& rhs );

	public:
		Rtt_INLINE void Append(const T& item) { Insert(-1, item); }
		Rtt_INLINE void Prepend(const T& item) { Insert(0, item); }

		void Insert(S32 index, const T& item);
		void Remove(S32 index, S32 numElements, bool finalize = true);
		void PadToSize(U32 size, const T& pad);

		Rtt_FORCE_INLINE const T* ReadAccess() const { return ElementPtr(); }
		Rtt_FORCE_INLINE T* WriteAccess() { return ElementPtr(); }

	public:
		// Rtt_FORCE_INLINE void Assign( S32 index, const T& item ) { * ElementAt( index ) = item; }
		virtual void Empty() { Remove(0, Length()); }
		void Clear() { Remove(0, Length(), false); } // Empty w/o calling d-tor

		Rtt_FORCE_INLINE const T& operator[](S32 index) const
		{
			Rtt_ASSERT(VerifyIndex(index));
			return *ElementAt(index);
		}
		Rtt_FORCE_INLINE T& operator[](S32 index)
		{
			Rtt_ASSERT(VerifyIndex(index));
			return *ElementAt(index);
		}

		void Reserve(U32 length)
		{
			Super::Preallocate(length, sizeof(T));
		}

	protected:
		static Rtt_FORCE_INLINE T* ElementPtr(void* p) { return reinterpret_cast<T*>(p); }
		static Rtt_FORCE_INLINE const T* ElementPtr(const void* p) { return reinterpret_cast<const T*>(p); }
		Rtt_FORCE_INLINE T* ElementPtr() const { return ElementPtr(Storage()); }
		Rtt_FORCE_INLINE T* ElementAt(S32 index) const
		{
			Rtt_ASSERT(index >= 0 && index < LengthMax());
			return ElementPtr(Storage()) + index;
		}

		void Copy(const void* src, S32 start, S32 end);

#ifdef Rtt_MINIMIZE_CODE_SIZE
		void Shift(S32 index, S32 shiftAmount);
#else
		void ShiftInsert(S32 index, S32 shiftAmount);
		void ShiftRemove(S32 index, S32 shiftAmount);
#endif
	};

	// ----------------------------------------------------------------------------

	// Use this if you have a set of ptr's that can be owned by the array
	template < typename T >
	class PtrArray : public Array< T* >
	{
		Rtt_CLASS_NO_COPIES(PtrArray)
			Rtt_STATIC_ASSERT(!Traits::IsPointer< T >::Value);

	public:
		typedef Array< T* > Super;

	public:
		PtrArray(Rtt_Allocator* pAllocator) : Super(pAllocator) {}

	public:
		T* Release(S32 index);
	};

	template < typename T >
	T*
		PtrArray< T >::Release(S32 index)
	{
		Rtt_ASSERT(Super::VerifyIndex(index));
		T* result = *Super::ElementAt(index);
		Super::Remove(index, 1, false);
		return result;
	}


	// ----------------------------------------------------------------------------

	// Use this if you have a set of ptr's that you do not own, but need to store a reference to
	template < typename T >
	class LightPtrArray : public PtrArray< T >
	{
		Rtt_CLASS_NO_COPIES(LightPtrArray)
			Rtt_STATIC_ASSERT(!Traits::IsPointer< T >::Value);

	public:
		typedef PtrArray< T > Super;

	public:
		LightPtrArray(Rtt_Allocator* pAllocator) : Super(pAllocator) {}
		~LightPtrArray()
		{
			// Disable removal by parent dtor's via setting array length to 0
			Super::fLength = 0;
		}

		virtual void Empty() { Super::Remove(0, Super::Length(), false); }
	};


	// Implementation
	// ----------------------------------------------------------------------------
	//
	//template < typename T >
	//Array< T >::Array( const Self& rhs )
	//:	Super( rhs.fAllocator )
	//{
	//	Rtt_ASSERT( this != & rhs );
	//	Copy( rhs );
	//}
	//
	//template < typename T >
	//const typename Array< T >::Self&
	//Array< T >::operator=( const Self& rhs )
	//{
	//	if ( this != & rhs )
	//	{
	//		Copy( rhs );
	//	}
	//
	//	return * this;
	//}
	//
	//template < typename T >
	//void
	//Array< T >::Copy( const Self& rhs )
	//{
	//	const size_t numBytes = Super::Copy( rhs, sizeof( T ) );
	//
	//	if ( numBytes > 0 )
	//	{
	//		T* dstStorage = ElementPtr();
	//
	//		if ( Traits::IsBitCopyable< T >::Value )
	//		{
	//			memcpy( dstStorage, rhs.fStorage, numBytes );
	//		}
	//		else
	//		{
	//			const S32 length = fLength;
	//			T* srcStorage = (T*)rhs.Storage();
	//			for ( S32 i = 0; i < length; i++ )
	//			{
	//				new( dstStorage++ ) T( srcStorage[i] );
	//			}
	//		}
	//	}
	//}

#ifdef Rtt_MINIMIZE_CODE_SIZE
#define ShiftInsert Shift
#define ShiftRemove Shift
#endif

	template < typename T >
	void
		Array< T >::Insert(S32 index, const T& item)
	{
		if (index < 0) { index = Length(); }

		Rtt_ASSERT(index >= 0 && index <= Length());

		if (Length() == LengthMax())
		{
			void* oldStorage = Expand(sizeof(T));
			if (oldStorage)
			{
				Copy(oldStorage, 0, Length());
				Rtt_FREE(oldStorage);
			}
		}

		// TODO: Handle case where we are inserting an item already
		// in the array that will end up being shifted and thus
		// have its d-tor called before we successfully insert it!
		// For now, assert:
		Rtt_ASSERT(0 == Length() ||
			(&item < ElementAt(index) || &item > ElementAt(Length() - 1)));

		if (index < Length())
		{
			ShiftInsert(index, 1);
		}

		Rtt_ASSERT(Length() < LengthMax());

		new(ElementAt(index)) T(item);
		fLength++;
	}

	template < typename T >
	void
		Array< T >::Remove(S32 index, S32 numElements, bool finalize)
	{
		if (Length() > 0 && Rtt_VERIFY(numElements > 0))
		{
			Rtt_ASSERT(VerifyIndex(index));

			numElements = Min(numElements, Length() - index);

			Rtt_ASSERT(VerifyIndex(index + numElements - 1));

			if (finalize)
			{
				// Ensure dtor is properly called for items being removed
				T* iElements = ElementPtr() + index;
				for (S32 i = numElements; i > 0; i--, iElements++)
				{
					ObjectFinalizer< T >::Collect(*iElements);
				}
			}

			// Base of remaining elements (to the "right" of the removed elements)
			S32 baseIndex = index + numElements;
			if (baseIndex < Length())
			{
				ShiftRemove(baseIndex, -numElements);
			}

			fLength -= numElements;
		}
	}

	template < typename T >
	void
		Array< T >::PadToSize(U32 size, const T& pad)
	{
		if (size > fLength)
		{
			if (!fLength)
			{
				Reserve(size);
			}

			else
			{
				void* oldStorage = Expand(sizeof(T), size);
			
				Rtt_ASSERT( oldStorage );
			
				Copy(oldStorage, 0, fLength);
					
				Rtt_FREE(oldStorage);
			}
			
			U32 oldLength = (U32)fLength;
			
			if (std::is_pod<T>::value)
			{
				fLength = (S32)size;

				for (U32 i = oldLength; i < size; i++)
				{
					WriteAccess()[i] = pad;
				}
			}

			else
			{
				for (U32 i = oldLength; i < size; i++)
				{
					Append(pad);
				}
			}
		}
	}

	/**
	 * Copy from start to (end-1). So in an array of 10 characters, to copy the whole thing you'd
	 *	specify start=0, end=10.
	 *
	 * @param src		Source of the copy, possibly another Array.
	 * @param start		First index
	 * @param end		Index past the actual end.
	 */
	template < typename T >
	void
		Array< T >::Copy(const void* src, S32 start, S32 end)
	{
		Rtt_ASSERT(end <= Length());

		// Hopefully, the optimizer will optimize the if-else away
		if (Traits::IsBitCopyable< T >::Value)
		{
			memcpy(fStorage, ElementPtr(src) + start, sizeof(T) * (end - start));
		}
		else
		{
			for (S32 i = start; i < end; i++)
			{
				const T* pItem = ElementPtr(src) + i; Rtt_ASSERT(pItem);
				new(ElementAt(i)) T(*pItem);
				pItem->~T();
			}
		}
	}

#ifdef Rtt_MINIMIZE_CODE_SIZE

#undef ShiftInsert
#undef ShiftRemove

	template < typename T >

	void
		Array< T >::Shift(S32 index, S32 shiftAmount)
	{
		Rtt_ASSERT(VerifyIndex(index));
		Rtt_ASSERT((index + shiftAmount) >= 0);
		Rtt_ASSERT((index + shiftAmount) < LengthMax());
		Rtt_ASSERT(shiftAmount != 0);

		// Hopefully, the optimizer will optimize the if-else away
		if (Traits::IsBitCopyable< T >::Value)
		{
			ShiftMem(index, shiftAmount, sizeof(T));
			// memmove( ElementAt(index + shiftAmount)), ElementAt(index), sizeof( T ) * ( Length() - index ) );
		}
		else
		{
			const bool shiftInsert = (shiftAmount > 0);
			T* srcItem = ElementAt(shiftInsert ? Length() - 1 : index);
			const S32 direction = (shiftInsert ? -1 : 1);
			T* dstItem = srcItem + shiftAmount;

			for (S32 i = Length() - index; i > 0; i--)
			{
				new(dstItem) T(*srcItem);
				srcItem->~T();

				srcItem += direction;
				dstItem += direction;
			}
		}
	}

#else

	template < typename T >
	void
		Array< T >::ShiftInsert(S32 index, S32 shiftAmount)
	{
		Rtt_ASSERT(VerifyIndex(index));
		Rtt_ASSERT((Length() + shiftAmount) < LengthMax());
		Rtt_ASSERT(shiftAmount > 0);

		// Hopefully, the optimizer will optimize the if-else away
		if (Traits::IsBitCopyable< T >::Value)
		{
			ShiftMem(index, shiftAmount, sizeof(T));
			// memmove( ElementAt(index+shiftAmount)), ElementAt(index), sizeof( T ) * ( Length() - index ) );
		}
		else
		{
			// const S32 len = Length();

			for (S32 i = Length() - 1, j = i + shiftAmount;
				i >= index;
				i--, j--)
			{
				T* srcItem = ElementAt(i);
				T* dstItem = ElementAt(j);
				new(dstItem) T(*srcItem);
				srcItem->~T();
			}
		}
	}

	template < typename T >
	void
		Array< T >::ShiftRemove(S32 index, S32 shiftAmount)
	{
		Rtt_ASSERT(VerifyIndex(index));
		Rtt_ASSERT((index + shiftAmount) >= 0);
		Rtt_ASSERT(shiftAmount < 0);

		// Hopefully, the optimizer will optimize the if-else away
		if (Traits::IsBitCopyable< T >::Value)
		{
			ShiftMem(index, shiftAmount, sizeof(T));
			// memmove( ElementAt(index + shiftAmount)), ElementAt(index), sizeof( T ) * ( Length() - index ) );
		}
		else
		{
			for (S32 i = index, iMax = Length(), j = i + shiftAmount;
				i < iMax;
				i++, j++)
			{
				T* srcItem = ElementAt(i);
				T* dstItem = ElementAt(j);
				new(dstItem) T(*srcItem);
				srcItem->~T();
			}

		}
	}

#endif // Rtt_MINIMIZE_CODE_SIZE

	// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_Array_H

