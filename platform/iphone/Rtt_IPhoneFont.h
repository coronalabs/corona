//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneFont_H__
#define _Rtt_IPhoneFont_H__

#include "Rtt_PlatformFont.h"

@class UIFont;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneFont : public PlatformFont
{
	public:
		typedef IPhoneFont Self;
		typedef PlatformFont Super;

	public:
		IPhoneFont( const char *fontName, Rtt_Real size, NSString *directory );
		IPhoneFont( PlatformFont::SystemFont fontType, Rtt_Real size );
		IPhoneFont( UIFont *font );
		IPhoneFont( const IPhoneFont &font );
		virtual ~IPhoneFont();

	public:
		// PlaformFont
		virtual PlatformFont* CloneUsing( Rtt_Allocator *allocator ) const;
		virtual const char* Name() const;
		virtual void SetSize( Real newValue );
		virtual Rtt_Real Size() const;
		virtual void* NativeObject() const;

	private:
		UIFont *fFont;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneFont_H__
