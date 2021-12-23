//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_GPUStream.h"
#include "Rtt_LinuxBitmap.h"
#include "Rtt_LinuxFont.h"
#include "Rtt_PlatformFont.h"
#include "Rtt_LinuxContainer.h"
#include "Rtt_Freetype.h"
#include "Display/Rtt_Display.h"
#include "Core/Rtt_Types.h"
#include "wx/image.h"
#include "wx/log.h"

namespace Rtt
{
	// LinuxBaseBitmap
	LinuxBaseBitmap::LinuxBaseBitmap()
		: Super(), fData(NULL), fWidth(0), fHeight(0), fFormat(kUndefined), fProperties(0)
	{
	}

	LinuxBaseBitmap::LinuxBaseBitmap(Rtt_Allocator *context, int w, int h, uint8_t *rgba)
		: Super(), fData(NULL), fWidth(w), fHeight(h), fFormat(kRGBA), fProperties(0)
	{
		int size = fHeight * fWidth * 4;
		fData = (U8 *)Rtt_MALLOC(&context, size);
		memset(fData, 0, size);

		U8 *dst = fData;

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				// premultiple alpha
				dst[0] = (rgba[0] * rgba[3]) >> 8;
				dst[1] = (rgba[1] * rgba[3]) >> 8;
				dst[2] = (rgba[2] * rgba[3]) >> 8;
				dst[3] = rgba[3];

				dst += 4;
				rgba += 4;
			}
		}

		fFormat = kRGBA;
	}

	LinuxBaseBitmap::~LinuxBaseBitmap()
	{
		free(fData);
	}

	const void *LinuxBaseBitmap::GetBits(Rtt_Allocator *context) const
	{
		return fData;
	}

	U32 LinuxBaseBitmap::Width() const
	{
		//return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceWidth() : UprightWidth() );
		return fWidth;
	}

	U32 LinuxBaseBitmap::Height() const
	{
		//return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceHeight() : UprightHeight() );
		return fHeight;
	}

	PlatformBitmap::Format LinuxBaseBitmap::GetFormat() const
	{
		return fFormat;
	}

	bool LinuxBaseBitmap::IsProperty(PropertyMask mask) const
	{
		return IsPropertyInternal(mask);
	}

	void LinuxBaseBitmap::SetProperty(PropertyMask mask, bool newValue)
	{
		if (!Super::IsPropertyReadOnly(mask))
		{
			const U8 p = fProperties;
			const U8 propertyMask = (U8)mask;
			fProperties = (newValue ? p | propertyMask : p & ~propertyMask);
		}

		switch (mask)
		{
			case kIsBitsFullResolution:
				break;
			case kIsBitsAutoRotated:
				break;
			default:
				break;
		}
	}

	bool LinuxBaseBitmap::LoadFileBitmap(Rtt_Allocator &context, const char *path)
	{
		Rtt_ASSERT(fData == NULL);

		// disable log messages
		wxLogNull logNo;
		wxImage img;

		if (img.LoadFile(path))
		{
			fWidth = img.GetWidth();
			fHeight = img.GetHeight();

			int size = fWidth * fHeight * 4;
			fData = (U8 *)malloc(size);
			U8 *dst = fData;

			for (int y = 0; y < fHeight; y++)
			{
				for (int x = 0; x < fWidth; x++)
				{
					U8 r = img.GetRed(x, y);
					U8 g = img.GetGreen(x, y);
					U8 b = img.GetBlue(x, y);
					U8 a = img.HasAlpha() ? img.GetAlpha(x, y) : 255;

					// premultiple alpha
					if (a < 255)
					{
						dst[0] = (r * a) >> 8;
						dst[1] = (g * a) >> 8;
						dst[2] = (b * a) >> 8;
					}
					else
					{
						dst[0] = r;
						dst[1] = g;
						dst[2] = b;
					}
					dst[3] = a;
					dst += 4;
				}
			}
			fFormat = kRGBA;
		}

		return fData != NULL;
	}

	bool LinuxBaseBitmap::SaveBitmap(Rtt_Allocator *context, PlatformBitmap *bitmap, const char *filePath)
	{
		// Validate.
		if ((NULL == bitmap) || (NULL == filePath))
		{
			return false;
		}

		wxBitmapType bitmapType = wxBITMAP_TYPE_JPEG;

		if (strcmp(filePath + strlen(filePath) - 4, ".png") == 0)
		{
			bitmapType = wxBITMAP_TYPE_PNG;
		}

		S32 w = bitmap->Width();
		S32 h = bitmap->Height();

		if (w <= 0 || h <= 0)
		{
			return false;
		}

		// Fetch the given bitmap's bits.
		U8 *bits = (U8 *)(bitmap->GetBits(context));

		if (bits == NULL)
		{
			return false;
		}

		// save
		U8 *rgb = (U8 *)malloc(w * h * 3);
		U8 *alpha = (U8 *)malloc(w * h);
		U8 *prgb = rgb;
		U8 *palpha = alpha;

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				prgb[0] = bits[1];
				prgb[1] = bits[2];
				prgb[2] = bits[3];
				*palpha = bits[0];

				bits += 4;
				prgb += 3;
				palpha++;
			}
		}

		wxImage img(w, h, rgb, alpha, true); // disable taking ownership of the data and free it afterwards.
		bool rc = img.SaveFile(filePath, bitmapType);

		free(rgb);
		free(alpha);
		return rc;
	}

	// FileBitmap
	LinuxFileBitmap::LinuxFileBitmap(Rtt_Allocator &context, const char *path)
		: Super(), fPath(&context, path)
	{
		LoadFileBitmap(context, path);
	}

	LinuxFileBitmap::~LinuxFileBitmap()
	{
	}

	// MaskFileBitmap
	LinuxMaskFileBitmap::LinuxMaskFileBitmap(Rtt_Allocator &context, const char *filePath)
		: Super(), fPath(&context, filePath)
	{
		if (LoadFileBitmap(context, filePath))
		{
			Rtt_ASSERT(fFormat == kRGBA || fFormat == kRGB);

			// convert to grayscale
			int size = fHeight * fWidth;
			U8 *newData = (U8 *)Rtt_MALLOC(&context, size);
			U8 *src = fData;
			U8 *dst = newData;

			for (int y = 0; y < fHeight; y++)
			{
				for (int x = 0; x < fWidth; x++)
				{
					*dst++ = (U8)(0.30f * src[0] + 0.59f * src[1] + 0.11f * src[2]);
					src += (fFormat == kRGBA) ? 4 : 3;
				}
			}

			free(fData);
			fData = newData;
			fFormat = kMask;
		}
	}

	LinuxMaskFileBitmap::~LinuxMaskFileBitmap()
	{
	}

	// TextBitmap
	LinuxTextBitmap::LinuxTextBitmap(Rtt_Allocator &context, const char str[], const Rtt::PlatformFont &inFont, int width, int height, const char alignment[], Real &baselineOffset)
		: Super(), fWrapWidth(width), fAlignment(&context, alignment)
	{
		glyph_freetype_provider *gp = getGlyphProvider();

		if (gp == NULL)
		{
			return;
		}

		int align = 0;
		bool is_bold = false;
		bool is_italic = false;
		std::vector<int> xleading;
		std::vector<int> yleading;
		bool multiline = false;
		float xscale = 1;
		float yscale = 1;
		const char *fontFile = inFont.Name();
		int fontSize = inFont.Size();
		
		if (width > 0 && width < 4)
		{
			width = 4;
		}
		
		smart_ptr<alpha> im = gp->render_string(str, alignment, fontFile, is_bold, is_italic, fontSize, xleading, yleading, width, height, multiline, xscale, yscale, &baselineOffset);

		if (im == NULL)
		{
			return;
		}

		fWidth = im->m_width;
		fHeight = im->m_height;
		const U8 *image = im->m_data;

		if (fWidth & 0x3)
		{
			fWidth = (fWidth + 3) & -4;
		}

		fData = (U8 *)Rtt_MALLOC(&context, fHeight * fWidth * 4);
		memset(fData, 0, fHeight * fWidth * 4);

		int pitch = fWidth * 4; // rgba
		int bpp = 4;

		for (int y = 0; y < fHeight; y++)
		{
			for (int x = 0; x < fWidth; x++)
			{
				const U8 *src = image + y * im->m_pitch + x * im->m_bpp;
				U8 *dst = fData + y * pitch + x * bpp;
				dst[0] = src[0];
				dst[1] = src[0];
				dst[2] = src[0];
				dst[3] = src[0];
			}
		}

		fFormat = kRGBA;
	}

	LinuxTextBitmap::~LinuxTextBitmap()
	{
	}

	PlatformBitmap::Format LinuxTextBitmap::GetFormat() const
	{
		return PlatformBitmap::kRGBA;
	}

	U8 LinuxTextBitmap::GetByteAlignment() const
	{
		return 1;
	}
}; // namespace Rtt
