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


#ifndef _Rtt_AndroidBitmap_H__
#define _Rtt_AndroidBitmap_H__

#include "librtt/Display/Rtt_PlatformBitmap.h"
#include "Core/Rtt_Array.h"
#include "Core/Rtt_String.h"
#include "Rtt_AndroidFont.h"
#include "AndroidImageData.h"
#include "AndroidImageDecoder.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{
class PlatformFont;

// ----------------------------------------------------------------------------

class AndroidBitmap : public PlatformBitmap
{
	public:
		AndroidBitmap( Rtt_Allocator & context );
		virtual ~AndroidBitmap();

		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual Real GetScale() const;
		virtual Format GetFormat() const;
		virtual PlatformBitmap::Orientation GetOrientation() const;
		
	protected:
		mutable AndroidImageData fImageData;
};

class AndroidAssetBitmap : public AndroidBitmap
{
	public:
		typedef AndroidBitmap Super;

		AndroidAssetBitmap( Rtt_Allocator& context, const char *filePath, NativeToJavaBridge *ntjb );
		virtual ~AndroidAssetBitmap();

		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual bool IsProperty( PropertyMask mask ) const;
		virtual void SetProperty( PropertyMask mask, bool newValue );
		virtual U32 UprightWidth() const;
		virtual U32 UprightHeight() const;
		virtual const void * GetBits( Rtt_Allocator * context ) const;

	protected:
		AndroidImageDecoder& ImageDecoder();
		U32 SourceWidth() const;
		U32 SourceHeight() const;
		Rtt_INLINE bool IsPropertyInternal( PropertyMask mask ) const { return (fProperties & mask) ? true : false; }

	private:
		U8 fProperties;
		String fPath;
		mutable AndroidImageDecoder fImageDecoder;
};

class AndroidMaskAssetBitmap : public AndroidAssetBitmap
{
	public:
		typedef AndroidAssetBitmap Super;

		AndroidMaskAssetBitmap( Rtt_Allocator& context, const char *filePath, NativeToJavaBridge *ntjb );

		virtual Format GetFormat() const;
};

class AndroidTextBitmap : public AndroidBitmap
{
	public:
		typedef AndroidBitmap Super;

		AndroidTextBitmap( Rtt_Allocator & context, NativeToJavaBridge *ntjb, const char str[], const PlatformFont& font, int width, int height, const char alignment[], Real & baselineOffset );
		virtual ~AndroidTextBitmap();

		virtual const void * GetBits( Rtt_Allocator * context ) const;
		virtual PlatformBitmap::Format GetFormat() const;
		
	private:
		AndroidFont fFont;
		String fText;
		int fWrapWidth;
		int fClipWidth;
		int fClipHeight;
		mutable Real fBaselineOffset;
		String fAlignment;
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidBitmap_H__
