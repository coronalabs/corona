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

#pragma once

#include "Core\Rtt_Build.h"
#include "Core\Rtt_String.h"
#include "Display\Rtt_PlatformBitmap.h"


#pragma region Forward Declarations
namespace Gdiplus
{
	class Bitmap;
	class BitmapData;
}
namespace Interop
{
	namespace Graphics
	{
		class HorizontalAlignment;
	}
	class RuntimeEnvironment;
}
namespace Rtt
{
	class WinFont;
}

#pragma endregion


namespace Rtt
{

class WinBitmap : public PlatformBitmap
{
	public:
		typedef PlatformBitmap Super;
		typedef WinBitmap Self;
		static const int kBytePackingAlignment = 4;

	protected:
		WinBitmap();
		virtual ~WinBitmap();

	public:
		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual PlatformBitmap::Format GetFormat() const;

	protected:
		mutable void * fData;

	protected:
		Gdiplus::Bitmap *	fBitmap;
		Gdiplus::BitmapData * fLockedBitmapData;

		virtual void Lock();
		virtual void Unlock();
};

class WinFileBitmap : public WinBitmap
{

	public:
		typedef WinBitmap Super;

	protected:
		WinFileBitmap( Rtt_Allocator &context );

	public:
		WinFileBitmap( const char *inPath, Rtt_Allocator &context );
		virtual ~WinFileBitmap();

	private:
		void InitializeMembers();

	protected:
		float CalculateScale() const;
		virtual U32 SourceWidth() const;
		virtual U32 SourceHeight() const;

	public:
		virtual PlatformBitmap::Orientation GetOrientation() const;

	protected:
		Rtt_INLINE bool IsPropertyInternal( PropertyMask mask ) const { return (fProperties & mask) ? true : false; }

	public:
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual bool IsProperty( PropertyMask mask ) const;
		virtual void SetProperty( PropertyMask mask, bool newValue );
		virtual U32 UprightWidth() const;
		virtual U32 UprightHeight() const;

	private:
		float fScale;
		U8 fProperties;
		S8 fOrientation;
		S16 fAngle; // [0, +-90, +-180]

	protected:
#ifdef Rtt_DEBUG
		String fPath;
#endif
};

class WinFileGrayscaleBitmap : public WinFileBitmap
{
	public:
		typedef WinFileBitmap Super;

		WinFileGrayscaleBitmap( const char *inPath, Rtt_Allocator &context );
		virtual ~WinFileGrayscaleBitmap();

		virtual void FreeBits() const;
		virtual PlatformBitmap::Format GetFormat() const;

	protected:
		virtual void Lock();
		virtual void Unlock();
		virtual U32 SourceWidth() const;
		virtual U32 SourceHeight() const;

		U32 fWidth;
		U32 fHeight;
};

class WinTextBitmap : public PlatformBitmap
{
	public:
		typedef WinBitmap Super;

		WinTextBitmap(
				Interop::RuntimeEnvironment& environment, const char str[], const WinFont& font,
				int width, int height, const Interop::Graphics::HorizontalAlignment& alignment, Real& baselineOffset);
		virtual ~WinTextBitmap();

		virtual PlatformBitmap::Format GetFormat() const;
		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;

	protected:
		virtual void Lock();
		virtual void Unlock();

	private:
		mutable void *fData;
		U32 fWidth;
		U32 fHeight;
};

} // namespace Rtt
