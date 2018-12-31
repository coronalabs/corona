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

#include "Renderer/Rtt_TextureBitmap.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

TextureBitmap::TextureBitmap( Rtt_Allocator* allocator, U32 width, U32 height, Format format, Filter filter)
:	Super( allocator ), fWidth( width ), fHeight( height ), fFormat( format ), fFilter( filter ), fData( NULL )
{
	Allocate();
}

TextureBitmap::~TextureBitmap()
{
	Deallocate();
}

void 
TextureBitmap::Allocate()
{
	if ( ! fData )
	{
		fData = new U8[GetSizeInBytes()];
	}
}

void 
TextureBitmap::Deallocate()
{
	delete [] fData;
	fData = NULL;
}

U32 
TextureBitmap::GetWidth() const
{
	return fWidth;
}

U32 
TextureBitmap::GetHeight() const
{
	return fHeight;
}

TextureBitmap::Format
TextureBitmap::GetFormat() const
{
	return fFormat;
}

TextureBitmap::Filter
TextureBitmap::GetFilter() const
{
	return fFilter;
}

const U8*
TextureBitmap::GetData() const
{
	return fData;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
