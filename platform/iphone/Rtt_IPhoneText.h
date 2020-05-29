//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneText_H__
#define _Rtt_IPhoneText_H__

#import <UIKit/UITextInputTraits.h>

// ----------------------------------------------------------------------------

@class UIColor;

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

// Common functions between text objects
class IPhoneText
{
	public:
		static UIColor* GetTextColor( lua_State *L, int index, bool isByteColorRange );
		static UIReturnKeyType GetUIReturnKeyTypeFromIMEType( const char* imeType );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneText_H__
