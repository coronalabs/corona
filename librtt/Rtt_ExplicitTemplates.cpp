//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#if !defined( Rtt_IPHONE_ENV )

#include "Core/Rtt_Build.h"

#include "Core/Rtt_Array.h"
#include "Core/Rtt_Geometry.h"
#include "Core/Rtt_ResourceHandle.h"
#include "Display/Rtt_DisplayObject.h"
#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

template class Array< DisplayObject* >;
template class PtrArray< DisplayObject >;
template class LightPtrArray< DisplayObject >;
template class Array< Vertex2 >;
template class Array< float >;
template class Array< S32 >;
template class Array< U32 >;
template class ResourceHandle< lua_State >;
template class ResourceHandleOwner< lua_State >;

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif
