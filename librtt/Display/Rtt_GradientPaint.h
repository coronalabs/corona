//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_GradientPaint__
#define __Rtt_GradientPaint__

#include "Display/Rtt_BitmapPaint.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class TextureFactory;

// ----------------------------------------------------------------------------

class GradientPaint : public BitmapPaint
{
	public:
		typedef BitmapPaint Super;

	public:
		enum Direction
		{
			kUpDirection = 0,
			kRightDirection,
			kDownDirection,
			kLeftDirection,

			kNumDirections,
			kDefaultDirection = kDownDirection,
		};

	public:
		static Direction StringToDirection( const char *str );

	public:
		static GradientPaint *New(
			TextureFactory& factory,
			Color start,
			Color end,
			Direction direction,
			Rtt_Real angle );

	public:
		GradientPaint( const SharedPtr< TextureResource >& resource, Rtt_Real angle );

	public:
		virtual const Paint* AsPaint( Super::Type type ) const;
		virtual const MLuaUserdataAdapter& GetAdapter() const;

	public:
		Color GetStart() const;
		Color GetEnd() const;
		void SetStart( Color color );
		void SetEnd( Color color );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_GradientPaint__
