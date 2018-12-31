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

#include "Renderer/Rtt_TextureVolatile.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

TextureVolatile::TextureVolatile(
	Rtt_Allocator* allocator,
	U32 width, U32 height, Format format, Filter filter, Wrap wrapX, Wrap wrapY )
:	Super( allocator ),
	fWidth( width ),
	fHeight( height ),
	fFormat( format ),
	fFilter( filter ),
	fWrapX( wrapX ),
	fWrapY( wrapY )
{
}

U32 
TextureVolatile::GetWidth() const
{
	return fWidth;
}

U32 
TextureVolatile::GetHeight() const
{
	return fHeight;
}

Texture::Format
TextureVolatile::GetFormat() const
{
	return fFormat;
}

Texture::Filter
TextureVolatile::GetFilter() const
{
	return fFilter;
}

Texture::Wrap
TextureVolatile::GetWrapX() const
{
	return fWrapX;
}

Texture::Wrap
TextureVolatile::GetWrapY() const
{
	return fWrapY;
}

void
TextureVolatile::SetFilter( Filter newValue )
{
	fFilter = newValue;
	Invalidate();
}

void
TextureVolatile::SetWrapX( Wrap newValue )
{
	fWrapX = newValue;
	Invalidate();
}

void
TextureVolatile::SetWrapY( Wrap newValue )
{
	fWrapY = newValue;
	Invalidate();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
