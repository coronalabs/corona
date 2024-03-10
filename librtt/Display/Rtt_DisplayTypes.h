//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_DisplayTypes_H__
#define _Rtt_DisplayTypes_H__

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Array.h"
#include "Core/Rtt_Geometry.h"
#include "Core/Rtt_Traits.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class DisplayObject;

// ----------------------------------------------------------------------------

Rtt_TRAIT_SPECIALIZE( IsBitCopyable, Vertex2, true )

typedef PtrArray< DisplayObject >		PtrArrayDisplayObject;
typedef Array< float >                  ArrayFloat;
typedef Array< Vertex2 >				ArrayVertex2;
typedef Array< S32 >					ArrayS32;
typedef Array< U16 >                    ArrayIndex;
typedef Array< U32 >					ArrayU32;

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_DisplayTypes_H__
