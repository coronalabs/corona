//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_ImageSheetPaintAdapter__
#define __Rtt_ImageSheetPaintAdapter__

#include "Display/Rtt_BitmapPaintAdapter.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class ImageSheetPaintAdapter : public BitmapPaintAdapter
{
	public:
		typedef BitmapPaintAdapter Super;

	public:
		static const ImageSheetPaintAdapter& Constant();

	protected:
		ImageSheetPaintAdapter();

	public:
		virtual int ValueForKey(
			const LuaUserdataProxy& sender,
			lua_State *L,
			const char *key ) const override;

		virtual bool SetValueForKey(
			LuaUserdataProxy& sender,
			lua_State *L,
			const char *key,
			int valueIndex ) const override;

        // Return the hash table containing the adapter's property keys
        virtual StringHash *GetHash( lua_State *L ) const override;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ImageSheetPaintAdapter__
