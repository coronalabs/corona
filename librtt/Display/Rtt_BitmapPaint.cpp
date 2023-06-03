//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_BitmapPaint.h"

#include "Core/Rtt_Geometry.h"
#include "Rtt_FilePath.h"
#include "Display/Rtt_BitmapMask.h"
#include "Display/Rtt_BitmapPaintAdapter.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Renderer/Rtt_RenderData.h"
#include "Rtt_Runtime.h"
#include "Display/Rtt_TextureFactory.h"
#include "Display/Rtt_TextureResource.h"

#include <stdio.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/*
SharedPtr< TextureResource >
BitmapPaint::NewTextureResource( Runtime& runtime, const char* filename, MPlatform::Directory baseDir, U32 flags, bool isMask )
{
	TextureFactory& factory = runtime.GetDisplay().GetTextureFactory();
	return factory.FindOrCreate( filename, baseDir, flags, isMask );
}
*/

BitmapPaint*
BitmapPaint::NewBitmap( Runtime& runtime, const char* filename, MPlatform::Directory baseDir, U32 flags )
{
	BitmapPaint *result = NULL;

	TextureFactory& factory = runtime.GetDisplay().GetTextureFactory();
	SharedPtr< TextureResource > pTexture =
		factory.FindOrCreate( filename, baseDir, flags, false );

	if ( pTexture.NotNull() )
	{
		if ( pTexture->GetBitmap() == NULL || !pTexture->GetBitmap()->IsMask() )
		{
			result = Rtt_NEW( runtime.Allocator(), BitmapPaint( pTexture ) ); Rtt_ASSERT( result );
		}
		else
		{
			Rtt_TRACE_SIM( ( "ERROR: The file (%s) has already been loaded as a mask, "
							 "so it cannot be used as an image display object.\n", filename ) );
		}
	}

	return result;
}

BitmapPaint*
BitmapPaint::NewBitmap( Runtime& runtime, const FilePath& data, U32 flags, bool isMask, bool onlyForHitTests )
{
	BitmapPaint *result = NULL;

	const char *filename = data.GetFilename();
	MPlatform::Directory baseDir = data.GetBaseDir();
	
	if ( onlyForHitTests && '\0' == *filename )
	{
		return NULL;
	}

	TextureFactory& factory = runtime.GetDisplay().GetTextureFactory();
	SharedPtr< TextureResource > pTexture =
		factory.FindOrCreate( filename, baseDir, flags, isMask, onlyForHitTests );

	if ( pTexture.NotNull() )
	{
		// Always create bitmap
		// Except when we want a mask but the texture isn't a mask
		bool shouldCreate = !isMask || pTexture->GetBitmap() == NULL || pTexture->GetBitmap()->IsMask();
		if ( shouldCreate )
		{
			result = Rtt_NEW( runtime.Allocator(), BitmapPaint( pTexture ) ); Rtt_ASSERT( result );
		}
		else
		{
			// Mask-specific error
			if ( isMask  )
			{
				Rtt_TRACE_SIM( ( "ERROR: The file (%s) has already been loaded as an image display object, so it cannot be used as a mask.\n", filename ) );
			}
		}
	}
	else
	{
		Rtt_TRACE_SIM( ( "ERROR: The bitmap file (%s) could not be loaded\n", filename ) );
	}

	return result;
}

BitmapPaint*
BitmapPaint::NewBitmap( TextureFactory& factory, PlatformBitmap* pBitmap, bool sharedTexture )
{
	BitmapPaint* result = NULL;

	SharedPtr< TextureResource > resource = factory.FindOrCreate( pBitmap, sharedTexture );

	result = Rtt_NEW( factory.GetDisplay().GetAllocator(), BitmapPaint( resource ) );
	Rtt_ASSERT( result );

	return result;
}

BitmapPaint*
BitmapPaint::NewBitmap( Runtime& runtime, const char text[], const PlatformFont& font, Real w, Real h, const char alignment[], Real& baselineOffset )
{
	BitmapPaint* result = NULL;

	TextureFactory& factory = runtime.GetDisplay().GetTextureFactory();
	SharedPtr< TextureResource > pTexture = factory.Create( text, font, w, h, alignment, baselineOffset );

	Rtt_ASSERT( pTexture.NotNull() );
	result = Rtt_NEW( runtime.Allocator(), BitmapPaint( pTexture ) ); Rtt_ASSERT( result );

	return result;
}

// ----------------------------------------------------------------------------

BitmapPaint::BitmapPaint( const SharedPtr< TextureResource >& resource )
:	Super(resource),
	fTransform()
{
	Initialize( kBitmap );
}

S32
BitmapPaint::DegreesToUpright() const
{
	PlatformBitmap *bitmap = GetBitmap();
	
	S32 angle = ( bitmap ? bitmap->DegreesToUprightBits() : 0 );
	Rtt_ASSERT( Abs( angle ) <= 180 );

	return angle;
}

const Paint*
BitmapPaint::AsPaint( Super::Type type ) const
{
	return ( Super::kBitmap == type ? this : NULL );
}

const MLuaUserdataAdapter&
BitmapPaint::GetAdapter() const
{
	return BitmapPaintAdapter::Constant();
}

void
BitmapPaint::UpdateTransform( Transform& t ) const
{
	t = GetTransform();
}

PlatformBitmap *
BitmapPaint::GetBitmap() const
{
	Rtt_ASSERT( fResource.NotNull() );
	return fResource->GetBitmap();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

