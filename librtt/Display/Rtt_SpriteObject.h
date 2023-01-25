//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_SpriteObject__
#define __Rtt_SpriteObject__

#include "Display/Rtt_RectObject.h"
#include "Display/Rtt_ImageSheet.h"
#include "Rtt_Event.h"

#include "Core/Rtt_Array.h"
#include "Core/Rtt_AutoPtr.h"
#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class ImageSheetPaint;
class SpriteObject;
class SpritePlayer;

// ----------------------------------------------------------------------------

// A sequence defines which frames in the ImageSheet will be used
class SpriteObjectSequence
{
	public:
		typedef S16 FrameIndex;

	public:
		typedef enum _Direction
		{
			kForward = 0,
			kBounce,

			kNumDirection
		}
		Direction;

		static Direction DirectionForString( const char *value );

	public:
		static SpriteObjectSequence* Create( Rtt_Allocator *allocator, lua_State *L, int index );

	protected:
		SpriteObjectSequence(
			Rtt_Allocator *allocator,
			const char *name,
			Real time,
			Real *timeArray,
			FrameIndex start,
			FrameIndex numFrames,
			int loopCount,
			Direction loopDirection );

		// Assumes ownership of 'frames' and assumed alloc'd via Rtt_MALLOC
		SpriteObjectSequence(
			Rtt_Allocator *allocator,
			const char *name,
			Real time,
			Real *timeArray,
			FrameIndex *frames,
			FrameIndex numFrames,
			int loopCount,
			Direction loopDirection );

	public:
		~SpriteObjectSequence();

	protected:
		// Called when sheets are per-sequence
		void Initialize( Rtt_Allocator *pAllocator, const AutoPtr< ImageSheet >& sheet );

	public:
		void Verify( const SpriteObject& owner ) const;

	public:
		const AutoPtr< ImageSheet >& GetSheet() const { return fSheet; }
		bool HasSheet() const { return fSheet.NotNull(); }

		ImageSheetPaint *GetPaint() const { return fPaint; }

	public:
		const char* GetName() const { return fName.GetString(); }
		Real GetTime() const { return fTime; }
		Real *GetTimeArray() const { return fTimeArray; }
		Real GetTimePerFrame() const { return fTimePerFrame; }

	public:
		// Returns index in sheet of first frame
		FrameIndex GetStartFrame() const;
		FrameIndex GetLastFrame() const;
		int GetTimeForFrame( int frameIndex ) const;

	protected:
		FrameIndex GetFrame( int i ) const;

	public:
		int GetNumFrames() const { return fNumFrames; }

	public:
		// For 0 <= i < GetEffectiveNumFrames() where i is the sequence index
		// int GetFrameIndex( int i ) const;

		// Returns index in sheet of ith frame of the sequence.
		// 'i' is the sequence index
		// Note that 0 <= i < GetNumEffectiveFrames()
		FrameIndex GetEffectiveFrame( int i, SpriteEvent::Phase *phase = NULL ) const;

	public:
		// Returns number of frames in sequence.
		// Note that if loop direction is kBounce, then this returns the number
		// of frames in the effective sequence, not the base sequence
		int GetEffectiveNumFrames() const;

		bool IsConsecutiveFrames() const { return fStart >= 0; }

		int GetLoopCount() const { return fLoopCount; }
		Direction GetLoopDirection() const { return fLoopDirection; }

	private:
		ImageSheetPaint *fPaint;
		AutoPtr< ImageSheet > fSheet;
		String fName;
		Real fTime;				// Length of sequence in ms
		Real *fTimeArray;
		Real fTimePerFrame;
		FrameIndex fNumFrames;	// Raw number of frames
		FrameIndex fStart;		// Sequence is defined by consecutive frames in the sheet
		FrameIndex *fFrames;	// or an array of frame indices. 
		int fLoopCount;
		Direction fLoopDirection;
};

class SpriteObject : public RectObject
{
	public:
		typedef RectObject Super;
		typedef SpriteObject Self;

	protected:
		typedef enum _PropertyMask
		{
			kIsPlaying = 0x1,
			kIsPlayingBegan = 0x2,
			kIsPlayingEnded = 0x4,
			kIsMarked = 0x8,
			kIsMultiSprite = 0x10,
			kIsPreviousFrameTrimmed = 0x20,

			// Contains the values we want to preserve across a Reset() call
			kResetMask = kIsMultiSprite
		}
		PropertyMask;

		typedef Properties U8;

	public:
		static SpriteObject* Create(
			Rtt_Allocator *pAllocator,
			const AutoPtr< ImageSheet >& sheet,
			SpritePlayer& player );

	protected:
		SpriteObject(
			RectPath *path,
			Rtt_Allocator *pAllocator,
			const AutoPtr< ImageSheet >& sheet,
			SpritePlayer& player );

	public:
		virtual ~SpriteObject();

		void Initialize( Rtt_Allocator *pAllocator );

	public:
		// Receiver takes ownership of 'sequence'
		void AddSequence( SpriteObjectSequence *sequence );

	public:
		// MDrawable
		virtual void Prepare( const Display& display );
//		virtual void Translate( Real dx, Real dy );
//		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBoundsForAnchor( Rect& rect ) const;
		virtual bool GetTrimmedFrameOffsetForAnchor( Real& deltaX, Real& deltaY ) const;

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

	public:
		const AutoPtr< ImageSheet >& GetDefaultSheet() const;
		const AutoPtr< ImageSheet >& GetCurrentSheet() const;
		void ResetTimeArrayIteratorCache(SpriteObjectSequence *sequence);
		int GetFrameIndexForDeltaTime( Real dt, SpriteObjectSequence *sequence, int effectiveNumFrames );
	
	protected:
		void SetBitmapFrame( int frameIndex );

	public:
		void Update( lua_State *L, U64 milliseconds );

	public:
		void Play( lua_State *L );
		void Pause();
		void SetSequence( const char *name );
		const char* GetSequence() const;
		int GetNumSequences() const;
		void SetFrame( int index );

	public:
		void UseFrameForAnchors( int index );
	
	public:
		// Read-only properties
		int GetFrame() const;
		int GetNumFrames() const;

	public:
		Real GetTimeScale() const { return fTimeScale; }
		void SetTimeScale( Real newValue );

	protected:
		bool IsProperty( PropertyMask mask ) const { return ( !! ( mask & fProperties ) ); }
		void SetProperty( PropertyMask mask, bool value );

	public:
		bool IsPlaying() const;
		void SetPlaying( bool newValue );

	public:
		bool IsMarked() const { return IsProperty( kIsMarked ); }
		void SetMarked( bool newValue ) { SetProperty( kIsMarked, newValue ); }

	protected:
		SpriteObjectSequence* GetCurrentSequence() const;
		void Reset();

	private:
		ImageSheetPaint *fPaint; // Weak ref to paint
		AutoPtr< ImageSheet > fSheet; // shared sheet (if all sequences use same sheet)
		PtrArray< SpriteObjectSequence > fSequences;
		SpritePlayer& fPlayer;
		Real fTimeScale;
		int fCurrentSequence; // index into fSequences of current sequence
		int fCurrentFrame; // which frame in sheet are we currently showing
		const ImageFrame *fFrameForAnchors;
		U64 fStartTime;
		U64 fPlayTime; // when paused, stores amount of time played
		U64 fTimeScaleIncrement; // Stores the extra amount of time it need to be added for a given timeScale
		int fTimeArrayCachedFrame; // stores iterator state for SpriteObjectSequence::GetFrameIndexForDeltaTime()
		Real fTimeArrayCachedNextFrameTime; // stores iterator state for SpriteObjectSequence::GetFrameIndexForDeltaTime()
	
		Properties fProperties;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_SpriteObject__
