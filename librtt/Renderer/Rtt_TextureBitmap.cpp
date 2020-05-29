//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
