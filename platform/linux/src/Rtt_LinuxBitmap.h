//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Display/Rtt_PlatformBitmap.h"
#include "Core/Rtt_Array.h"
#include "Core/Rtt_String.h"

namespace Rtt
{
	class PlatformFont;

	class LinuxBaseBitmap : public PlatformBitmap
	{
	public:
		typedef PlatformBitmap Super;

		typedef enum _Channel
		{
			kRed, kGreen, kBlue, kAlpha,
		}
		Channel;

	public:
		LinuxBaseBitmap();
		LinuxBaseBitmap(Rtt_Allocator *context, int w, int h, uint8_t *rgba);
		virtual ~LinuxBaseBitmap();

		virtual const void *GetBits(Rtt_Allocator *context) const;
		virtual void FreeBits() const override {};
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual Format GetFormat() const;
		virtual bool IsProperty(PropertyMask mask) const;
		virtual void SetProperty(PropertyMask mask, bool newValue);
		bool LoadFileBitmap(Rtt_Allocator &context, const char *path);
		static bool SaveBitmap(Rtt_Allocator *context, PlatformBitmap *bitmap, const char *filePath);

	protected:
		Rtt_INLINE bool IsPropertyInternal(PropertyMask mask) const { return (fProperties & mask) ? true : false; }

		mutable U8 *fData;
		mutable S32 fWidth;
		mutable S32 fHeight;
		Format fFormat;
		U8 fProperties;
	};

	class LinuxFileBitmap : public LinuxBaseBitmap
	{
	public:
		typedef LinuxBaseBitmap Super;

		LinuxFileBitmap(Rtt_Allocator &context, const char *filePath);
		virtual ~LinuxFileBitmap();

	private:
		String fPath;
	};

	class LinuxMaskFileBitmap : public LinuxBaseBitmap
	{
	public:
		typedef LinuxBaseBitmap Super;

		LinuxMaskFileBitmap(Rtt_Allocator &context, const char *filePath);
		virtual ~LinuxMaskFileBitmap();

	private:
		String fPath;
	};

	class LinuxTextBitmap : public LinuxBaseBitmap
	{
	public:
		typedef LinuxBaseBitmap Super;

		LinuxTextBitmap(Rtt_Allocator &context, const char str[], const PlatformFont &font, int width, int height, const char alignment[], Real &baselineOffset);
		virtual ~LinuxTextBitmap();

		virtual void FreeBits() const override {};
		virtual PlatformBitmap::Format GetFormat() const;
		virtual U8 GetByteAlignment() const;

	private:
		void wrapText();

		int fWrapWidth;
		String fAlignment;
	};
}; // namespace Rtt
