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

#if 0

#ifndef __Rtt_ImageGroupObject__
#define __Rtt_ImageGroupObject__

#include "Rtt_GroupObject.h"

#include "Core/Rtt_AutoPtr.h"
#include "Rtt_VertexArray.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class BitmapPaint;
class ImageSheet;

// ----------------------------------------------------------------------------

class ImageGroupObject : public GroupObject
{
	public:
		typedef GroupObject Super;
		typedef ImageGroupObject Self;

	public:
		static bool IsCompatibleParent(
			CompositeObject *parent, const AutoPtr< ImageSheet >& sheet );

	public:
		ImageGroupObject(
			Rtt_Allocator *pAllocator,
			StageObject *canvas,
			const AutoPtr< ImageSheet >& sheet );
		virtual ~ImageGroupObject();

	public:
		// MDrawable
		virtual void Build();
		virtual void Translate( Real dx, Real dy );
		virtual void Draw( RenderingStream& rStream ) const;

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

	protected:
		virtual void DidInsert( bool childParentChanged );
		virtual void DidRemove();

	public:
		const AutoPtr< ImageSheet >& GetSheet() const { return fSheet; }

	private:
		AutoPtr< ImageSheet > fSheet;
		BitmapPaint *fPaint;
		VertexArray fArray;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ImageGroupObject__

#endif