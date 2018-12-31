//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
BitmapPaint::NewBitmap( Runtime& runtime, const FilePath& data, U32 flags, bool isMask )
{
	BitmapPaint *result = NULL;

	const char *filename = data.GetFilename();
	MPlatform::Directory baseDir = data.GetBaseDir();

	TextureFactory& factory = runtime.GetDisplay().GetTextureFactory();
	SharedPtr< TextureResource > pTexture =
		factory.FindOrCreate( filename, baseDir, flags, isMask );

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

