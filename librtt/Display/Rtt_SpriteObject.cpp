//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
#include "Display/Rtt_Display.h"

#include "Display/Rtt_LuaLibDisplay.h"

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
	
	FrameIndex *frames = NULL;
	int numFrames;
	if ( start > 0 )
	{
		lua_getfield( L, index, "count" );
		numFrames = (int) lua_tointeger( L, -1 );
		lua_pop( L, 1 );
		
		if ( numFrames <= 0 )
		{
			Rtt_TRACE_SIM( ( "WARNING: Invalid 'count' value(%d) in sequenceData. Assuming the frame count of the sequence is 1.\n", numFrames ) );
			numFrames = 1;
		}
	}
	else
	{
		lua_getfield( L, index, "frames" );
		if ( lua_istable( L, -1 ) )
		{
			numFrames = (int) lua_objlen( L, -1 );
			
			frames = (FrameIndex *)Rtt_MALLOC( allocator, numFrames * sizeof( FrameIndex ) );
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
		}
		lua_pop( L, 1 );
	}
	
	Real time = 0;
	Real *timeArray = NULL;
	lua_getfield( L, index, "time" );
	if ( lua_isnumber(L, -1) )
	{
		time = luaL_toreal( L, -1 );
	}
	else if ( lua_istable( L, -1 ) )
	{
		int numFramesInTimeArray = (int) lua_objlen( L, -1 );
		timeArray = (Real *)Rtt_MALLOC( allocator, numFrames * sizeof( Real ) );
		
		for ( int i = 0; i < Min(numFrames, numFramesInTimeArray); i++ ) // Resolve timeArray with available values in given lua array
		{
			lua_rawgeti( L, -1, i+1 ); // Lua is 1-based
			int value = (int)lua_tointeger( L, -1 );
			if ( value < 1 )
			{
				Rtt_TRACE_SIM( ( "WARNING: Invalid value(%d) in 'time' array. Assuming the frame's time is 1\n", value ) );
				value = 1;
			}
			timeArray[i] = value;
			lua_pop( L, 1 );
		}
		
		if (numFramesInTimeArray > numFrames)
		{
			Rtt_TRACE_SIM( ( "WARNING: Size of 'time' array (%d) in sequenceData differs from number of frames(%d). 'time' array will be cropped.\n", numFramesInTimeArray, numFrames ) );
		}
		else if ( numFramesInTimeArray < numFrames) // If given lua array was smaller, repeat last frame
		{
			Rtt_TRACE_SIM( ( "WARNING: Size of 'time' array (%d) in sequenceData differs from number of frames(%d). 'time' array will be extended with last frame time.\n", numFramesInTimeArray, numFrames ) );
			for ( int i = numFramesInTimeArray - 1; i < numFrames; i++ )
			{
				timeArray[i] = timeArray[numFramesInTimeArray - 1];
			}
		}
	}
	lua_pop( L, 1 );
	
	if ( start > 0 )
	{
		--start; // Lua value was 1-based
		result = Rtt_NEW( allocator, SpriteObjectSequence(
			allocator, name, time, timeArray, start, numFrames, loopCount, loopDirection ) );
	}
	else if ( frames != NULL)
	{
			result = Rtt_NEW( allocator, SpriteObjectSequence(
				allocator, name, time, timeArray, frames, numFrames, loopCount, loopDirection ) );
	}
	else
	{
		Rtt_TRACE_SIM( ( "ERROR: sequenceData missing data. One of the following must be supplied: a pair of properties 'start'/'count' or an array value for the property 'frames'.\n" ) );
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
	Real *timeArray,
	FrameIndex start,
	FrameIndex numFrames,
	int loopCount,
	Direction loopDirection )
:	fPaint( NULL ),
	fSheet(),
	fName( allocator, name ),
	fTime( time ),
	fTimeArray( timeArray ),
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
	Real *timeArray,
	FrameIndex *frames,
	FrameIndex numFrames,
	int loopCount,
	Direction loopDirection )
:	fPaint( NULL ),
	fSheet(),
	fName( allocator, name ),
	fTime( time ),
	fTimeArray( timeArray ),
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
	Rtt_FREE( fTimeArray );
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

int
SpriteObjectSequence::GetTimeForFrame( int frameIndex ) const
{
	Real *timeArray = GetTimeArray();
	if (timeArray == NULL)
	{
		return frameIndex * GetTimePerFrame();
	}
	else
	{
		Real summedTime = 0;
		for (int i = 0; i < frameIndex; ++i)
		{
			summedTime += timeArray[i];
		}
		return summedTime;
	}
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

static SpriteObject *
NewSprite( Rtt_Allocator * allocator, RectPath * path, const AutoPtr< ImageSheet > & sheet, SpritePlayer & player )
{
    return Rtt_NEW( allocator, SpriteObject( path, allocator, sheet, player ) );
}

SpriteObject*
SpriteObject::Create(
    lua_State * L,
	Rtt_Allocator *pAllocator,
	const AutoPtr< ImageSheet >& sheet,
	SpritePlayer& player,
	Display& display )
{
	auto * spriteFactory = GetObjectFactory( L, &NewSprite, display ); // n.b. done first to ensure factory function is consumed
	SpriteObject *result = NULL;

	if ( ! sheet->IsEmpty() )
	{
		const ImageFrame *frame = sheet->GetFrame( 0 );

		if ( Rtt_VERIFY( frame ) )
		{
			Real width = frame->GetWidth();
			Real height = frame->GetHeight();

			RectPath *path = RectPath::NewRect( pAllocator, width, height );

            result = spriteFactory( pAllocator, path, sheet, player );
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
		const AutoPtr<ImageSheet> &sheet,
		SpritePlayer &player )
		: Super(path),
			fPaint(NULL),
			fSheet(sheet),
			fSequences(pAllocator),
			fPlayer(player),
			fTimeScale(Rtt_REAL_1),
			fCurrentSequence(0), // Default is first sequence
			fCurrentFrame(0),
			fFrameForAnchors(NULL),
			fStartTime(0),
			fPlayTime(0),
			fTimeScaleIncrement(0),
			fProperties(0)
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
			ResetTimeArrayIteratorCache( sequence );
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

void
SpriteObject::GetSelfBoundsForAnchor( Rect& rect ) const
{
	if ( NULL != fFrameForAnchors )
	{
		// cf. TesselatorRect
		rect.Initialize( fFrameForAnchors->GetWidth() / 2, fFrameForAnchors->GetHeight() / 2 );
	}
	
	else
	{
		Super::GetSelfBoundsForAnchor( rect );
	}
}

bool
SpriteObject::GetTrimmedFrameOffsetForAnchor( Real& deltaX, Real& deltaY ) const
{
	if ( NULL != fFrameForAnchors )
	{
		deltaX = fFrameForAnchors->IsTrimmed() ? fFrameForAnchors->GetOffsetX() : 0;
		deltaY = fFrameForAnchors->IsTrimmed() ? fFrameForAnchors->GetOffsetY() : 0;

		return true;
	}

	else
	{
		return Super::GetTrimmedFrameOffsetForAnchor( deltaX, deltaY );
	}
}

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
		
		// Any trim correction will change the matrix.
		if (sheet->CorrectsTrimOffsets()) GetTransform().Invalidate();
	}

	// Store whether or not the new frame is trimmed or not
	SetProperty( kIsPreviousFrameTrimmed, isTrimmed );
	
	// Update texture coords for new frame
	Invalidate( kGeometryFlag );
	GetPath().Invalidate( ClosedPath::kFillSourceTexture );
}

void
SpriteObject::ResetTimeArrayIteratorCache(SpriteObjectSequence *sequence)
{
	Real *timeArray = sequence->GetTimeArray();
	if (timeArray != NULL){
		fTimeArrayCachedFrame = 0;
		fTimeArrayCachedNextFrameTime = timeArray[0];
	}
}

int
SpriteObject::GetFrameIndexForDeltaTime( Real dt, SpriteObjectSequence *sequence, int effectiveNumFrames)
{
	if (sequence->GetTimeArray() == NULL)
	{
		return (int)Rtt_RealDiv( dt, sequence->GetTimePerFrame() );
	}
	else if (dt < fTimeArrayCachedNextFrameTime)
	{
		return fTimeArrayCachedFrame;
	}
	else
	{
		int numFrames = sequence->GetNumFrames();
		Real *timeArray = sequence->GetTimeArray();
		
		// Increase cachedFrame until dt is lower than cachedNextFrameTime again OR effectiveNumFrames is reached when using finite loops
		for (int i = fTimeArrayCachedFrame; (0 == sequence->GetLoopCount() && dt > fTimeArrayCachedNextFrameTime) || i < effectiveNumFrames; ++i)
		{
			fTimeArrayCachedFrame += 1;
			
			int numSeqFrames = SpriteObjectSequence::kBounce == sequence->GetLoopDirection() ? 2 * (numFrames - 1) : numFrames;
			int nextFrame = fTimeArrayCachedFrame % numSeqFrames;
			if ( nextFrame >= numFrames ) {
				nextFrame = 2 * (numFrames - 1) - nextFrame;
			}
			
			fTimeArrayCachedNextFrameTime += timeArray[nextFrame];
			if (dt < fTimeArrayCachedNextFrameTime) break;
		}
		return fTimeArrayCachedFrame;
	}
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

	bool shouldDispach = false;
	SpriteEvent::Phase nextPhase = SpriteEvent::kNumPhases; // unknown

	// By default, assume frame does not advance
	int nextFrame = -1;

	SpriteObjectSequence *sequence = GetCurrentSequence();

	int effectiveNumFrames = sequence->GetEffectiveNumFrames();

	if ( ! IsProperty( kIsPlayingBegan ) )
	{
		// Base case: first frame
		nextFrame = fCurrentFrame;

		SetProperty( kIsPlayingBegan, true );

		if ( HasListener( kSpriteListener ) )
		{
			shouldDispach = true;
			nextPhase = SpriteEvent::kBegan;
		}
	}
	else
	{
		// Inductive step: advance frame
		if ( sequence->GetTime() > 0 || sequence->GetTimeArray() != NULL)
		{
			// time-based sequence.
			Real dt = Rtt_IntToReal((U32)(milliseconds - fStartTime + fTimeScaleIncrement));
			if ( ! Rtt_RealIsOne( fTimeScale ) )
			{
				dt = Rtt_RealMul( dt, fTimeScale );
			}
			
			int frameIndexForDeltaTime = GetFrameIndexForDeltaTime(dt, sequence, effectiveNumFrames);

			// Infinite looping will never let fCurrentFrame > numFrames, finite looping does.
			bool isInfiniteLooping = ( 0 == sequence->GetLoopCount() );
			if ( isInfiniteLooping )
			{
				bool isFirstLoopDone = ( frameIndexForDeltaTime >= effectiveNumFrames );
				
				frameIndexForDeltaTime = frameIndexForDeltaTime % effectiveNumFrames;
				if ( isFirstLoopDone && frameIndexForDeltaTime < fCurrentFrame )
				{
					if ( HasListener( kSpriteListener ) )
					{
						shouldDispach = true;
						nextPhase = SpriteEvent::kLoop;
					}

					if (sequence -> GetTimeArray() != NULL){
						fTimeArrayCachedFrame = 0; // Just reset fTimeArrayCachedFrame since dt keeps counting.
					}
					fCurrentFrame = 0;
					nextFrame = 0;
				}
			}

			if ( frameIndexForDeltaTime > fCurrentFrame )
			{
				// Only dispatch if we advance.
				// Assume the phase is 'next' for now, and set to other phases below.
				if ( HasListener( kSpriteListener ) )
				{
					shouldDispach = true;
					nextPhase = SpriteEvent::kNext;
				}

				// Check for "ended" on finite loops
				if ( ! isInfiniteLooping )
				{
					// Handle end condition when we are at the last frame
					int lastIndex = ( effectiveNumFrames - 1 );

					if ( frameIndexForDeltaTime >= lastIndex )
					{
						frameIndexForDeltaTime = lastIndex;

						// We're about to advance beyond the last frame
						if ( ! IsProperty( kIsPlayingEnded ) )
						{
							SetPlaying( false );
							SetProperty( kIsPlayingEnded, true );

							if ( HasListener( kSpriteListener ) )
							{
								shouldDispach = true;
								nextPhase = SpriteEvent::kEnded;
							}
						}
					}
				}

				fCurrentFrame = frameIndexForDeltaTime;
				nextFrame = frameIndexForDeltaTime;
			}
		}
		else
		{
			// frame-based sequence, so every update is an advance
			nextFrame = ++fCurrentFrame;

			// Assume the phase is 'next' for now, and set to other phases below.
			if ( HasListener( kSpriteListener ) )
			{
				shouldDispach = true;
				nextPhase = SpriteEvent::kNext;
			}

			bool isInfiniteLooping = ( 0 == sequence->GetLoopCount() );
			if ( isInfiniteLooping )
			{
				// For infinite looping, handle wrap-around (no last frame)
				bool isFirstLoopDone = ( nextFrame >= effectiveNumFrames );
				if ( isFirstLoopDone )
				{
					nextFrame = 0;
					fCurrentFrame = nextFrame;

					if ( HasListener( kSpriteListener ) )
					{
						shouldDispach = true;
						nextPhase = SpriteEvent::kLoop;
					}
				}
			}
			else
			{
				// Handle end condition when we are at the last frame
				int lastIndex = ( effectiveNumFrames - 1 );
				if ( nextFrame >= lastIndex )
				{
					nextFrame = lastIndex;
					fCurrentFrame = nextFrame;
					
					// We're about to advance beyond the last frame
					if ( ! IsProperty( kIsPlayingEnded ) )
					{
						SetPlaying( false );
						SetProperty( kIsPlayingEnded, true );
						if ( HasListener( kSpriteListener ) )
						{
							shouldDispach = true;
							nextPhase = SpriteEvent::kEnded;
						}
					}
				}
			}
		}
	}

	// Advance frame if valid index is set
	if ( nextFrame >= 0 && nextFrame < effectiveNumFrames )
	{
		SpriteEvent::Phase defaultPhase = SpriteEvent::kNext;
		int effectiveFrame = sequence->GetEffectiveFrame( nextFrame, & defaultPhase );
		SetBitmapFrame( effectiveFrame );

		if ( shouldDispach
			 && SpriteEvent::kNext != defaultPhase // Did 'defaultPhase' change from default within GetEffectiveFrame?
			 && SpriteEvent::kNext == nextPhase )	// Is 'phase' still the default?
		{
			nextPhase = defaultPhase;
		}
	}

	if ( shouldDispach )
	{
		SpriteEvent e( * this, nextPhase );
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

					// Anchor frame is sequence-related, so invalidate it.
					if (fFrameForAnchors)
					{
						fFrameForAnchors = NULL;

						Invalidate( kTransformFlag );
						
						GetTransform().Invalidate();
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
		
		if ( sequence->GetTime() > 0 || sequence->GetTimeArray() != NULL )
		{
			// time-based, so frame changes depend on current time
			Real playTime = sequence->GetTimeForFrame(index);
			if ( ! Rtt_RealIsOne( fTimeScale ) )
			{
				playTime = Rtt_RealDiv( playTime, fTimeScale );
			}

			if ( !IsPlaying() )
			{
				fPlayTime = Rtt_RealToInt( playTime );
			}
			else
			{
				U64 curTime = fPlayer.GetAnimationTime();
				fStartTime = curTime - Rtt_RealToInt( playTime );
			}
		}

		fCurrentFrame = index;
		ResetTimeArrayIteratorCache(sequence);

		int frameIndex = sequence->GetEffectiveFrame( index );
		SetBitmapFrame( frameIndex );
	}
}

void
SpriteObject::UseFrameForAnchors( int index )
{
	const SpriteObjectSequence *sequence = GetCurrentSequence();

	if (sequence)
	{
		Paint *paint = Super::GetPath().GetFill();
		Rtt_ASSERT( paint->IsCompatibleType( Paint::kBitmap ) );

		ImageSheetPaint *bitmapPaint = (ImageSheetPaint *)paint->AsPaint(Paint::kImageSheet);

		// Ensure 0 <= frameIndex < sheet->GetNumFrames()
		int maxFrameIndex = sequence->GetNumFrames() - 1;
		index = Min( index, maxFrameIndex );
		index = Max( index, 0 );
		index = sequence->GetEffectiveFrame( index );
		
		const AutoPtr< ImageSheet >& sheet = bitmapPaint->GetSheet();
		fFrameForAnchors = sheet->GetFrame( index );

		Invalidate( kTransformFlag );
		
		GetTransform().Invalidate();
	}
}

/*
int
SpriteObject::GetFrame() const
{
	SpriteObjectSequence *sequence = GetCurrentSequence();
	int numFrames = sequence->GetEffectiveNumFrames();
	int sequenceIndex = fCurrentFrame;
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
		int currentFrame = fCurrentFrame;

		int rawFrames = GetNumFrames();
		int numSeqFrames = rawFrames;
		if ( SpriteObjectSequence::kBounce == sequence->GetLoopDirection() )
		{
			// For the bounce sequence: 1234543212345432...
			// The # seq frames is 8, since the 
			// base sequence that repeats is: 12345432...
			numSeqFrames = 2*(rawFrames - 1);
		}
		currentFrame = currentFrame % numSeqFrames;

//Rtt_TRACE( ( "seqIndex %d\n", sequenceIndex ) );

		result = currentFrame;
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
		// For infinite loops, fCurrentFrame will always be in range
		// For finite loops, fCurrentFrame will eventually be out of range
		// which means the sprite has completed.
		SpriteObjectSequence *sequence = GetCurrentSequence();
		result = ( fCurrentFrame < sequence->GetEffectiveNumFrames() );
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

void
SpriteObject::SetTimeScale( Real newValue ) {
  SpriteObjectSequence *sequence = GetCurrentSequence();

  if ( sequence )
  {
		if (!IsPlaying()) 
		{
			Real playTime = sequence->GetTimeForFrame(fCurrentFrame);
			Real newTimeElapsed = Rtt_RealDiv(playTime, newValue) - Rtt_IntToReal(fPlayTime);
			fTimeScaleIncrement = newTimeElapsed;
		} 
		else 
		{
			U64 curTime = fPlayer.GetAnimationTime();
			Real timeElapsed = curTime - fStartTime;

			Real newTimeElapsed = Rtt_RealDiv(Rtt_RealMul(timeElapsed + Rtt_IntToReal(fTimeScaleIncrement), fTimeScale), newValue);
			fTimeScaleIncrement = Rtt_RealToInt(newTimeElapsed-timeElapsed);
		}
	}


  fTimeScale = newValue;
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
	fCurrentFrame = 0;
	fStartTime = 0;
	fPlayTime = 0;
	fTimeScaleIncrement = 0;

			// Set to initial frame
			SpriteObjectSequence *sequence = GetCurrentSequence();
	ResetTimeArrayIteratorCache(sequence);
	int frameIndex = sequence->GetEffectiveFrame( 0 );
	SetBitmapFrame( frameIndex );
}
	

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

