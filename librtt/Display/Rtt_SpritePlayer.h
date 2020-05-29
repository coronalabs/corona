//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_SpritePlayer__
#define __Rtt_SpritePlayer__

#include "Core/Rtt_Array.h"
#include "Display/Rtt_SpriteObject.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

class SpritePlayer
{
	public:
		typedef SpritePlayer Self;

	public:
		SpritePlayer( Rtt_Allocator *pAllocator );

	public:
		void AddSprite( SpriteObject *object );
		void RemoveSprite( SpriteObject *object );

	public:
		void Run( lua_State *L, U64 millisecondTime );
		U64 GetAnimationTime() const;

	protected:
		void Update( lua_State *L );
		void Collect();

	private:
		U64 fAnimationTime;
		LightPtrArray< SpriteObject > fSprites;
		Array< int > fRemoveList; // indices in fSprites of items to remove
		bool fIsRunning;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_SpritePlayer__
