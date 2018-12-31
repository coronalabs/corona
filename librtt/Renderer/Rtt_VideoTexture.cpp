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

#include "Renderer/Rtt_VideoTexture.h"

#include "Core/Rtt_Assert.h"

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

VideoTexture::VideoTexture( Rtt_Allocator* allocator )
:	Super( allocator ),
	fSource( kCamera )
{
}

VideoSource
VideoTexture::GetSource() const
{
	return fSource;
}

void
VideoTexture::SetSource( VideoSource source )
{
	fSource = source;
	Invalidate();
}
		
CPUResource::ResourceType 
VideoTexture::GetType() const
{
	return CPUResource::kVideoTexture;
}
		
U32 
VideoTexture::GetWidth() const
{
	return 100;
}

U32 
VideoTexture::GetHeight() const
{
	return 100;
}

Texture::Format
VideoTexture::GetFormat() const
{
	return Super::kLuminance; //Super::kRGBA;
}

Texture::Filter
VideoTexture::GetFilter() const
{
	return Super::kLinear;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
