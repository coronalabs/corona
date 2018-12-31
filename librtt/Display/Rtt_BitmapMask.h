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

#ifndef __Rtt_BitmapMask__
#define __Rtt_BitmapMask__

#include "Core/Rtt_String.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class FilePath;
class BitmapPaint;
class UserdataWrapper;

// ----------------------------------------------------------------------------

class BitmapMask
{
	public:
		static BitmapMask* Create( Runtime& runtime, const FilePath& maskData );

	public:
		BitmapMask( BitmapPaint *paint );
		BitmapMask( BitmapPaint *paint, Real contentW, Real contentH );

	public:
		~BitmapMask();

	public:
		const BitmapPaint* GetPaint() const { return fPaint; }
		BitmapPaint* GetPaint() { return fPaint; }

	public:
		const Transform& GetTransform() const { return fTransform; }
		Transform& GetTransform() { return fTransform; }

	public:
		void GetSelfBounds( Rect& rect ) const;
		void GetSelfBounds( Real& width, Real& height ) const;
		void SetSelfBounds( Real width, Real height );

	public:
		bool HitTest( Rtt_Allocator *allocator, int i, int j ) const;

	private:
		BitmapPaint* fPaint;
		Transform fTransform;
		Real fContentWidth;
		Real fContentHeight;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_BitmapMask__
