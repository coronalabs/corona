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

#include "Core/Rtt_Build.h"

#include "Display/Rtt_SpriteObject.h"

#include "Core/Rtt_Time.h"
#include "Display/Rtt_RectPath.h"
#include "Display/Rtt_ImageFrame.h"
#include "Display/Rtt_ImageSheet.h"
#include "Display/Rtt_ImageSheetPaint.h"
#include "Display/Rtt_ImageSheetUserdata.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaAux.h"
#include "Rtt_LuaProxyVTable.h"
#include "Display/Rtt_SpritePlayer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

SpriteObjectSequence::Direction
SpriteObjectSequence::DirectionForString( const char *value )
{
	Direction result = kForward;

	if ( Rtt_StringCompareNoCase( value, "bounce" ) == 0 )
	{
		result = kBounce;
	}

	return result;
}

SpriteObjectSequence*
SpriteObjectSequence::Create( Rtt_Allocator *allocator, lua_State *L, int index )
{
	const char kEmptyStr[] = "";

	SpriteObjectSequence *result = NULL;

	// Canonicalize the index to positive value
	if ( index < 0 )
	{
		index += lua_gettop( L ) + 1;
	}

	lua_getfield( L, index, "name" );
	const char *name = lua_tostring( L, -1 );
	if ( ! name )
	{
		name = kEmptyStr;
	}
	lua_pop( L, 1 );

	lua_getfield( L, index, "start" );
	int start = (int) lua_tointeger( L, -1 );
	lua_pop( L, 1 );

	lua_getfield( L, index, "time" );
	Real time = luaL_toreal( L, -1 );
	lua_pop( L, 1 );

	lua_getfield( L, index, "loopCount" );
	int loopCount = (int) lua_tointeger( L, -1 );
	if ( loopCount < 0 )
	{
		Rtt_TRACE_SIM( ( "WARNING: The 'loopCount' value(%d) cannot be negative. We'll be taking the minus sign away. You should set 'loopDirection' to 'bounce' if you want that looping behavior.\n", loopCount ) );
		loopCount = Abs( loopCount );
	}
	lua_pop( L, 1 );

	lua_getfield( L, index, "loopDirection" );
	Direction loopDirection = DirectionForString( lua_tostring( L, -1 ) );
	lua_pop( L, 1 );

	if ( start > 0 )
	{
		lua_getfield( L, index, "count" );
		int numFrames = (int) lua_tointeger( L, -1 );
		lua_pop( L, 1 );

		if ( numFrames <= 0 )
		{
			Rtt_TRACE_SIM( ( "WARNING: Invalid 'count' value(%d) in sequenceData. Assuming the frame count of the sequence is 1.\n", numFrames ) );
			numFrames = 1;
		}

		--start; // Lua value was 1-based

		// Consecutive frames
		result = Rtt_NEW( allocator, SpriteObjectSequence(
			allocator, name, time, start, numFrames, loopCount, loopDirection ) );
	}
	else
	{
		lua_getfield( L, index, "frames" );
		if ( lua_istable( L, -1 ) )
		{
			int numFrames = (int) lua_objlen( L, -1 );

			FrameIndex *frames = (FrameIndex *)Rtt_MALLOC( allocator, numFrames * sizeof( FrameIndex ) );
			for ( int i = 0; i < numFrames; i++ )
			{
				lua_rawgeti( L, -1, i+1 ); // Lua is 1-based
				FrameIndex value = lua_tointeger( L, -1 );
				if ( value < 1 )
				{
					Rtt_TRACE_SIM( ( "WARNING: Invalid value(%d) in 'frames' array.\n", value ) );
				}
				frames[i] = (value - 1); // Lua is 1-based
				lua_pop( L, 1 );
			}

			// Non-consecutive
			result = Rtt_NEW( allocator, SpriteObjectSequence(
				allocator, name, time, frames, numFrames, loopCount, loopDirection ) );
		}
		else
		{
			Rtt_TRACE_SIM( ( "ERROR: sequenceData missing data. One of the following must be supplied: a pair of properties 'start'/'count' or an array value for the property 'frames'.\n" ) );
		}
		lua_pop( L, 1 );
	}


	if ( result )
	{
		ImageSheetUserdata *ud = NULL;

		lua_getfield( L, index, "sheet" );
		if ( lua_isuserdata( L, -1 ) )
		{
			ud = ImageSheet::ToUserdata( L, -1 );
		}
		lua_pop( L, 1 );

		if ( ud )
		{
			// TODO: Optimize memory. 
			// Don't set sheet if it's the same as the sprite's default sheet
			const AutoPtr< ImageSheet >& sheet = ud->GetSheet();
			result->Initialize( allocator, sheet );
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

SpriteObjectSequence::SpriteObjectSequence(
	Rtt_Allocator *allocator,
	const char *name,
	Real time,
	FrameIndex start,
	FrameIndex numFrames,
	int loopCount,
	Direction loopDirection )
:	fPaint( NULL ),
	fSheet(),
	fName( allocator, name ),
	fTime( time ),
	fTimePerFrame( Rtt_RealDiv( Rtt_IntToReal( time ), Rtt_IntToReal( numFrames ) ) ),
	fNumFrames( numFrames ),
	fStart( start ),
	fFrames( NULL ),
	fLoopCount( loopCount ),
	fLoopDirection( loopDirection )
{
	Rtt_ASSERT( loopCount >= 0 );
}

// Assumes ownership of 'frames' and assumes alloc'd via Rtt_MALLOC
SpriteObjectSequence::SpriteObjectSequence(
	Rtt_Allocator *allocator,
	const char *name,
	Real time,
	FrameIndex *frames,
	FrameIndex numFrames,
	int loopCount,
	Direction loopDirection )
:	fPaint( NULL ),
	fSheet(),
	fName( allocator, name ),
	fTime( time ),
	fTimePerFrame( Rtt_RealDiv( Rtt_IntToReal( time ), Rtt_IntToReal( numFrames ) ) ),
	fStart( -1 ),
	fFrames( frames ),
	fNumFrames( numFrames ),
	fLoopCount( loopCount ),
	fLoopDirection( loopDirection )
{
	Rtt_ASSERT( loopCount >= 0 );
}

SpriteObjectSequence::~SpriteObjectSequence()
{
	Rtt_FREE( fFrames );
	Rtt_DELETE( fPaint );
}

void
SpriteObjectSequence::Initialize( Rtt_Allocator *pAllocator, const AutoPtr< ImageSheet >& sheet )
{
	if ( Rtt_VERIFY( fSheet.IsNull() && NULL == fPaint ) && sheet.NotNull() )
	{
		fPaint = ImageSheetPaint::NewBitmap( pAllocator, sheet );
		fSheet = sheet;
	}
}

void
SpriteObjectSequence::Verify( const SpriteObject& owner ) const
{
	const AutoPtr< ImageSheet >& sheet = fSheet.NotNull() ? fSheet : owner.GetDefaultSheet();

	Rtt_ASSERT( sheet.NotNull() );

	int numFrames = sheet->GetNumFrames();

	if ( IsConsecutiveFrames() )
	{
		int start = GetStartFrame() + 1;
		int last = GetLastFrame() + 1;

		if ( start <= 0 || start > numFrames )
		{
			Rtt_TRACE_SIM( ( "WARNING: Sequence (%s) has an invalid 'start' index (%d) that falls outside the range of valid image sheet frame indices: 1 <= index <= %d.\n", GetName(), start, numFrames ) );
		}

		if ( last <= 0 || last > numFrames )
		{
			Rtt_TRACE_SIM( ( "WARNING: Sequence (%s) has an invalid count (%d) that makes the last frame index of the sequence (%d) fall outside the range of valid image sheet frame indices: 1 <= index <= %d.\n", GetName(), last - start + 1, last, numFrames ) );
		}
	}
	else
	{
		// Iterate through sequence frames and verify the image sheet index for 
		// each sequence frame is valid.
		for ( int i = 0; i < fNumFrames; i++ )
		{
			// GetFrame() returns the index in the imagesheet for the sequence frame 'i'
			int index = GetFrame( i ) + 1;

			if ( index <= 0 || index > numFrames )
			{
				Rtt_TRACE_SIM( ( "WARNING: Sequence (%s) has an invalid index (%d) that falls outside the range of valid image sheet frame indices: 1 <= index <= %d.\n", GetName(), index, numFrames ) );
			}
		}
	}
}

Real
SpriteObjectSequence::GetEffectiveTime() const
{
	Real numFrames = Rtt_IntToReal( GetEffectiveNumFrames() );
	return Rtt_RealMul( numFrames, GetTimePerFrame() );
}

SpriteObjectSequence::FrameIndex
SpriteObjectSequence::GetStartFrame() const
{
	return ( IsConsecutiveFrames() ? fStart : fFrames[0] );
}

SpriteObjectSequence::FrameIndex
SpriteObjectSequence::GetLastFrame() const
{
	return ( IsConsecutiveFrames() ? ( fStart + GetNumFrames() - 1 ) : fFrames[0] );
}

SpriteObjectSequence::FrameIndex
SpriteObjectSequence::GetFrame( int index ) const
{
	return ( IsConsecutiveFrames() ? (fStart+index) : fFrames[index] );
}

SpriteObjectSequence::FrameIndex
SpriteObjectSequence::GetEffectiveFrame( int index, SpriteEvent::Phase *phase ) const
{
	FrameIndex result = 0;

	// Get effective frame count
	int numFrames = GetEffectiveNumFrames();

	// Something really bad happened...
	Rtt_ASSERT( index >= 0 );
	Rtt_ASSERT( index < numFrames );

	// ...but just in case, ensure index is within proper boundaries
	index = Max( index, 0 );
	index = Min( index, (int)(numFrames - 1) ); // clamp max index

	if ( kForward == fLoopDirection )
	{
		if ( index >= fNumFrames )
		{
			// Map index into [0, fNumFrames)
			index = index % fNumFrames;
		}

		if ( phase )
		{
			// Detect loop
			if ( 0 == index )
			{
				* phase = SpriteEvent::kLoop;
			}
		}

		result = GetFrame( index );
	}
	else if ( kBounce == fLoopDirection )
	{
		int i = fLoopCount;
		int n = fNumFrames;

		if ( i > 0 )
		{
			if ( n > 1 )
			{
				// Map index into [0, nSequence)
				// where nSequence = 2*(n-1)
				// 
				// Example (n=3)
				//   Sequence:      123212321
				//   Initial index: 012345678
				//   Final index:   012301230
				// 
				// Note: we don't have to worry about index exceeding 
				// numFrames because we clamp it above
				int nSequence = 2*(n-1);
				index = index % nSequence;
			}
			else
			{
				index = 0; // Only 1 frame, so always the first one
			}
		}
		else
		{
			Rtt_ASSERT( 0 == i );

			if ( 1 == n )
			{
				index = 0; // Only 1 frame, so always the first one
			}
			else
			{
				// Map index into [0,numFrames)
				// 
				// For example, for n=5, the sequence looks like:
				//       (12345432 12345432 12345432 ...)
				// so the core sequence is [12345432] which has 8 elements.
				// Therefore the index must lie in the range [0,8)
				index = ( index % numFrames );
			}
		}

		// Map index into [0, fNumFrames)
		if ( index < fNumFrames )
		{
			if ( phase )
			{
				// Detect bounce
				if ( (fNumFrames - 1) == index )
				{
					* phase = SpriteEvent::kBounce;
				}
				else if ( 0 == index )
				{
					* phase = SpriteEvent::kLoop;
				}
			}

			// Forward
			result = GetFrame( index );
		}
		else
		{
			// Backward
			// 
			// Map the index into [0, n-1) using the following formula
			// 
			// indexMapped = 2*(n-1) - index
			// 
			// For example, for n=5, the sequence is
			//                    [123454321]
			// index values:       012345678 
			// final index value:       3210
			// 
			// The index values 5,6,7,8 need to be mapped to [0,3]
			// so we first calculate 'j', the index relative to the middle index (4)
			index = 2*(fNumFrames - 1) - index;
			result = GetFrame( index );
		}
	}

	return result;
}

// n is the raw number of frames in the sequence (fNumFrames)
// i is the number of iterations (fLoopCount)
//
// For (i > 0), the number of effective frames depends on the
// loop count.
// for kBounce
// 
//  \ i (num iterations)
// n \   1            2             3
// -------------------------------------------------------
// 1  |  1 (1)        2 (11)        3 (111)
// 2  |  3 (121)      5 (12121)     7 (1212121)
// 3  |  5 (12321)    9 (123212321) 13 (1232123212321)
// .  |
// n  |  For (n > 1; i > 0), numFrames(n,i) = 2*i*(n-1) + 1
//    |  For (n = 1; i > 0), numFrames(1,i) = i
// 
// For (i == 0), the loop occurs over the effective sequences [in brackets]
// which forms the basis for the infinite loop (in parens). 
// Here, we show the forward case for completeness
//   
// n  |  kForward                  kBounce
// -------------------------------------------------------
// 1  |  [1] (1 1 ...)             [1] (1 1 ...)        
// 2  |  [12] (12 12 ...)          [12] (12 12 ...)
// 3  |  [123] (123 123 ...)       [1232] (1232 1232 ...)
// 4  |  [1234] (1234 1234 ...)    [123432] (123432 123432 ...)
// .  |
// n  |  numFrames = n             numFrames = 2*(n-1) except for n=1
int
SpriteObjectSequence::GetEffectiveNumFrames() const
{
	int result = fNumFrames; // Default to something reasonable

	int i = fLoopCount;
	int n = fNumFrames;

	if ( kForward == fLoopDirection )
	{
		if ( i > 0 )
		{
			result = i*n;
		}
		else
		{
			Rtt_ASSERT( 0 == i );
			result = fNumFrames;
		}
	}
	else if ( kBounce == fLoopDirection )
	{
		if ( i > 0 )
		{
			if ( n > 1 )
			{
				result = 2*i*(n - 1) + 1;
			}
			else
			{
				Rtt_ASSERT( 1 == n );
				result = i;
			}
		}
		else
		{
			Rtt_ASSERT( 0 == i );
			if ( 1 == n )
			{
				result = 1;
			}
			else
			{
				result = 2*(n - 1);
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

SpriteObject*
SpriteObject::Create(
	Rtt_Allocator *pAllocator,
	const AutoPtr< ImageSheet >& sheet,
	SpritePlayer& player )
{
	SpriteObject *result = NULL;

	if ( ! sheet->IsEmpty() )
	{
		const ImageFrame *frame = sheet->GetFrame( 0 );

		if ( Rtt_VERIFY( frame ) )
		{
			Real width = frame->GetWidth();
			Real height = frame->GetHeight();

			RectPath *path = RectPath::NewRect( pAllocator, width, height );

			result = Rtt_NEW( pAllocator, SpriteObject( path, pAllocator, sheet, player ) );
		}
		else
		{
			Rtt_TRACE_SIM( ( "ERROR: Could not create sprite. The imageSheet is missing a first frame.\n" ) );
		}
	}
	else
	{
		Rtt_TRACE_SIM( ( "ERROR: Could not create sprite. The imageSheet has no frames defined.\n" ) );
	}

	return result;
}

SpriteObject::SpriteObject(
	RectPath *path,
	Rtt_Allocator *pAllocator,
	const AutoPtr< ImageSheet >& sheet,
	SpritePlayer& player )
:	Super( path ),
	fPaint( NULL ),
	fSheet( sheet ),
	fSequences( pAllocator ),
	fPlayer( player ),
	fTimeScale( Rtt_REAL_1 ),
	fCurrentSequence( 0 ), // Default is first sequence
	fSequenceIndex( 0 ),
	fStartTime( 0 ),
	fPlayTime( 0 ),
	fProperties( 0 )
{
    SetObjectDesc( "SpriteObject" );     // for introspection
}

SpriteObject::~SpriteObject()
{
	fPlayer.RemoveSprite( this );

	Rtt_DELETE( fPaint );
}

void
SpriteObject::Initialize( Rtt_Allocator *pAllocator )
{
	Rtt_ASSERT( GetStage() );

	ClosedPath& path = GetPath();

	// Ensure this instance owns the paint, and not the path
	path.SetFillWeakReference( true );

	ImageSheetPaint *paint = ImageSheetPaint::NewBitmap( pAllocator, fSheet );
	SetFill( paint );
	// path.SetFill( paint );

	fPaint = paint;

	fPlayer.AddSprite( this );
}

void
SpriteObject::AddSequence( SpriteObjectSequence *sequence )
{
	if ( Rtt_VERIFY( sequence ) )
	{
		// Detect multisprite. If it's already a multisprite, then skip the check.
		if ( ! IsProperty( kIsMultiSprite ) )
		{
			bool isMultiSprite =
				( sequence->HasSheet()
				&& sequence->GetSheet() != GetDefaultSheet() );
			SetProperty( kIsMultiSprite, isMultiSprite );
		}

		sequence->Verify( * this );

		// If no sequences have been added
		if ( 0 == fSequences.Length() )
		{
			SetBitmapFrame( sequence->GetEffectiveFrame( 0 ) );
		}
		fSequences.Append( sequence );
	}
}

void
SpriteObject::Prepare( const Display& display )
{
#ifdef OLD_GRAPHICS
	const Matrix& xform = GetSrcToDstMatrix();
	GetPath().Build( xform, AlphaCumulative() );
#endif

	Super::Prepare( display );
}

/*
// Translate is the only operation where we can update all data _without_
// a rebuild. All other ops will trigger a rebuild
void
SpriteObject::Translate( Real dx, Real dy )
{
//	Rtt_ASSERT_NOT_IMPLEMENTED();
	Super::Translate( dx, dy );
}
*/

const LuaProxyVTable&
SpriteObject::ProxyVTable() const
{
	return LuaSpriteObjectProxyVTable::Constant();
}

const AutoPtr< ImageSheet >&
SpriteObject::GetDefaultSheet() const
{
	return fSheet;
}

const AutoPtr< ImageSheet >&
SpriteObject::GetCurrentSheet() const
{
	SpriteObjectSequence *sequence = GetCurrentSequence();

	if ( sequence && sequence->HasSheet() )
	{
		return sequence->GetSheet();
	}

	return GetDefaultSheet();
}

void
SpriteObject::SetBitmapFrame( int frameIndex )
{
	Paint *paint = Super::GetPath().GetFill();
	Rtt_ASSERT( paint->IsCompatibleType( Paint::kBitmap ) );

	ImageSheetPaint *bitmapPaint = (ImageSheetPaint *)paint->AsPaint(Paint::kImageSheet);

	// Ensure 0 <= frameIndex < sheet->GetNumFrames()
	const AutoPtr< ImageSheet >& sheet = bitmapPaint->GetSheet();
	int maxFrameIndex = sheet->GetNumFrames() - 1;
	frameIndex = Min( frameIndex, maxFrameIndex );
	frameIndex = Max( frameIndex, 0 );

	bitmapPaint->SetFrame( frameIndex );
	const ImageFrame *frame = sheet->GetFrame( frameIndex );

	SetSelfBounds( frame->GetWidth(), frame->GetHeight() );

	// We need to invalidate the srcToDst transform if either:
	// (1) The new frame is trimmed, or
	// (2) The new frame is not trimmed AND the previous one was
	bool isTrimmed = frame->IsTrimmed();
	if ( isTrimmed || IsProperty( kIsPreviousFrameTrimmed ) )
	{
		Invalidate( kTransformFlag );
	}

	// Store whether or not the new frame is trimmed or not
	SetProperty( kIsPreviousFrameTrimmed, isTrimmed );

	// Update texture coords for new frame
	Invalidate( kGeometryFlag );
	GetPath().Invalidate( ClosedPath::kFillSourceTexture );
}

void
SpriteObject::Update( lua_State *L, U64 milliseconds )
{
	if( ! IsPlaying() )
	{
		// Nothing to do.
		return;
	}

	if( milliseconds < fStartTime )
	{
		// Nothing to do.
		return;
	}

	bool shouldDispatch = false;
	SpriteEvent::Phase phase = SpriteEvent::kNumPhases; // unknown

	// By default, assume frame does not advance
	int i = -1;

	SpriteObjectSequence *sequence = GetCurrentSequence();

	int numFrames = sequence->GetEffectiveNumFrames();

	if ( ! IsProperty( kIsPlayingBegan ) )
	{
		// Base case: first frame
		i = fSequenceIndex;

		SetProperty( kIsPlayingBegan, true );

		if ( HasListener( kSpriteListener ) )
		{
			shouldDispatch = true;
			phase = SpriteEvent::kBegan;
		}
	}
	else
	{
		// Inductive step: advance frame
		if ( sequence->GetTime() > 0 )
		{
			// time-based sequence.

			Real dt = Rtt_IntToReal( (U32) (milliseconds - fStartTime) );

			if ( ! Rtt_RealIsOne( fTimeScale ) )
			{
				dt = Rtt_RealMul( dt, fTimeScale );
			}

			// time-based, so frame changes depend on current time
			Real timePerFrame = sequence->GetTimePerFrame();
			Real sequenceIndex = Rtt_RealDiv( dt, timePerFrame );
			int index = (int)sequenceIndex; // truncate fractional part to get index

			bool isInfiniteLooping = ( 0 == sequence->GetLoopCount() );
			if ( isInfiniteLooping )
			{
				bool isFirstLoopDone = ( index >= numFrames );

				// For infinite looping, handle wrap-around
				index = index % numFrames;

				if ( isFirstLoopDone && index < fSequenceIndex )
				{
					// It's unclear what case this is catching and it impedes debugging
					// so commenting out until the reason for it is remembered
					// Rtt_ASSERT( 0 == index );

					if ( HasListener( kSpriteListener ) )
					{
						shouldDispatch = true;
						phase = SpriteEvent::kLoop;
					}

					fSequenceIndex = 0;
					i = 0;
				}
			}

			if ( index > fSequenceIndex )
			{
				// Only dispatch if we advance.
				// Assume the phase is 'next' for now, and set to other phases below.
				if ( HasListener( kSpriteListener ) )
				{
					shouldDispatch = true;
					phase = SpriteEvent::kNext;
				}

				if ( ! isInfiniteLooping )
				{
					// Handle end condition when we are at the last frame
					int lastIndex = ( numFrames - 1 );

					if ( index >= lastIndex )
					{
						index = lastIndex;

						// We're about to advance beyond the last frame
						if ( ! IsProperty( kIsPlayingEnded ) )
						{
							SetPlaying( false );
							SetProperty( kIsPlayingEnded, true );

							if ( HasListener( kSpriteListener ) )
							{
								shouldDispatch = true;
								phase = SpriteEvent::kEnded;
							}
						}
					}
				}

				fSequenceIndex = index;
				i = index;
			}
		}
		else
		{
			// frame-based sequence, so every update is an advance
			i = ++fSequenceIndex;

			// Assume the phase is 'next' for now, and set to other phases below.
			if ( HasListener( kSpriteListener ) )
			{
				shouldDispatch = true;
				phase = SpriteEvent::kNext;
			}

			if ( 0 == sequence->GetLoopCount() )
			{
				// For infinite looping, handle wrap-around (no last frame)
				if ( i >= numFrames )
				{
					i = 0;
					fSequenceIndex = i;

					if ( HasListener( kSpriteListener ) )
					{
						shouldDispatch = true;
						phase = SpriteEvent::kLoop;
					}
				}
			}
			else
			{
				// Handle end condition when we are at the last frame
				int lastIndex = ( numFrames - 1 );

				if ( i >= lastIndex )
				{
					i = lastIndex;
					fSequenceIndex = i;

					// We're about to advance beyond the last frame
					if ( ! IsProperty( kIsPlayingEnded ) )
					{
						SetPlaying( false );
						SetProperty( kIsPlayingEnded, true );

						if ( HasListener( kSpriteListener ) )
						{
							shouldDispatch = true;
							phase = SpriteEvent::kEnded;
						}
					}
				}
			}
		}
	}

	// Advance frame if valid index is set
	if ( i >= 0 )
	{
		if ( i < numFrames )
		{
			SpriteEvent::Phase p = SpriteEvent::kNext;
			int frameIndex = sequence->GetEffectiveFrame( i, & p );
			SetBitmapFrame( frameIndex );

			if ( shouldDispatch
				 && SpriteEvent::kNext != p			// Did 'p' change from default?
				 && SpriteEvent::kNext == phase )	// Is 'phase' still the default?
			{
				phase = p;
			}
		}
	}

	if ( shouldDispatch )
	{
		SpriteEvent e( * this, phase );
		DispatchEvent( L, e );
	}
}

void
SpriteObject::Play( lua_State *L )
{
	if ( IsProperty( kIsPlayingEnded ) )
	{
		Reset();
	}

	if ( ! IsPlaying() )
	{
		if ( fPlayTime > 0 )
		{
			fStartTime = fPlayer.GetAnimationTime();

			Rtt_ASSERT( fStartTime >= fPlayTime );
			fStartTime -= fPlayTime;

			fPlayTime = 0;
		}

		SetPlaying( true );
	}
}

void
SpriteObject::Pause()
{
	if ( IsPlaying() )
	{
		U64 curTime = fPlayer.GetAnimationTime();
		Rtt_ASSERT( curTime >= fStartTime );
		fPlayTime = ( curTime - fStartTime );
		fStartTime = 0;

		SetPlaying( false );
	}
}

void
SpriteObject::SetSequence( const char *name )
{
	SpriteObjectSequence *sequence = GetCurrentSequence();

	if ( name && Rtt_StringCompare( name, sequence->GetName() ) != 0 )
	{
		// Find sequence since current sequence at 'index' does not match
		for ( int i = 0, iMax = fSequences.Length(); i < iMax; i++ )
		{
			if ( fCurrentSequence != i )
			{
				sequence = fSequences[i];
				if ( Rtt_StringCompare( name, sequence->GetName() ) == 0 )
				{
					if ( IsProperty( kIsMultiSprite ) )
					{
						BitmapPaint *paint = fPaint; Rtt_ASSERT( paint );

						// If sequence has a sheet, use its paint
						if ( sequence->HasSheet() )
						{
							paint = sequence->GetPaint(); Rtt_ASSERT( paint );
						}

						if ( paint )
						{
							SetFill( paint );

							// New texture, so need to update texture coords for new frame
							Invalidate( kGeometryFlag | kPaintFlag );
						}
					}
					else
					{
						// TODO: Re-enable these once we do the memory optimization above
						// Rtt_ASSERT( ! sequence->HasSheet() );
						// Rtt_ASSERT( ! sequence->GetPaint() );
					}

					fCurrentSequence = i;
					break;
				}
			}
		}
	}

	Reset();
}

const char*
SpriteObject::GetSequence() const
{
	const char *result = NULL;

	SpriteObjectSequence *sequence = GetCurrentSequence();
	if ( sequence )
	{
		result = sequence->GetName();
	}

	return result;
}

int
SpriteObject::GetNumSequences() const
{
	return fSequences.Length();
}

void
SpriteObject::SetFrame( int index )
{
	SpriteObjectSequence *sequence = GetCurrentSequence();

	if ( sequence )
	{
		// Clamp: 0 <= index < GetNumFrames()
		index = Max( index, 0 );
		index = Min( index, GetNumFrames() );

		if ( sequence->GetTime() > 0 )
		{
			// time-based, so frame changes depend on current time
			Real timePerFrame = sequence->GetTimePerFrame();
			Real playTime = timePerFrame * index;

			if ( ! Rtt_RealIsOne( fTimeScale ) )
			{
				playTime = Rtt_RealDiv( playTime, fTimeScale );
			}

			U64 curTime = fPlayer.GetAnimationTime();
			fStartTime = curTime - Rtt_RealToInt( playTime );
		}

		fSequenceIndex = index;

		int frameIndex = sequence->GetEffectiveFrame( index );
		SetBitmapFrame( frameIndex );
	}
}

/*
int
SpriteObject::GetFrame() const
{
	SpriteObjectSequence *sequence = GetCurrentSequence();
	int numFrames = sequence->GetEffectiveNumFrames();
	int sequenceIndex = fSequenceIndex;
	if ( IsProperty( kIsPlayingEnded ) )
	{
		--sequenceIndex;
	}
	
	sequenceIndex = sequenceIndex % numFrames;

	int result = sequenceIndex;
	int rawFrames = GetNumFrames();
	if ( result >= rawFrames )
	{
		result = 2*(rawFrames-1) - result;
	}

	return result;
}
*/
int
SpriteObject::GetFrame() const
{
	int result = 0;

	SpriteObjectSequence *sequence = GetCurrentSequence();
	if ( IsProperty( kIsPlayingEnded ) )
	{
		if ( SpriteObjectSequence::kBounce == sequence->GetLoopDirection() )
		{
			result = 0;
		}
		else
		{
			result = sequence->GetNumFrames() - 1;
		}
	}
	else
	{
//		int numFrames = sequence->GetEffectiveNumFrames();
		int sequenceIndex = fSequenceIndex;

		int rawFrames = GetNumFrames();
		int numSeqFrames = rawFrames;
		if ( SpriteObjectSequence::kBounce == sequence->GetLoopDirection() )
		{
			// For the bounce sequence: 1234543212345432...
			// The # seq frames is 8, since the 
			// base sequence that repeats is: 12345432...
			numSeqFrames = 2*(rawFrames - 1);
		}
		sequenceIndex = sequenceIndex % numSeqFrames;

//Rtt_TRACE( ( "seqIndex %d\n", sequenceIndex ) );

		result = sequenceIndex;
		if ( result >= rawFrames )
		{
			result = 2*(rawFrames-1) - result;
		}
	}

	return result;
}

int
SpriteObject::GetNumFrames() const
{
	SpriteObjectSequence *sequence = GetCurrentSequence();
	return sequence->GetNumFrames();
}

void
SpriteObject::SetProperty( PropertyMask mask, bool value )
{
	const Properties p = fProperties;
	fProperties = ( value ? p | mask : p & ~mask );
}

bool
SpriteObject::IsPlaying() const
{
	bool result = IsProperty( kIsPlaying );
	if ( result )
	{
		// For infinite loops, fSequenceIndex will always be in range
		// For finite loops, fSequenceIndex will eventually be out of range
		// which means the sprite has completed.
		SpriteObjectSequence *sequence = GetCurrentSequence();
		result = ( fSequenceIndex < sequence->GetEffectiveNumFrames() );
	}

	return result;
}

void
SpriteObject::SetPlaying( bool newValue )
{
	if ( IsProperty( kIsPlaying ) != newValue )
	{
		SetProperty( kIsPlaying, newValue );

		if ( newValue && ( 0 == fStartTime ) )
		{
			Rtt_ASSERT( ! IsProperty( kIsPlayingBegan ) );
			fStartTime = fPlayer.GetAnimationTime();
		}
	}
}

SpriteObjectSequence*
SpriteObject::GetCurrentSequence() const
{
	int index = fCurrentSequence;
	return fSequences[index];
}

void
SpriteObject::Reset()
{
	fProperties = ( kResetMask & fProperties );
	fSequenceIndex = 0;
	fStartTime = 0;
	fPlayTime = 0;

	// Set to initial frame
	SpriteObjectSequence *sequence = GetCurrentSequence();
	int frameIndex = sequence->GetEffectiveFrame( 0 );
	SetBitmapFrame( frameIndex );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

