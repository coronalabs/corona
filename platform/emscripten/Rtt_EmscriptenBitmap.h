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

// ----------------------------------------------------------------------------

class EmscriptenBaseBitmap : public PlatformBitmap
{
	public:
		typedef PlatformBitmap Super;

		typedef enum _Channel
		{
			kRed, kGreen, kBlue, kAlpha,
		}
		Channel;

	public:
		EmscriptenBaseBitmap();
		EmscriptenBaseBitmap(Rtt_Allocator *context, int w, int h, uint8_t* rgba);
		virtual ~EmscriptenBaseBitmap();

		virtual const void * GetBits( Rtt_Allocator *context ) const;
		virtual void FreeBits() const override {};
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual Format GetFormat() const;
		virtual bool IsProperty( PropertyMask mask ) const;
		virtual void SetProperty( PropertyMask mask, bool newValue );
		bool LoadFileBitmap(Rtt_Allocator &context, const char *path);
		static bool SaveBitmap(Rtt_Allocator *context, PlatformBitmap * bitmap, const char * filePath);

	protected:
		Rtt_INLINE bool IsPropertyInternal( PropertyMask mask ) const { return (fProperties & mask) ? true : false; }

		mutable U8 *fData;
		mutable S32 fWidth;
		mutable S32 fHeight;
		Format fFormat;
		U8 fProperties;
};

class EmscriptenFileBitmap : public EmscriptenBaseBitmap
{
	public:
		typedef EmscriptenBaseBitmap Super;

		EmscriptenFileBitmap(Rtt_Allocator& context, const char *filePath);
		virtual ~EmscriptenFileBitmap();

	private:
		String fPath;
};


class EmscriptenMaskFileBitmap : public EmscriptenBaseBitmap
{
	public:
		typedef EmscriptenBaseBitmap Super;

		EmscriptenMaskFileBitmap(Rtt_Allocator& context, const char *filePath);
		virtual ~EmscriptenMaskFileBitmap();
		
	private:
		String fPath;
};


class EmscriptenTextBitmap : public EmscriptenBaseBitmap
{
	public:
		typedef EmscriptenBaseBitmap Super;

		EmscriptenTextBitmap(Rtt_Allocator & context, const char str[], const PlatformFont& font, int width, int height, const char alignment[], Real& baselineOffset);
		virtual ~EmscriptenTextBitmap();
		void setBitmap(int size, uint8_t* image, int w, int h, int isSafari);

		virtual void FreeBits() const override {};
		virtual PlatformBitmap::Format GetFormat() const;
		virtual U8 GetByteAlignment() const;

	private:

		void wrapText();

		int fWrapWidth;
		String fAlignment;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
