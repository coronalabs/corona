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

#ifndef _Rtt_List_H__
#define _Rtt_List_H__

#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

template < typename T >
class ListSingleIterator;

// ----------------------------------------------------------------------------

template < typename T >
class NodeSingle
{
	public:
		typedef NodeSingle< T > Self;

	public:
		NodeSingle() : fItem(), fNext( NULL ) {}

		NodeSingle( const T& item, Self* pNext )
		:	fItem( item ),
			fNext( pNext )
		{
		}

		Rtt_FORCE_INLINE Self* Next() { return fNext; }
		Rtt_FORCE_INLINE void SetNext( Self* pNode ) { fNext = pNode; }

		void InsertAfter( Self* pNode )
		{
			Rtt_ASSERT( NULL != pNode );
			Rtt_ASSERT( NULL == pNode->Next() );
			pNode->SetNext( fNext );
			SetNext( pNode );
		}
		
		void RemoveAfter()
		{
			Self* pCurNext = fNext;
			SetNext( pCurNext ? pCurNext->fNext : NULL );
		}
		
		Rtt_FORCE_INLINE const T& Item() const { return fItem; }
		Rtt_FORCE_INLINE T& Item() { return fItem; }

	private:
		T fItem;
		Self* fNext;
};

template < typename T >
class ListSingle
{
	public:
		typedef ListSingle< T > Self;
		typedef ListSingleIterator< T > Iterator;
		typedef NodeSingle< T > Node;

	public:
		ListSingle( Rtt_ALLOCATOR_VAR(p) )
		:	fDummy(),
			fLast( NULL )
#ifdef Rtt_ALLOCATOR_ENABLE
			, fAllocator( p )
#endif
		{
		}

		~ListSingle()
		{
			Rtt_TRACE( ("~ListSingle(%p)\n", this) );
			Rtt_TRACE( ("fDummy(%p)\n", & fDummy) );
			for ( Node* iNode = Head(); iNode; )
			{
				Node* p = iNode;
				iNode = iNode->Next();

				Rtt_TRACE( ("\tdtor node(%p) with next(%p)...", p, iNode ) );
				Rtt_DELETE( p );
				Rtt_TRACE( ("done\n" ) );
			}
		}

		void Append( const T& item )
		{
			Node* pNode = Rtt_NEW( fAllocator, Node( item, NULL ) );
			Rtt_ASSERT( pNode );

			( fLast ? fLast : & fDummy )->InsertAfter( pNode );
			fLast = pNode;
		}

		void Prepend( const T& item )
		{
			Node* pCurHead = Head();
			Node* pNewHead = Rtt_NEW( fAllocator, Node( item, pCurHead ) );
			
			Rtt_TRACE( ("\tnewNode(%p) with next(%p)\n", pNewHead, pCurHead ) );
			Rtt_ASSERT( pNewHead );

			fDummy.SetNext( pNewHead );
		}

		Rtt_FORCE_INLINE Iterator Begin() { return Iterator( * this, Head() ); }
		Rtt_FORCE_INLINE Iterator End() { return Iterator( * this, NULL ); }

	protected:
		Rtt_FORCE_INLINE Node* Head() { return fDummy.Next(); }

	private:
		Node fDummy;
		Node* fLast;

		Rtt_ALLOCATOR_VAR( fAllocator );
};

template < typename T >
Rtt_FORCE_INLINE bool operator==( const ListSingleIterator< T >& lhs, const ListSingleIterator< T >& rhs )
{
	return lhs.fIndex == rhs.fIndex;
}

template < typename T >
Rtt_FORCE_INLINE bool operator!=( const ListSingleIterator< T >& lhs, const ListSingleIterator< T >& rhs )
{
	return ! operator==( lhs, rhs );
}

template < typename T >
class ListSingleIterator
{
	public:
		typedef ListSingleIterator< T > Self;
		typedef ListSingle< T > Owner;
		typedef NodeSingle< T > Node;

	private:
		ListSingleIterator( Owner& owner, Node* pIndex ) : fOwner( owner ), fIndex( pIndex ) {}

	public:
		Rtt_FORCE_INLINE void Next()
		{
			Rtt_ASSERT( fIndex );
			fIndex = fIndex->Next();
		}

		Rtt_FORCE_INLINE operator T&()
		{
			Rtt_ASSERT( fIndex );
			return fIndex->Item();
		}
		
		Rtt_FORCE_INLINE operator const T&() const
		{
			Rtt_ASSERT( fIndex );
			return fIndex->Item();
		}

		Rtt_FORCE_INLINE const T& operator->() const { return *this; }
		Rtt_FORCE_INLINE T& operator->() { return *this; }

		Rtt_FORCE_INLINE bool NotAtEnd() const { return NULL != fIndex; }

	private:
		Owner& fOwner;
		Node* fIndex;

		friend class ListSingle< T >;
		friend bool operator==< >( const ListSingleIterator< T >& lhs, const ListSingleIterator< T >& rhs );
};

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_List_H__
