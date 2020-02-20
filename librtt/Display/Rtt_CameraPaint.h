//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CameraPaint_H__
#define _Rtt_CameraPaint_H__

#include "Display/Rtt_Paint.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Display;

// ----------------------------------------------------------------------------

class CameraPaint : public Paint
{
	public:
		typedef Paint Super;

		CameraPaint(const SharedPtr< TextureResource >& resource);

	public:
		virtual const Paint* AsPaint( Super::Type type ) const;
///		virtual const MLuaUserdataAdapter& GetAdapter() const;
		virtual void UpdateTransform( Transform& t ) const;

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CameraPaint_H__
