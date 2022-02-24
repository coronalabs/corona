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
#include "Rtt_EmscriptenBitmap.h"
#include "Rtt_EmscriptenFont.h"
#include "Rtt_PlatformFont.h"
#include "Display/Rtt_Display.h"
#include "Core/Rtt_Types.h"
#include "Rtt_BitmapUtils.h"
#include <SDL2/SDL.h>

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"		// for native alert and etc

extern "C"
{
	extern int jsRenderText(void* thiz, const char* text, int width, int height, const char* alignment, const char* fontName, int fontSize);

	// Java ==> Lua callback
	void EMSCRIPTEN_KEEPALIVE jsEmscriptenBitmapSaveImage(Rtt::EmscriptenTextBitmap* thiz, int size, uint8_t* image, int w, int h, int isSafari)
	{
		//printf("jsEmscriptenBitmapSaveImage: %p %d %p %d %d\n", thiz, size, image, w, h);
		thiz->setBitmap(size, image, w, h, isSafari); 
	}
}
#else
	int jsRenderText(void* thiz, const char* text, int width, int height, const char* alignment, const char* fontName, int fontSize) { return 0; };
#endif

// for debugging
#ifdef _DEBUG
void printBitmap(const char* path, const U8* img, int w, int h, int bpp, int channel)
{
	FILE* f = fopen(path, "w");
	if (f)
	{
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				int i = (y * w + x) * bpp;
				if (channel == 0)	// all
				{
					for (int k = 0; k < bpp; k++)
					{
						fprintf(f, "%02X", img[i + k]);
					}
				}
				else
				{
					if (img[i + channel] == 0) fprintf(f, "  "); else fprintf(f, "%02X", img[i + channel]);
				}
				fprintf(f, " ");
			}
			fprintf(f, "\n");
		}
		fclose(f);
	}
}
#endif

namespace Rtt
{

	//
	// EmscriptenBaseBitmap
	//

	EmscriptenBaseBitmap::EmscriptenBaseBitmap()
		: Super()
		, fData(NULL)
		, fWidth(0)
		, fHeight(0)
		, fFormat(kUndefined)
		, fProperties(0)
	{
	}

	EmscriptenBaseBitmap::EmscriptenBaseBitmap(Rtt_Allocator *context, int w, int h, uint8_t* rgba)
		: Super()
		, fData(NULL)
		, fWidth(w)
		, fHeight(h)
		, fFormat(kRGBA)
		, fProperties(0)
	{
		int size = fHeight * fWidth * 4;
		fData = (U8*)Rtt_MALLOC(&context, size);
		memset(fData, 0, size);
		
		U8* dst = fData;
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

	EmscriptenBaseBitmap::~EmscriptenBaseBitmap()
	{
		free(fData);
	}

	const void* EmscriptenBaseBitmap::GetBits(Rtt_Allocator *context) const
	{
		return fData;
	}

	U32 EmscriptenBaseBitmap::Width() const
	{
		//	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceWidth() : UprightWidth() );
		return fWidth;
	}

	U32 EmscriptenBaseBitmap::Height() const
	{
		//	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceHeight() : UprightHeight() );
		return fHeight;
	}

	PlatformBitmap::Format EmscriptenBaseBitmap::GetFormat() const
	{
		return fFormat;
	}

	bool EmscriptenBaseBitmap::IsProperty(PropertyMask mask) const
	{
		return IsPropertyInternal(mask);
	}

	void EmscriptenBaseBitmap::SetProperty(PropertyMask mask, bool newValue)
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

	bool EmscriptenBaseBitmap::LoadFileBitmap(Rtt_Allocator &context, const char *path)
	{
		Rtt_ASSERT(fData == NULL);

		// get file ext
		int n = strlen(path);
		if (n < 5)
		{
			return false;
		}

		std::string ext = path + n - 4;
		if (ext == ".bmp")
		{
			fData = bitmapUtil::loadBMP(path, fWidth, fHeight, fFormat);
			if (fData)
			{
				fFormat = kRGBA;
			}
		}
		else
		if (ext == ".png")
		{
			FILE* f = fopen(path, "rb");
			if (f)
			{
				fData = bitmapUtil::loadPNG(f, fWidth, fHeight);
				if (fData)
				{
					fFormat = kRGBA;
				}
				fclose(f);
			}
		}
		else
		if (ext == ".jpg")
		{
			FILE* f = fopen(path, "rb");
			if (f)
			{
				uint8_t* img = bitmapUtil::loadJPG(f, fWidth, fHeight);
				if (img)
				{
					// convert to RGBA, for some reason RGB images are rendered incorrectly
					fData = (uint8_t*) malloc(fWidth * fHeight * 4);
					fFormat = kRGBA;

					U8* src = img;
					U8* dst = fData;
					for (int y = 0; y < fHeight; y++)
					{
						for (int x = 0; x < fWidth; x++)
						{
							dst[0] = src[0];
							dst[1] = src[1];
							dst[2] = src[2];
							dst[3] = 255;
							dst += 4;
							src += 3;
						}
					}
					free(img);
				}
				fclose(f);
				return fData != NULL;
			}
		}

		if (fData && fFormat == kRGBA)
		{
			U8* dst = fData;
			for (int y = 0; y < fHeight; y++)
			{
				for (int x = 0; x < fWidth; x++)
				{
					// premultiple alpha
					if (dst[3] < 255)
					{
						dst[0] = (dst[0] * dst[3]) >> 8;
						dst[1] = (dst[1] * dst[3]) >> 8;
						dst[2] = (dst[2] * dst[3]) >> 8;
					}
					dst += 4;
				}
			}
		}

		return fData != NULL;
	}

	bool EmscriptenBaseBitmap::SaveBitmap(Rtt_Allocator* context, PlatformBitmap * bitmap, const char * filePath)
	{
		// Validate.
		if ((NULL == bitmap) || (NULL == filePath))
		{
			return false;
		}

		if (strcmp(filePath + strlen(filePath) - 4, ".png") != 0)
		{
			Rtt_LogException("Failed to save %s, HTML5 supports only .png files\n", filePath);
			return false;
		}

		S32 w = bitmap->Width();
		S32 h = bitmap->Height();
		if (w <= 0 || h <= 0)
		{
			return false;
		}

		// Fetch the given bitmap's bits.
		U8 *bits = (U8*)(bitmap->GetBits(context));
		if (bits == NULL)
		{
			return false;
		}

		return bitmapUtil::savePNG(filePath, bits, w, h, bitmap->GetFormat());
	}

	//
	// FileBitmap
	//

	EmscriptenFileBitmap::EmscriptenFileBitmap(Rtt_Allocator &context, const char *path)
		: Super()
		, fPath(&context, path)
	{
		LoadFileBitmap(context, path);
	}

	EmscriptenFileBitmap::~EmscriptenFileBitmap()
	{
	}
	 
	//
	// MaskFileBitmap
	//

	EmscriptenMaskFileBitmap::EmscriptenMaskFileBitmap(Rtt_Allocator& context, const char *filePath)
		: Super()
		, fPath(&context, filePath)
	{
		if (LoadFileBitmap(context, filePath))
		{
			Rtt_ASSERT(fFormat == kRGBA || fFormat == kRGB);

			// convert to grayscale
			int size = fHeight * fWidth;
			U8* newData = (U8*)Rtt_MALLOC(&context, size);
			U8* src = fData;
			U8* dst = newData;
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

	EmscriptenMaskFileBitmap::~EmscriptenMaskFileBitmap()
	{
	}

	 
	//
	// TextBitmap
	//

	EmscriptenTextBitmap::EmscriptenTextBitmap(Rtt_Allocator & context, const char str[], const Rtt::PlatformFont& inFont, int width, int height, const char alignment[], Real& baselineOffset)
		: Super()
		, fWrapWidth(width)
		, fAlignment(&context, alignment)
	{
		jsRenderText(this, str, width, height, alignment, inFont.Name(), inFont.Size());
		baselineOffset = fHeight * 0.5f - inFont.Size();
	}

	void EmscriptenTextBitmap::setBitmap(int size, uint8_t* image, int w, int h, int isSafari)
	{
		fWidth = w;
		fHeight = h;

		fData = (U8*)Rtt_MALLOC(&context, fHeight * fWidth);
		memset(fData, 0, fHeight * fWidth);

		// extract alpha component
		int pitch = w * 4;		// rgba
		int bpp = 4;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				// Here p is the address to the pixel we want to retrieve
				uint8_t *p = image + y * pitch + x * bpp;
				fData[y*w + x] = (isSafari == 1) ? p[0] : p[3];
			}
		}
	}

	EmscriptenTextBitmap::~EmscriptenTextBitmap()
	{
	}

	PlatformBitmap::Format EmscriptenTextBitmap::GetFormat() const
	{
		return PlatformBitmap::kMask;
	}

	U8 EmscriptenTextBitmap::GetByteAlignment() const
	{
		return 1;
	}

} // namespace Rtt

