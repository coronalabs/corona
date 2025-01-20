//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmbossedTextObject.h"
#include "Core/Rtt_String.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_RectPath.h"
#include "Display/Rtt_Shader.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_RenderTypes.h"
#include "Renderer/Rtt_Uniform.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"

#include "Rtt_Profiling.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new display object for rendering text with an embossed effect.
/// @param display Reference to the display to render to.
/// @param text The string to be displayed. Can be NULL or empty.
/// @param font The font to be used when rendering the given string.
/// @param w The wrapping width in content coordinates. Set to zero if you don't want line wrapping.
/// @param h The clipping height for multiline text. Set to zero for no clipping.
/// @param alignment Set to "left", "center", or "right" for the respective text alignment.
///                  Setting this to NULL will default it to "left" text alignment.
EmbossedTextObject::EmbossedTextObject(
	Display& display, const char text[], PlatformFont *font, Real w, Real h, const char alignment[])
:	Super( display, text, font, w, h, alignment ),
	fHighlightGeometry( NULL ),
	fShadowGeometry( NULL ),
	fHighlightMaskUniform( Rtt_NEW( display.GetAllocator(), Uniform( display.GetAllocator(), Uniform::kMat3 ) ) ),
	fShadowMaskUniform( Rtt_NEW( display.GetAllocator(), Uniform( display.GetAllocator(), Uniform::kMat3 ) ) ),
	fIsUsingDefaultHighlightColor( true ),
	fIsUsingDefaultShadowColor( true )
{
    SetObjectDesc( "EmbossedTextObject" );
}

/// Destroys this text object and its resources.
EmbossedTextObject::~EmbossedTextObject()
{
	QueueRelease( fShadowMaskUniform );
	QueueRelease( fHighlightMaskUniform );
	QueueRelease( fShadowGeometry );
	QueueRelease( fHighlightGeometry );
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the RGBA highlight color shown 1 pixel to the bottom-right of the text.
/// <br>
/// If you've never called the SetHighlightColor() function, then this object will use a default
/// white highlight color. But note that the default color's alpha value will change depending on the
/// brightness of the foreground text color.
/// @return Returns the highlight color to be used.
RGBA
EmbossedTextObject::GetHighlightColor()
{
	if (fIsUsingDefaultHighlightColor)
	{
		RGBA foreColor = GetForeColor();
		fHighlightColor.r = 255;
		fHighlightColor.g = 255;
		fHighlightColor.b = 255;
		fHighlightColor.a = IsColorBright(foreColor) ? 20 : 140;
		fHighlightColor.a = (U8)((((int)fHighlightColor.a * (int)foreColor.a) >> 8 ) * (int)Alpha() >> 8);

	}
	return fHighlightColor;
}

/// Sets the highlight color to be shown 1 pixel to the bottom-right of the text.
/// @param color The RGBA color value to be used.
void
EmbossedTextObject::SetHighlightColor(RGBA color)
{
	// Determine if the color is changing.
	bool wasChanged = false;
	if (fIsUsingDefaultHighlightColor || memcmp(&color, &fHighlightColor, sizeof(RGBA)))
	{
		wasChanged = true;
	}
	
	// Store the custom emboss color.
	fHighlightColor = color;
	fIsUsingDefaultHighlightColor = false;
	
	// Invalidate this object if changed.
	if (wasChanged)
	{
		Invalidate(kColorFlag);
	}
}

/// Gets the RGBA shadow color shown 1 pixel to the rop-left of the text.
/// <br>
/// If you've never called the SetShadowColor() function, then this object will use a default
/// dark color. But note that the default color's alpha value will change depending on the
/// brightness of the foreground text color.
/// @return Returns the shadow color to be used.
RGBA
EmbossedTextObject::GetShadowColor()
{
	if (fIsUsingDefaultShadowColor)
	{
		RGBA foreColor = GetForeColor();
		fShadowColor.r = 0;
		fShadowColor.g = 0;
		fShadowColor.b = 0;
		fShadowColor.a = IsColorBright(foreColor) ? 128 : 20;
		fShadowColor.a = (U8)((((int)fShadowColor.a * (int)foreColor.a) >> 8 ) * (int)Alpha() >> 8);
	}
	return fShadowColor;
}

/// Sets the shadow color to be shown 1 pixel to the top-left of the text.
/// @param color The RGBA color value to be used.
void
EmbossedTextObject::SetShadowColor(RGBA color)
{
	// Determine if the color is changing.
	bool wasChanged = false;
	if (fIsUsingDefaultShadowColor || memcmp(&color, &fHighlightColor, sizeof(RGBA)))
	{
		wasChanged = true;
	}
	
	// Store the custom emboss color.
	fShadowColor = color;
	fIsUsingDefaultShadowColor = false;
	
	// Invalidate this object if changed.
	if (wasChanged)
	{
		Invalidate(kColorFlag);
	}
}

/// Forces this display object to use its default white highlight color in the bottom-right of the text.
/// <br>
/// Note that calling the SetHighlightColor() function forces this object to use the custom color
/// that you gave it. You must call this function to return it to the default color scheme.
void
EmbossedTextObject::UseDefaultHighlightColor()
{
	if (!fIsUsingDefaultHighlightColor)
	{
		fIsUsingDefaultHighlightColor = true;
		Invalidate(kColorFlag);
	}
}

/// Forces this display object to use its default dark shadow color in the top-left of the text.
/// <br>
/// Note that calling the SetShadowColor() function forces this object to use the custom color
/// that you gave it. You must call this function to return it to the default color scheme.
void
EmbossedTextObject::UseDefaultShadowColor()
{
	if (!fIsUsingDefaultShadowColor)
	{
		fIsUsingDefaultShadowColor = true;
		Invalidate(kColorFlag);
	}
}

/// Updates this display object's geometry, colors, and textures.
/// <br>
/// This function is only expected to be called by the rendering system.
/// @param display Reference to the display this object will be rendering to.
void
EmbossedTextObject::Prepare( const Display& display )
{
	SUMMED_TIMING( etp, "EmbossedText: Prepare" );

	Real offsetX;
	Real offsetY;
	RGBA backColor;

	// The base text object must be prepared first.
	// This generates/updates its text bitmap and updates its transformation matrix.
	Super::Prepare( display );
	if (IsInitialized() == false)
	{
		return;
	}

	// Fetch this text object's geometry/vertices based on bitmap generated above.
	const Geometry *geometryPointer = GetFillData().fGeometry;
	if (!geometryPointer)
	{
		return;
	}

	// Update the highlight text's rendering data.
	{
		// Fetch the highlight color and apply premultiplied alpha.
		// Note: The default color might change according to the brightness of the foreground text.
		backColor = GetHighlightColor();
		backColor.a = (U8)( (unsigned)backColor.a*AlphaCumulative()/255u );
		if (backColor.a < 255)
		{
			// Apply premultiplied alpha.
			backColor.r = (U8)(((int)backColor.r * (int)backColor.a) >> 8);
			backColor.g = (U8)(((int)backColor.g * (int)backColor.a) >> 8);
			backColor.b = (U8)(((int)backColor.b * (int)backColor.a) >> 8);
		}

		// Set the offset to 1 pixel to the bottom-right of the foreground text.
		// Convert this offset to content coordinates.
		offsetX = display.GetSx();
		offsetY = display.GetSy();

		// Update the highlight text's vertices, colors, and mask coordinates.
		QueueRelease( fHighlightGeometry );
		fHighlightGeometry = Rtt_NEW( display.GetAllocator(), Geometry( *geometryPointer ) );
		for (int index = (int)fHighlightGeometry->GetVerticesUsed() - 1; index >= 0; index--)
		{
			Geometry::Vertex *vertexPointer = &(fHighlightGeometry->GetVertexData()[index]);
			vertexPointer->x += offsetX;
			vertexPointer->y += offsetY;
			vertexPointer->rs = backColor.r;
			vertexPointer->gs = backColor.g;
			vertexPointer->bs = backColor.b;
			vertexPointer->as = backColor.a;
		}
		Matrix translationMatrix(GetSrcToDstMatrix());
		translationMatrix.Translate(offsetX, offsetY);
		UpdateMaskUniform( *fHighlightMaskUniform, translationMatrix, *GetMask() );
	}

	// Update the shadow text's rendering data.
	{
		// Fetch the shadow color and apply premultiplied alpha.
		// Note: The default color might change according to the brightness of the foreground text.
		backColor = GetShadowColor();
		backColor.a = (U8)( (unsigned)backColor.a*AlphaCumulative()/255u );
		if (backColor.a < 255)
		{
			// Apply premultiplied alpha.
			backColor.r = (U8)(((int)backColor.r * (int)backColor.a) >> 8);
			backColor.g = (U8)(((int)backColor.g * (int)backColor.a) >> 8);
			backColor.b = (U8)(((int)backColor.b * (int)backColor.a) >> 8);
		}

		// Set the offset to 1 pixel to the top-left of the foreground text.
		// Convert this offset to content coordinates.
		offsetX = -display.GetSx();
		offsetY = -display.GetSy();

		// Update the shadow text's vertices, colors, and mask coordinates.
		QueueRelease( fShadowGeometry );
		fShadowGeometry = Rtt_NEW( display.GetAllocator(), Geometry( *geometryPointer ) );
		for (int index = (int)fShadowGeometry->GetVerticesUsed() - 1; index >= 0; index--)
		{
			Geometry::Vertex *vertexPointer = &(fShadowGeometry->GetVertexData()[index]);
			vertexPointer->x += offsetX;
			vertexPointer->y += offsetY;
			vertexPointer->rs = backColor.r;
			vertexPointer->gs = backColor.g;
			vertexPointer->bs = backColor.b;
			vertexPointer->as = backColor.a;
		}
		Matrix translationMatrix(GetSrcToDstMatrix());
		translationMatrix.Translate(offsetX, offsetY);
		UpdateMaskUniform( *fShadowMaskUniform, translationMatrix, *GetMask() );
	}
}

/// Draws this object's embossed text to the display.
/// @param renderer Reference to the renderer used to draw the text.
void
EmbossedTextObject::Draw( Renderer& renderer ) const
{
	if ( ShouldDraw() )
	{
		SUMMED_TIMING( etd, "EmbossedText: Draw" );

		// Draw the highlight text.
		RenderData highlightData = GetFillData();
		highlightData.fGeometry = fHighlightGeometry;
		highlightData.fMaskUniform = fHighlightMaskUniform;
		GetFillShader()->Draw( renderer, highlightData );

		// Draw the shadow text.
		RenderData shadowData = GetFillData();
		shadowData.fGeometry = fShadowGeometry;
		shadowData.fMaskUniform = fShadowMaskUniform;
		GetFillShader()->Draw( renderer, shadowData );

		// Draw the main foreground text.
		Super::Draw( renderer );
	}
}

/// Obtains this object's Lua API interface.
/// @return Returns a reference to the Lua interface for this object.
const LuaProxyVTable&
EmbossedTextObject::ProxyVTable() const
{
	return LuaEmbossedTextObjectProxyVTable::Constant();
}


// ----------------------------------------------------------------------------
// Private Member Functions
// ----------------------------------------------------------------------------

/// Gets the color for the main foreground text, which is displayed on top of the
/// highlight and shadow text.
/// <br>
/// This is obtained from the TextObject this class inherits from.
/// @return Returns the foreground text color.
RGBA
EmbossedTextObject::GetForeColor() const
{
	RGBA color = { 255, 255, 255, 255 };
	const Paint *paintPointer = GetPath().GetFill();
	if (paintPointer)
	{
		color = paintPointer->GetRGBA();
	}
	return color;
}

/// Determines if the given color is considered bright (ie: near white).
/// @return Returns true if the color is bright. Returns false if dark.
bool
EmbossedTextObject::IsColorBright(RGBA color) const
{
	// The color is considered bright if the sum of the RGB components is above the halfway mark.
	// Note: 381 = (255 + 255 + 255) / 2
	return (((int)color.r + (int)color.g + (int)color.b) > 381);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
