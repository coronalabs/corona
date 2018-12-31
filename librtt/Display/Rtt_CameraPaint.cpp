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

#include "Display/Rtt_CameraPaint.h"

#include "Display/Rtt_Display.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Renderer/Rtt_VideoTexture.h"
#include "Rtt_Transform.h"

//#include <stdio.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

CameraPaint::CameraPaint(const SharedPtr< TextureResource >& resource)
:	Super(resource)
{
	Super::Initialize( kCamera );

	Invalidate( Paint::kTextureTransformFlag );
}

const Paint*
CameraPaint::AsPaint( Super::Type type ) const
{
	return ( kCamera == type ? this : NULL );
}

// const MLuaUserdataAdapter&
// CameraPaint::GetAdapter() const
// {
// 	return CameraPaintAdapter::Constant();
// }

void
CameraPaint::UpdateTransform( Transform& t ) const
{
	t.Rotate( Rtt_IntToReal( 90 ) );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

