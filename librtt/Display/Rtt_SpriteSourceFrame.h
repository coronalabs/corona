//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_SpriteSourceFrame_H__
#define _Rtt_SpriteSourceFrame_H__

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

struct IntPoint
{
	int x;
	int y;
};

struct SpriteSourceFrame
{
	static void Initialize( lua_State *L, SpriteSourceFrame& sourceFrame );

	IntPoint fTrimmedOrigin;	// sprite rect relative to source rect, origin at top left
	IntPoint fSourceSize;		// original size before trimming
	IntPoint fSheetOrigin;		// origin in sheet
	IntPoint fSheetSize;		// size in sheet
	bool fTrimmed;				// Is the sprite trimmed
	bool fRotated;				// Is the sprite rotated
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_SpriteSourceFrame_H__
