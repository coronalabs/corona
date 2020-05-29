//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

