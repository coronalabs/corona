//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_SpritePlayer.h"

#include "Core/Rtt_Time.h"
#include "Display/Rtt_SpriteObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

SpritePlayer::SpritePlayer( Rtt_Allocator *pAllocator )
:	fSprites( pAllocator ),
	fRemoveList( pAllocator ),
	fIsRunning( false ),
	fAnimationTime(0)
{
}

U64 SpritePlayer::GetAnimationTime() const
{
	return fAnimationTime;
}

void
SpritePlayer::AddSprite( SpriteObject *object )
{
	for ( int i = 0, iMax = fSprites.Length(); i < iMax; i++ )
	{
		const SpriteObject *item = fSprites[i];
		if ( item == object )
		{
			object = NULL;
			break;
		}
	}

	if ( object )
	{
		fSprites.Append( object );
	}
}

static int
GetInsertionIndex( Array< int >& array, int element, int start, int end )
{
	Rtt_ASSERT( start >= 0 );
	Rtt_ASSERT( start <= end );
	Rtt_ASSERT( end < array.Length() );

	int pivot = ( start + ((end-start) >> 1) ); // start + ( end - start ) / 2;

	if ( start == end )
	{
		return ( array[pivot] >= element ? pivot : pivot+1 );
	}
	else if ( array[pivot] >= element )
	{
		return GetInsertionIndex( array, element, start, pivot );
	}
	else
	{
		return GetInsertionIndex( array, element, pivot+1, end );
	}
}

void
SpritePlayer::RemoveSprite( SpriteObject *object )
{
	for ( int i = 0, iMax = fSprites.Length(); i < iMax; i++ )
	{
		const SpriteObject *item = fSprites[i];
		if ( item == object )
		{
			if ( fIsRunning )
			{
				object->SetMarked( true );

				// Always insert indices ('i'), small to large
				int index = 0;
				int removeListLen = fRemoveList.Length();
				if ( removeListLen > 0 )
				{
					index = GetInsertionIndex( fRemoveList, i, 0, removeListLen - 1 );
				}
				fRemoveList.Insert( index, i );
			}
			else
			{
				fSprites.Remove( i, 1, false );
			}
			break;
		}
	}
}

void
SpritePlayer::Run( lua_State *L, U64 millisecondTime )
{
	fAnimationTime = millisecondTime;
	fIsRunning = true;
	Update( L );
	fIsRunning = false;

	Collect();
}

void
SpritePlayer::Update( lua_State *L )
{
	for ( int i = 0, iMax = fSprites.Length(); i < iMax; i++ )
	{
		SpriteObject *sprite = fSprites[i];
		if ( ! sprite->IsMarked() )
		{
			sprite->Update( L, GetAnimationTime() );
		}
	}
}

void
SpritePlayer::Collect()
{
	Rtt_ASSERT( ! fIsRunning );

	// Remove backwards so that larger indices are removed first.
	// Otherwise larger indices into fSprites are invalidated by 
	// compacting fSprites at smaller indices.
	for ( int i = fRemoveList.Length(); --i >= 0; )
	{
		int index = fRemoveList[i];
		fSprites.Remove( index, 1, false );
		fRemoveList.Remove( i, 1 );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
