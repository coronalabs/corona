// ----------------------------------------------------------------------------
// 
// Rtt_WinRTBitmap.h
// Copyright (c) 2013 Corona labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include "CoronaLabs\Corona\WinRT\Interop\Graphics\BitmapInfo.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\IBitmap.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\IImageServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\IImageDecoder.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\ITextRenderer.h"
#include "Rtt_WinRTFont.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Display/Rtt_PlatformBitmap.h"
#	include "Core/Rtt_Array.h"
#	include "Core/Rtt_String.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{
class PlatformFont;

class WinRTImageFileBitmap : public PlatformBitmap
{
	public:
		WinRTImageFileBitmap(
					Rtt_Allocator &allocator, Platform::String^ filePath,
					CoronaLabs::Corona::WinRT::Interop::Graphics::IImageDecoder^ imageDecoder);
		virtual ~WinRTImageFileBitmap();

		virtual const void* GetBits(Rtt_Allocator *allocatorPointer) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual U32 UprightWidth() const;
		virtual U32 UprightHeight() const;
		virtual Real GetScale() const;
		virtual PlatformBitmap::Format GetFormat() const;
		virtual bool IsProperty(PropertyMask mask) const;
		virtual void SetProperty(PropertyMask mask, bool newValue);

	protected:
		Rtt_INLINE bool IsPropertyInternal( PropertyMask mask ) const { return (fProperties & mask) ? true : false; }

	private:
		CoronaLabs::Corona::WinRT::Interop::Graphics::IImageDecoder^ fImageDecoder;
		mutable CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo^ fBitmapInfo;
		mutable CoronaLabs::Corona::WinRT::Interop::Graphics::IBitmap^ fBitmapData;
		Rtt_Allocator* fAllocatorPointer;
		Platform::String^ fFilePath;
		U8 fProperties;
};

class WinRTTextBitmap : public PlatformBitmap
{
	public:
		WinRTTextBitmap(Rtt_Allocator &allocator, CoronaLabs::Corona::WinRT::Interop::Graphics::ITextRenderer^ textRenderer);
		virtual ~WinRTTextBitmap();

		virtual const void* GetBits(Rtt_Allocator* allocatorPointer) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual Format GetFormat() const;
		virtual bool IsProperty(PropertyMask mask) const;
		virtual void SetProperty(PropertyMask mask, bool newValue);

	private:
		CoronaLabs::Corona::WinRT::Interop::Graphics::ITextRenderer^ fTextRenderer;
		mutable CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo^ fBitmapInfo;
		mutable CoronaLabs::Corona::WinRT::Interop::Graphics::IBitmap^ fBitmapData;
		Rtt_Allocator* fAllocatorPointer;
		U8 fProperties;
};

} // namespace Rtt
