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

#ifndef _Rtt_RectObject_H__
#define _Rtt_RectObject_H__

#include "Display/Rtt_ShapeObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class RectPath;

// ----------------------------------------------------------------------------

// The RectObject is the base representation of all closed rects
class RectObject : public ShapeObject
{
	Rtt_CLASS_NO_COPIES( RectObject )

	public:
		typedef ShapeObject Super;
		typedef RectObject Self;

	public:
		static RectObject *NewRect( Rtt_Allocator*, Real width, Real height );
		virtual void GetSelfBoundsForAnchor( Rect& rect ) const;
		virtual ShaderResource::ProgramMod GetProgramMod() const;

	protected:
		RectObject( RectPath *path );

	protected:
		RectPath& GetRectPath()
		{
			return const_cast< RectPath& >(
				((const Self *)this)->GetRectPath() );
		}
		const RectPath& GetRectPath() const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_RectObject_H__
