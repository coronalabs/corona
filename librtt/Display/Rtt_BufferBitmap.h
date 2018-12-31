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

#ifndef _Rtt_BufferBitmap_H__
#define _Rtt_BufferBitmap_H__

#include "Core/Rtt_Types.h"

#include "Display/Rtt_PlatformBitmap.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// TODO: Does TextureBitmap replace the need for this???
// 
class BufferBitmap : public PlatformBitmap
{
	public:
		typedef PlatformBitmap Super;

	public:
		BufferBitmap( Rtt_Allocator* allocator, size_t w, size_t h, Super::Format format, Orientation orientation = kUp );
		BufferBitmap( Rtt_Allocator* allocator, size_t w, size_t h, Super::Format format, Real angle );
		virtual ~BufferBitmap();

	protected:
		void ReleaseBits();

	public:
		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual Super::Format GetFormat() const;
		virtual U32 UprightWidth() const;
		virtual U32 UprightHeight() const;
		virtual bool IsProperty( PropertyMask mask ) const;
		virtual void SetProperty( PropertyMask mask, bool newValue );
		virtual Orientation GetOrientation() const;

	public:
		const void *ReadAccess() { return fData; }
		void* WriteAccess() { return fData; }
		void Flip( bool flipHorizontally, bool flipVertically );
		void UndoPremultipliedAlpha();

	private:
		mutable void* fData;
		U32 fWidth;
		U32 fHeight;
		U8 fProperties;
		U8 fFormat;
		S8 fOrientation;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_BufferBitmap_H__
