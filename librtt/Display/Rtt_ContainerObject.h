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

#ifndef _Rtt_ContainerObject_H__
#define _Rtt_ContainerObject_H__

#include "Display/Rtt_GroupObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class BitmapMask;
class Display;
class MCachedResourceLibrary;
class Uniform;

// ----------------------------------------------------------------------------

class ContainerObject : public GroupObject
{
	Rtt_CLASS_NO_COPIES( ContainerObject )

	public:
		typedef GroupObject Super;
		typedef ContainerObject Self;

	public:
		ContainerObject(
			Rtt_Allocator* pAllocator, StageObject *canvas, Real width, Real height );
		virtual ~ContainerObject();

	public:
		void Initialize( Display& display );

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;
		virtual void GetSelfBoundsForAnchor( Rect& rect ) const;

	public:
		virtual bool HitTest( Real contentX, Real contentY );
		virtual bool CanCull() const;

	public:
		virtual void SetSelfBounds( Real width, Real height );

	protected:
		bool ShouldOffsetClip() const;

	private:
		BitmapMask *fContainerMask;
		Uniform *fContainerMaskUniform;

		Real fWidth;
		Real fHeight;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GroupObject_H__
