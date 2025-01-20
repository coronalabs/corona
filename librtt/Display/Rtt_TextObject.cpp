//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_TextObject.h"

#include "Core/Rtt_String.h"
#include "Display/Rtt_BitmapMask.h"
#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_RectPath.h"
#include "Display/Rtt_Shader.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Uniform.h"
#include "Rtt_GroupObject.h"
#include "Rtt_LuaProxyVTable.h"
#include "Display/Rtt_PlatformBitmap.h"
#include "Rtt_PlatformFont.h"
#include "Rtt_Runtime.h"
#include "Rtt_Profiling.h"

#ifdef Rtt_WIN_PHONE_ENV
#	include <vector>
#endif

#ifdef Rtt_WIN_ENV
#	undef CreateFont
#endif

// [Experimental Feature]
// If the below is defined, text objects will be offsetted and rendered to the nearest screen pixel.
// This prevents the text from looking blurry/fuzzy if the text bitmap lands between screen pixels,
// causing the GPU to interpolate the text's bitmap pixels between screen pixels.
// Notes:
// - This issue is only noticeable on screens with a low DPI, which is typically the case on desktop platforms.
// - This feature won't prevent scaled or rotated text objects from looking blurry.
// - Currently only defined on Win32 to minimize any unforseen negative impacts this change might cause.
// - Theoretically, GL_NEAREST should do this for us, but it doesn't work with Corona's masks. (Why?)
#if defined(Rtt_WIN_DESKTOP_ENV)
#	define Rtt_RENDER_TEXT_TO_NEAREST_PIXEL
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void
TextObject::Unload( DisplayObject& parent )
{
	// NOTE: Assumes no subclasses of TextObject exist
	if ( & parent.ProxyVTable() == & LuaTextObjectProxyVTable::Constant() )
	{
		TextObject& t = (TextObject&)parent;
		t.Reset();
	}

	// Tail recursion
	GroupObject *group = parent.AsGroupObject();
	if ( group )
	{
		for ( S32 i = 0, iMax = group->NumChildren(); i < iMax; i++ )
		{
			Unload( group->ChildAt( i ) );
		}
	}
}

void
TextObject::Reload( DisplayObject& parent )
{
	// NOTE: Assumes no subclasses of TextObject exist
	if ( & parent.ProxyVTable() == & LuaTextObjectProxyVTable::Constant() )
	{
		TextObject& t = (TextObject&)parent;
		t.Initialize();
	}

	// Tail recursion
	GroupObject *group = parent.AsGroupObject();
	if ( group )
	{
		for ( S32 i = 0, iMax = group->NumChildren(); i < iMax; i++ )
		{
			Reload( group->ChildAt( i ) );
		}
	}
}

// ------------------------------------------------------------------------------------------------------------------------
// The following code only applies to Windows Phone 8.0.
// It is needed because Windows Phone 8.0 can only render text to bitmap while Corona is not synchonrized/blocking
// the Direct3D thread or else deadlock will occur. So, Corona must create the text bitmap before rendering.
// ------------------------------------------------------------------------------------------------------------------------
#ifdef Rtt_WIN_PHONE_ENV
typedef std::vector<TextObject*> TextObjectCollection;

/// Gets a collection used to store all TextObjects that exist for all runtime instances.
/// A TextObject constructor is expected to add an instance of itself to this collection and remove itself upon destruction.
static TextObjectCollection& GetCollection()
{
	static TextObjectCollection sCollection;
	return sCollection;
}

bool
TextObject::IsUpdateNeededFor(Display& display)
{
	TextObjectCollection& collection = GetCollection();
	TextObjectCollection::iterator iter;
	TextObject* textObjectPointer;

	for (iter = collection.begin(); iter != collection.end(); iter++)
	{
		textObjectPointer = *iter;
		if (textObjectPointer && (&display == &(textObjectPointer->fDisplay)))
		{
			if (textObjectPointer->IsInitialized() == false)
			{
				return true;
			}
		}
	}
	return false;
}

void
TextObject::UpdateAllBelongingTo(Display& display)
{
	TextObjectCollection& collection = GetCollection();
	TextObjectCollection::iterator iter;
	TextObject* textObjectPointer;

	for (iter = collection.begin(); iter != collection.end(); iter++)
	{
		textObjectPointer = *iter;
		if (textObjectPointer && (&display == &(textObjectPointer->fDisplay)))
		{
			if (textObjectPointer->IsInitialized() == false)
			{
				textObjectPointer->Initialize();
			}
		}
	}
}

#endif

// w,h are in content coordinates
// font's size is in pixel coordinates
TextObject::TextObject( Display& display, const char text[], PlatformFont *font, Real w, Real h, const char alignment[] )
:	Super( RectPath::NewRect( display.GetRuntime().GetAllocator(), w, h ) ),
	fDisplay( display ),
	fText( display.GetRuntime().GetAllocator() ),
	fOriginalFont( font ),
	fScaledFont( NULL ),
	fWidth( w ),
	fHeight( h ),
	fAlignment( display.GetRuntime().GetAllocator() ),
	fGeometry( NULL ),
	fBaselineOffset( Rtt_REAL_0 ),
	fMaskUniform( Rtt_NEW( display.GetAllocator(), Uniform( display.GetAllocator(), Uniform::kMat3 ) ) )
{
	if ( ! fOriginalFont )
	{
		const MPlatform& platform = display.GetRuntime().Platform();
		Real standardFontSize = platform.GetStandardFontSize() * display.GetSxUpright();
		fOriginalFont = platform.CreateFont( PlatformFont::kSystemFont, standardFontSize );
	}

	SetText( text );
	SetAlignment(alignment);

	Invalidate( kMaskFlag );
	Initialize();
	SetHitTestMasked(false);

#ifdef Rtt_WIN_PHONE_ENV
	GetCollection().push_back(this);
#endif

    SetObjectDesc( "TextObject" );
}

TextObject::~TextObject()
{
	Rtt_DELETE( fOriginalFont );
	Rtt_DELETE( fScaledFont );

	QueueRelease( fMaskUniform );
	QueueRelease( fGeometry );

#ifdef Rtt_WIN_PHONE_ENV
	TextObjectCollection& collection = GetCollection();
	TextObjectCollection::iterator iter;
	for (iter = collection.begin(); iter != collection.end(); iter++)
	{
		if (this == *iter)
		{
			collection.erase(iter);
			break;
		}
	}
#endif
}

bool
TextObject::Initialize()
{
	Rtt_ASSERT( fOriginalFont );

	// Fetch the content scales.
	Real sx = fDisplay.GetSxUpright();
	Real sy = fDisplay.GetSyUpright();
	bool shouldScale = ! Rtt_RealIsOne( sx ) || ! Rtt_RealIsOne( sy );

	// If the rendering system has been scaled, then use a font with a scaled font size.
	UpdateScaledFont();
	PlatformFont *font = fScaledFont ? fScaledFont : fOriginalFont;

	// For text boxes (multiline), use pixel dimensions
	Real pixelW = fWidth;
	Real pixelH = fHeight;
	bool isTextBox = ( pixelW > Rtt_REAL_0 );
	if ( shouldScale && isTextBox )
	{
		pixelW = Rtt_RealDiv( pixelW, sx );
		pixelH = Rtt_RealDiv( pixelH, sy );
	}

	// Get the text to be rendered.
	// Note: If no text was assigned, then use a string with just a space so that we can generate
	//       a bitmap at about the same height as a bitmap with text. This is especially needed
	//       when this text object is initially created or else its center reference point will
	//       be at the same position as the top-left reference point, which might not be expected.
	const char *text = " ";
	if (fText.IsEmpty() == false)
	{
		text = fText.GetString();
	}

	// TODO: We are handling two cases here. Can we separate more cleanly?
	// We need to request an appropriate sized text bitmap (with proper pixel resolution)
	// (1) Single-line: we already scaled the font size (fWidth/fHeight should be 0)
	// (2) Multi-line: fWidth/fHeight define the box and needs to be scaled
	BitmapPaint *paint = BitmapPaint::NewBitmap( fDisplay.GetRuntime(), text, *font, pixelW, pixelH, fAlignment.GetString(), fBaselineOffset );
	PlatformBitmap *bitmap = paint->GetBitmap(); Rtt_ASSERT( bitmap );

	Real contentW = Rtt_IntToReal( bitmap->Width() );
	Real contentH = Rtt_IntToReal( bitmap->Height() );

	// bitmap returns dimensions in pixels, so convert to content coords
	if ( shouldScale )
	{
		contentW = Rtt_RealMul( contentW, sx );
		contentH = Rtt_RealMul( contentH, sy );
		fBaselineOffset = Rtt_RealMul( fBaselineOffset , sy );
	}

	if ( isTextBox )
	{
		// Apple (Mac/iOS) weirdness. Need to round up to nearest multiple of 4.
		Rtt_ASSERT( contentW <= (fWidth+4) );
		Rtt_ASSERT( Rtt_RealIsZero( fHeight ) || contentH <= (fHeight+4) );
	}

	BitmapMask *mask =
		Rtt_NEW( fDisplay.GetRuntime().GetAllocator(), BitmapMask( paint, contentW, contentH ) );
	SetMask( fDisplay.GetRuntime().GetAllocator(), mask );
	SetSelfBounds( contentW, contentH );

	return ( NULL != mask );
}

/// Updates member variable "fScaledFont" with a scaled font size based on "fOriginalFont".
/// Should be called every time the rendering system's scale factor has changed.
/// Member variable "fScaledFont" will be set to NULL if the scale factor is 1.0.
void
TextObject::UpdateScaledFont()
{
	// Fetch the content scaling factor from the rendering system.
	// Note: There is a bug in the Corona Simulator where the scales are wrongly swapped when rotating
	//       to an orientation that the app does not support. The below code works-around this issue.
	Real scale = fDisplay.GetSxUpright();

	// Scale the font's point size.
	Real fontSizeEpsilon = Rtt_FloatToReal( 0.1f );
	Real scaledFontSize = Rtt_RealDiv( fOriginalFont->Size(), scale );

	// Create a scaled font, if necessary.
	if ((scaledFontSize >= (fOriginalFont->Size() + fontSizeEpsilon)) ||
	    (scaledFontSize <= (fOriginalFont->Size() - fontSizeEpsilon)))
	{
		// Font needs to be scaled. Create a new scaled font, if not already made.
		if (fScaledFont)
		{
			if ((scaledFontSize >= (fScaledFont->Size() + fontSizeEpsilon)) ||
			    (scaledFontSize <= (fScaledFont->Size() - fontSizeEpsilon)))
			{
				Reset();
			}
		}
		if (!fScaledFont)
		{
			Reset();
			fScaledFont = fOriginalFont->CloneUsing(fDisplay.GetRuntime().GetAllocator());
			if (fScaledFont)
			{
				fScaledFont->SetSize(scaledFontSize);
			}
		}
	}
	else if (fScaledFont)
	{
		// Font does not need to be scaled. Delete the old scaled font.
		Reset();
	}
}

void
TextObject::Reset()
{
	SetMask( NULL, NULL );

	Rtt_DELETE( fScaledFont );
	fScaledFont = NULL;
	
	Invalidate( kGeometryFlag | kStageBoundsFlag | kMaskFlag );
	GetPath().Invalidate( ClosedPath::kFillSource | ClosedPath::kStrokeSource );
}

bool
TextObject::UpdateTransform( const Matrix& parentToDstSpace )
{
	SUMMED_TIMING( tut, "Text: UpdateTransform" );

	bool result = false;
	
	// First, attempt to scale the font, if necessary.
	// If the font does not need to be scaled, then this function will flag this object to be re-initialized.
	UpdateScaledFont();
	
	// Update the text object.
	if ( IsInitialized() || Rtt_VERIFY( Initialize() ) )
	{
		// Update this object's transformation matrix.
		result = Super::UpdateTransform( parentToDstSpace );
	}
	
	return result;
}

void
TextObject::GetSelfBounds( Rect& rect ) const
{
	if ( ! IsInitialized() )
	{
		const_cast< TextObject* >( this )->Initialize();
	}

	if ( Rtt_VERIFY( IsInitialized() ) )
	{
		// NOTE: we do not use fWidth/fHeight b/c that's only used for multiline text
		// The final bounds might be slightly larger for byte alignment/pixel row stride issues
		Super::GetSelfBounds( rect );
	}
	else
	{
		rect.SetEmpty();
	}
}

void
TextObject::Prepare( const Display& display )
{
#ifdef Rtt_RENDER_TEXT_TO_NEAREST_PIXEL
	Real offsetX = Rtt_REAL_0;
	Real offsetY = Rtt_REAL_0;

	// The base display object must be prepared first.
	// This generates/updates its text bitmap and updates its transformation matrix.
	Super::Prepare( display );

	SUMMED_TIMING( tp, "Text: post-Super::Prepare" );

	if ( IsInitialized() == false )
	{
		return;
	}

	// Fetch this text object's geometry/vertices based on the bitmap generated above.
	const Geometry *geometryPointer = GetFillData().fGeometry;
	if ( !geometryPointer )
	{
		return;
	}

	// Calculate offsets so that this object's bitmap pixels line-up with the closest screen pixels.
	// This prevents the bitmap from appearing blurry/fuzzy onscreen. (Avoids interpolation between screen pixels.)
	Rect bounds;
	ClosedPath& path = GetPath();
	path.GetSelfBounds( bounds );
	Matrix translationMatrix( GetSrcToDstMatrix() );
	{
		offsetX = translationMatrix.Tx() + Rtt_RealMul( bounds.Width(), GetAnchorX() - Rtt_REAL_HALF );
		offsetX -= Rtt_RealMul( bounds.Width(), GetAnchorX() );
		offsetX += display.GetXOriginOffset();
		offsetX = Rtt_RealDiv( offsetX, fDisplay.GetSx() );                           // Convert to pixels.
		offsetX = Rtt_FloatToReal( floorf( Rtt_RealToFloat( offsetX ) ) ) - offsetX;  // Round-down to nearest pixel.
		offsetX = Rtt_RealMul( offsetX, fDisplay.GetSx() );                           // Convert to content coordinates.
	}
	{
		offsetY = translationMatrix.Ty() + Rtt_RealMul( bounds.Height(), GetAnchorY() - Rtt_REAL_HALF );
		offsetY -= Rtt_RealMul( bounds.Height(), GetAnchorY() );
		offsetY += display.GetYOriginOffset();
		offsetY = Rtt_RealDiv( offsetY, fDisplay.GetSy() );                           // Convert to pixels.
		offsetY = Rtt_FloatToReal( floorf( Rtt_RealToFloat( offsetY ) ) ) - offsetY;  // Round-down to nearest pixel.
		offsetY = Rtt_RealMul( offsetY, fDisplay.GetSy() );                           // Convert to content coordinates.
	}

	// Apply the above offsets to this object's temporary geometry and mask uniform, to be used by Draw() method later.
	// Note: Don't apply to base class' geometry. We do not want this object's (x,y) properties to change in Lua.
	{
		// Offset uniform to nearest screen pixel.
		translationMatrix.Translate( offsetX, offsetY );
		UpdateMaskUniform( *fMaskUniform, translationMatrix, *GetMask() );
	}
	{
		// Offset geometry to nearest screen pixel.
		//TODO: Avoid memory allocation.
		QueueRelease( fGeometry );
		fGeometry = Rtt_NEW( display.GetAllocator(), Geometry( *geometryPointer ) );
		for ( int index = (int)fGeometry->GetVerticesUsed() - 1; index >= 0; index-- )
		{
			Geometry::Vertex *vertexPointer = &(fGeometry->GetVertexData()[index]);
			vertexPointer->x += offsetX;
			vertexPointer->y += offsetY;
		}
	}
#else
	Super::Prepare( display );
#endif
}

void
TextObject::Draw( Renderer& renderer ) const
{
#ifdef Rtt_RENDER_TEXT_TO_NEAREST_PIXEL
	if ( ShouldDraw() )
	{
		SUMMED_TIMING( td, "Text: Draw" );

		RenderData fillData = GetFillData();
		fillData.fGeometry = fGeometry;
		fillData.fMaskUniform = fMaskUniform;
		GetFillShader()->Draw( renderer, fillData );
	}
#else
	Super::Draw( renderer );
#endif
}

const LuaProxyVTable&
TextObject::ProxyVTable() const
{
	return LuaTextObjectProxyVTable::Constant();
}

/*
void
TextObject::SetColor( Paint* newValue )
{
	if ( fShape )
	{
		fShape->SetFill( newValue );
		InvalidateDisplay();
	}
}
*/

void
TextObject::SetText( const char* newValue )
{
	const char kEmptyString[] = "";
	if ( ! newValue )
	{
		newValue = kEmptyString;
	}

	if ( 0 != Rtt_StringCompare( fText.GetString(), newValue ) ) 
	{
		fText.Set( newValue );
		Reset();
	}
}

void
TextObject::SetSize( Real newValue )
{
	if ( fOriginalFont )
	{
		// If the given font size is invalid, then use the system's default font size.
		if ( newValue < Rtt_REAL_1 )
		{
			newValue = fDisplay.GetRuntime().Platform().GetStandardFontSize() * fDisplay.GetSxUpright();
		}

		// Update the font and text bitmap, but only if the font size has changed.
		if ( GetSize() != newValue )
		{
			fOriginalFont->SetSize( newValue );
			Reset();
		}
	}
}

Real
TextObject::GetSize() const
{
	Rtt_ASSERT( fOriginalFont );
	return fOriginalFont->Size();
}

void
TextObject::SetFont( PlatformFont *newValue )
{
	if ( newValue && fOriginalFont != newValue )
	{
		Rtt_DELETE( fOriginalFont );
		fOriginalFont = newValue;
		Reset();
	}
}
void
TextObject::SetAlignment( const char* newValue )
{
	if ( newValue )
	{
		if ( 0 != Rtt_StringCompare( fAlignment.GetString(), newValue ) )
		{
			fAlignment.Set( newValue );
			Reset();
		}
	}
}
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

