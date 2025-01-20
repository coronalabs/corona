//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_ImageSheetUserdata__
#define __Rtt_ImageSheetUserdata__

#include "Core/Rtt_AutoPtr.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class ImageSheet;

// ----------------------------------------------------------------------------

class ImageSheetUserdata
{
	public:
		typedef ImageSheetUserdata Self;

		typedef AutoPtr< ImageSheet > AutoImageSheet;

	public:
		ImageSheetUserdata( const AutoImageSheet& pSheet );

	public:
		const AutoImageSheet& GetSheet() const { return fSheet; }

	private:
		AutoImageSheet fSheet;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ImageSheetUserdata__
