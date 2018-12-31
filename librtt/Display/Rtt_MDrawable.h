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

#ifndef _Rtt_MDrawable_H__
#define _Rtt_MDrawable_H__

#include "Core/Rtt_Real.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Display;
class Renderer;
class Matrix;
struct Rect;

// ----------------------------------------------------------------------------

class MDrawable
{
	public:
		enum
		{
			kDefaultAlpha = 0xFF
		};

	public:
		// Returns true if srcToDst transform is updated
		virtual bool UpdateTransform( const Matrix& parentToDstSpace ) = 0;
		virtual void Prepare( const Display& display ) = 0;
		virtual void Translate( Real deltaX, Real deltaY ) = 0;
		virtual void Draw( Renderer& renderer ) const = 0;
		virtual void GetSelfBounds( Rect& rect ) const = 0;
		virtual void GetSelfBoundsForAnchor( Rect& rect ) const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MDrawable_H__
