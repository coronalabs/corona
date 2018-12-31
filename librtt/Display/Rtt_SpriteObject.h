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
			FrameIndex start,
			FrameIndex numFrames,
			int loopCount,
			Direction loopDirection );

		// Assumes ownership of 'frames' and assumed alloc'd via Rtt_MALLOC
		SpriteObjectSequence(
			Rtt_Allocator *allocator,
			const char *name,
			Real time,
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
		Real GetTimePerFrame() const { return fTimePerFrame; }
		Real GetEffectiveTime() const;

	public:
		// Returns index in sheet of first frame
		FrameIndex GetStartFrame() const;
		FrameIndex GetLastFrame() const;

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

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

	public:
		const AutoPtr< ImageSheet >& GetDefaultSheet() const;
		const AutoPtr< ImageSheet >& GetCurrentSheet() const;

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
		// Read-only properties
		int GetFrame() const;
		int GetNumFrames() const;

	public:
		Real GetTimeScale() const { return fTimeScale; }
		void SetTimeScale( Real newValue ) { fTimeScale = newValue; }

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
		int fSequenceIndex; // which frame in sheet are we currently showing
		U64 fStartTime;
		U64 fPlayTime; // when paused, stores amount of time played
		Properties fProperties;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_SpriteObject__
