//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_EmbossedTextObject_H__
#define _Rtt_EmbossedTextObject_H__

#include "Core/Rtt_Real.h"
#include "Core/Rtt_String.h"
#include "Display/Rtt_TextObject.h"
#include "Renderer/Rtt_RenderTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Geometry;
class PlatformFont;
class Uniform;

// ----------------------------------------------------------------------------

/// Renders text with an embossed style, giving it an engraved style.
/// <br>
/// This is achieved by rendering the text 3 times, where each text mask is offsetted by 1 pixel.
/// The top-left text is dark, showing a shadow effect.
/// The bottom-right text is white, showing a highlight effect.
/// The text in the middle is the normal text using the provided color.
class EmbossedTextObject : public TextObject
{
	Rtt_CLASS_NO_COPIES( EmbossedTextObject )

	public:
		typedef TextObject Super;

	public:
		// TODO: Use a string class instead...
		// TextObject retains ownership of font
		EmbossedTextObject( Display& display, const char text[], PlatformFont *font, Real w, Real h, const char alignment[] );
		virtual ~EmbossedTextObject();

		void SetHighlightColor(RGBA color);
		RGBA GetHighlightColor();
		void SetShadowColor(RGBA color);
		RGBA GetShadowColor();
		void UseDefaultHighlightColor();
		void UseDefaultShadowColor();
		virtual void Prepare( const Display& display );
		virtual void Draw( Renderer& renderer ) const;
		virtual const LuaProxyVTable& ProxyVTable() const;

	private:
		RGBA GetForeColor() const;
		bool IsColorBright(RGBA color) const;

		mutable Geometry *fHighlightGeometry;
		mutable Geometry *fShadowGeometry;
		mutable Uniform *fHighlightMaskUniform;
		mutable Uniform *fShadowMaskUniform;
		RGBA fHighlightColor;
		RGBA fShadowColor;
		bool fIsUsingDefaultHighlightColor;
		bool fIsUsingDefaultShadowColor;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_EmbossedTextObject_H__
