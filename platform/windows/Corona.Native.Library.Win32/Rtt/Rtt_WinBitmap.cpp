//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinBitmap.h"
#include "Core\Rtt_Build.h"
#include "Display\Rtt_Display.h"
#include "Interop\Graphics\FontServices.h"
#include "Interop\Graphics\HorizontalAlignment.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_GPUStream.h"
#include "Rtt_WinFont.h"
#include "WinString.h"
#include <windows.h>
#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>


namespace Rtt
{

//
// LoadImageFromFileWithoutLocking
//
// Load a bitmap from a file without leaving the file locked until the
// bitmap is released (as happens by default for bitmaps from files).
//
// From StackOverflow: http://stackoverflow.com/questions/4978419
//
static Gdiplus::Bitmap* LoadImageFromFileWithoutLocking(const WCHAR* fileName)
{
	using namespace Gdiplus;
	Bitmap src(fileName);

	if (src.GetLastStatus() != Ok)
	{
		Rtt_TRACE(( "LoadImageFromFileWithoutLocking: failed to create bitmap for '%S'\n", fileName ));
		return 0;
	}

	Bitmap *dst = new Bitmap(src.GetWidth(), src.GetHeight(), PixelFormat32bppARGB);
	BitmapData srcData;
	BitmapData dstData;
	Gdiplus::Rect rc(0, 0, src.GetWidth(), src.GetHeight());

	if (src.LockBits(&rc, ImageLockModeRead, PixelFormat32bppARGB, &srcData) == Ok)
	{
		if (dst->LockBits(&rc, ImageLockModeWrite, PixelFormat32bppARGB, &dstData) == Ok)
		{
			uint8_t * srcBits = (uint8_t *)srcData.Scan0;
			uint8_t * dstBits = (uint8_t *)dstData.Scan0;
			unsigned int stride;

			if (srcData.Stride > 0)
			{
				stride = srcData.Stride;
			}
			else
			{
				stride = -srcData.Stride;
			}
			memcpy(dstBits, srcBits, src.GetHeight() * stride);

			dst->UnlockBits(&dstData);
		}
		src.UnlockBits(&srcData);
	}
	return dst;
}

WinBitmap::WinBitmap() 
	: fData( NULL ), fBitmap( NULL ), fLockedBitmapData( NULL )
{
}

WinBitmap::~WinBitmap()
{
	Self::FreeBits();

	if ( fBitmap ) {
		Rtt_ASSERT( fLockedBitmapData == NULL );
		delete fBitmap;
		fData = NULL;
	}
}

const void * 
WinBitmap::GetBits( Rtt_Allocator* context ) const
{
	const_cast< WinBitmap * >( this )->Lock();

	return fData; 
}

void 
WinBitmap::FreeBits() const
{
	const_cast< WinBitmap * >( this )->Unlock();
}

void
WinBitmap::Lock()
{
	if ( fBitmap == NULL )
		return;

	Gdiplus::Rect rect;

	rect.X = rect.Y = 0;
	rect.Width = fBitmap->GetWidth();
	rect.Height = fBitmap->GetHeight();

	fLockedBitmapData = new Gdiplus::BitmapData;
	Gdiplus::Status status = fBitmap->LockBits(      
		&rect,
		Gdiplus::ImageLockModeRead,
		PixelFormat32bppPARGB,
		fLockedBitmapData
	);

	fData = fLockedBitmapData->Scan0;
}

void
WinBitmap::Unlock()
{
	if ( fBitmap == NULL )
		return;

	fBitmap->UnlockBits( fLockedBitmapData );
	delete fLockedBitmapData;
	fLockedBitmapData = NULL;
	fData = NULL;
}

U32 
WinBitmap::Width() const
{
	if ( fLockedBitmapData != NULL )
		return fLockedBitmapData->Width;
	if ( fBitmap == NULL )
		return 0;
	return fBitmap->GetWidth();
}

U32 
WinBitmap::Height() const
{
	if ( fLockedBitmapData != NULL )
		return fLockedBitmapData->Height;
	if ( fBitmap == NULL )
		return 0;
	return fBitmap->GetHeight();
}


PlatformBitmap::Format 
WinBitmap::GetFormat() const
{
	return kARGB;
}


// ----------------------------------------------------------------------------

static U8
GetInitialPropertiesValue()
{
	return PlatformBitmap::kIsPremultiplied;
}

WinFileBitmap::WinFileBitmap( Rtt_Allocator &context )
#ifdef Rtt_DEBUG
	: fPath(&context)
#endif
{
	InitializeMembers();
}

WinFileBitmap::WinFileBitmap( const char * inPath, Rtt_Allocator &context )
#ifdef Rtt_DEBUG
	: fPath(&context)
#endif
{
	WinString wPath;

	// Initialize all member variables.
	InitializeMembers();

	// Load bitmap from file.
	wPath.SetUTF8( inPath );
#if defined(Rtt_AUTHORING_SIMULATOR)
	// Use a method of loading the bitmap that doesn't lock the underlying file
	Gdiplus::Bitmap *bm = LoadImageFromFileWithoutLocking(wPath.GetTCHAR());
#else
	Gdiplus::Bitmap *bm = Gdiplus::Bitmap::FromFile( wPath.GetTCHAR() );
#endif
	if ( bm != NULL && bm->GetLastStatus() == Gdiplus::Ok )
	{
		fBitmap = bm;
#ifdef Rtt_DEBUG
		fPath.Set( inPath );
#endif
	}
	else
	{
		delete bm;
	}
}

WinFileBitmap::~WinFileBitmap()
{
}

void
WinFileBitmap::InitializeMembers()
{
	fScale = 1.0;
	fOrientation = kUp;
	fProperties = GetInitialPropertiesValue();
	fBitmap = NULL;
	fLockedBitmapData = NULL;
	fData = NULL;
}

PlatformBitmap::Orientation 
WinFileBitmap::GetOrientation() const
{
	return kUp;
}

float
WinFileBitmap::CalculateScale() const
{
	return 1.0;
}

U32
WinFileBitmap::SourceWidth() const
{
	return WinBitmap::Width();
}

U32
WinFileBitmap::SourceHeight() const
{
	return WinBitmap::Height();
}

U32
WinFileBitmap::Width() const
{
	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceWidth() : UprightWidth() );
}

U32
WinFileBitmap::Height() const
{
	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceHeight() : UprightHeight() );
}

U32
WinFileBitmap::UprightWidth() const
{
	S32 angle = DegreesToUpright();
	return ( 0 == angle || 180 == Abs( angle ) ) ? SourceWidth() : SourceHeight();
}

U32
WinFileBitmap::UprightHeight() const
{
	S32 angle = DegreesToUpright();
	return ( 0 == angle || 180 == Abs( angle ) ) ? SourceHeight() : SourceWidth();
}

bool
WinFileBitmap::IsProperty( PropertyMask mask ) const
{
	return IsPropertyInternal( mask );
}

void
WinFileBitmap::SetProperty( PropertyMask mask, bool newValue )
{
	if ( ! Super::IsPropertyReadOnly( mask ) )
	{
		const U8 p = fProperties;
		const U8 propertyMask = (U8)mask;
		fProperties = ( newValue ? p | propertyMask : p & ~propertyMask );
	}

	switch ( mask )
	{
		case kIsBitsFullResolution:
			fScale = ( newValue ? -1.0f : CalculateScale() );
			break;
		case kIsBitsAutoRotated:
			break;
		default:
			break;
	}
}


// ----------------------------------------------------------------------------

WinFileGrayscaleBitmap::WinFileGrayscaleBitmap( const char *inPath, Rtt_Allocator &context )
	: WinFileBitmap(context)
{
	Gdiplus::Color sourceColor;
	Gdiplus::Bitmap *sourceBitmap = NULL;
	U8 *bitmapBuffer = NULL;
	WinString wPath;
	int byteCount;
	U32 xIndex;
	U32 yIndex;
	U8 grayscaleColor;

#ifdef Rtt_DEBUG
	// Store the path.
	fPath.Set( inPath );
#endif

	// Fetch the bitmap from file.
	wPath.SetUTF8( inPath );
#if defined(Rtt_AUTHORING_SIMULATOR)
	// Use a method of loading the bitmap that doesn't lock the underlying file
	sourceBitmap = LoadImageFromFileWithoutLocking(wPath.GetTCHAR());
#else
	sourceBitmap = Gdiplus::Bitmap::FromFile(wPath.GetTCHAR());
#endif

	if (sourceBitmap == NULL || sourceBitmap->GetLastStatus() != Gdiplus::Ok)
	{
		delete sourceBitmap;
		return;
	}

	// Store the bitmap's dimensions for fast retrieval.
	fWidth = sourceBitmap->GetWidth();
	fHeight = sourceBitmap->GetHeight();

	// Convert the given bitmap to an 8-bit grayscaled bitmap.
	byteCount = fWidth * fHeight;
	if (byteCount > 0)
	{
		// Calculate the pitch of the image, which is the width of the image padded to the byte packing alignment.
		U32 pitch = fWidth;
		U32 delta = fWidth % kBytePackingAlignment;
		if (delta > 0)
			pitch += kBytePackingAlignment - delta;

		// Create the 8-bit grayscaled bitmap.
		// --------------------------------------------------------------------------------------------------------
		// Microsoft GDI cannot create a grayscaled bitmap that OpenGL needs for masking.
		// GDI can only create 8-bit bitmaps with color palettes. So we have to create the bitmap binary ourselves.
		// --------------------------------------------------------------------------------------------------------
		byteCount = pitch * fHeight;
		bitmapBuffer = new U8[byteCount];
		for (yIndex = 0; yIndex < fHeight; yIndex++)
		{
			for (xIndex = 0; xIndex < pitch; xIndex++)
			{
				if (xIndex < fWidth)
				{
					// Convert the source bitmap color to grayscale.
					sourceBitmap->GetPixel(xIndex, yIndex, &sourceColor);
					grayscaleColor = (U8)(
							(0.30 * sourceColor.GetRed()) +
							(0.59 * sourceColor.GetGreen()) +
							(0.11 * sourceColor.GetBlue()));
				}
				else
				{
					// Fill the padded area of the bitmap (due to the pitch) with the color black.
					// This assumes that the user wants black on the edges. A bitmask is expected to have a
					// black border so that only its center area shows through on screen.
					grayscaleColor = 0;
				}
				bitmapBuffer[xIndex + (pitch * yIndex)] = grayscaleColor;
			}
		}

		// Set the image width to the pitch in case it is larger. Otherwise it will not be rendered correctly.
		// Ideally, you shouldn't do this because it will make the DisplayObject wider than expected by at
		// most 3 pixels (assuming the packing alignment is 4 bytes), but until the DisplayObject can compensate
		// for pitch then this will have to do for now.
		fWidth = pitch;
	}

	// The source bitmap is no longer needed.
	delete sourceBitmap;

	// Store the grayscale bitmap binary.
	// The base class will provide the bits via the inherited member variable "fData".
	fData = (void*)bitmapBuffer;
}

WinFileGrayscaleBitmap::~WinFileGrayscaleBitmap()
{
	if (fData)
	{
		delete fData;
		fData = nullptr;
	}
}

void
WinFileGrayscaleBitmap::FreeBits() const
{
	// Do not delete the grayscale bitmap until this object's destructor has been called.
	// This improves hit-test performance in "Rtt_PlatformBitmap.cpp" which tests a pixel's transparency value.
}

void
WinFileGrayscaleBitmap::Lock()
{
}

void
WinFileGrayscaleBitmap::Unlock()
{
}

U32
WinFileGrayscaleBitmap::SourceWidth() const
{
	return fWidth;
}

U32
WinFileGrayscaleBitmap::SourceHeight() const
{
	return fHeight;
}

PlatformBitmap::Format 
WinFileGrayscaleBitmap::GetFormat() const
{
	return kMask;
}

#ifdef Rtt_USE_GDIPLUS_RENDERER

WinTextBitmap::WinTextBitmap(
	Interop::RuntimeEnvironment& environment, const char str[], const WinFont& font,
	int width, int height, const Interop::Graphics::HorizontalAlignment& alignment, Real& baselineOffset)
{
	// Initialize width and height to invalid values.
	// Will set to valid values when bitmap has been successfull generated.
	fWidth = 0;
	fHeight = 0;
	auto fontAscender = .0f;
	// Attempt to load the requested font.
	Interop::Graphics::FontSettings fontSettings;
	fontSettings.CopyFrom(font, Gdiplus::UnitPixel);
	auto gdiFontPointer = environment.GetFontServices().LoadUsing(fontSettings);
	if (!gdiFontPointer)
	{
		Rtt_ASSERT(0);
		return;
	}
	
	// Convert the given UTF-8 text to UTF-16.
	WinString stringConverter;
	stringConverter.SetUTF8(str);

	// Set up the text format and alignment.
	// Note: Removing the LineLimit flag causes text to be clipped if there is not enough height.
	Gdiplus::StringFormat textFormat(Gdiplus::StringFormat::GenericTypographic());
	textFormat.SetAlignment(alignment.GetGdiPlusStringAlignmentId());
	INT formatFlags = textFormat.GetFormatFlags();
	formatFlags &= ~Gdiplus::StringFormatFlagsLineLimit;
	if (width <= 0)
	{
		formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
	}

	// Needs to set the "Gdiplus::StringFormatFlagsMeasureTrailingSpaces" flag.
	// Without this flag Windows trims trailing characters at the end of string.
	formatFlags |= Gdiplus::StringFormatFlagsMeasureTrailingSpaces;

	textFormat.SetFormatFlags(formatFlags);

	// sometimes this makes last character invisible because the measured width less than width required to draw the string
#if 0
	// Append a Unicode "Zero-Width Space" (0x200B) to the end of every line in the given string.
	// This allows GDI+ to render trailing spaces without issue.
	// Note: Do not use the "Gdiplus::StringFormatFlagsMeasureTrailingSpaces" flag.
	//       It causes '\n' characters to be rendered as spaces and mismeasures text with line wrapping.
	if (!stringConverter.IsEmpty())
	{
		stringConverter.Replace(L"\r\n", L"\n");
		stringConverter.Replace(L"\n", L"\x200B\n");
	}
	stringConverter.Append(L'\x200B');
#endif

	// Windows ignores last '\n', so needs a space after last '\n' to force 
	if (stringConverter.EndsWith("\n") || stringConverter.EndsWith("\r\n"))
	{
		stringConverter.Append(L'\n');
	}
	
	// Measure the string width and height and store it to "boundingBox".
	Gdiplus::RectF boundingBox;
	Gdiplus::Status status;
	{
		// Set up a temporary device context to measure text rendering with.
		HDC screenDC = ::CreateDCW(L"DISPLAY", nullptr, nullptr, 0);
		Gdiplus::Graphics screenGraphics(screenDC);
		screenGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

		// Measure the string's text output.
		if (width > 0)
		{
			Gdiplus::RectF layoutBox(0, 0, (Gdiplus::REAL)width, (Gdiplus::REAL)height);
			status = screenGraphics.MeasureString(
						stringConverter.GetUTF16(), stringConverter.GetLength(),
						gdiFontPointer.get(), layoutBox, &textFormat, &boundingBox);
		}
		else
		{
			Gdiplus::PointF pointF(0.0f, 0.0f);
			status = screenGraphics.MeasureString(
						stringConverter.GetUTF16(), stringConverter.GetLength(),
						gdiFontPointer.get(), pointF, &textFormat, &boundingBox);
			boundingBox.X = 0;
			boundingBox.Y = 0;
		}

		// Remove the line height from the bottom line, if applied.
		auto fontFamilyPointer = environment.GetFontServices().GetFamilyFrom(*gdiFontPointer.get());
		if (fontFamilyPointer)
		{
			auto fontStyle = gdiFontPointer->GetStyle();
			auto emHeight = fontFamilyPointer->GetEmHeight(fontStyle);
			if (emHeight > 0)
			{
				auto emScale = gdiFontPointer->GetSize() / (Gdiplus::REAL)emHeight;
				auto ascentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellAscent(fontStyle) * emScale;
				auto descentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellDescent(fontStyle) * emScale;
				auto fontHeight = ascentHeight + descentHeight;
				if (boundingBox.Height > fontHeight)
				{
					auto totalHeight = (Gdiplus::REAL)fontFamilyPointer->GetLineSpacing(fontStyle) * emScale;
					auto lineHeight = totalHeight - fontHeight;
					if ((lineHeight > 0) && (lineHeight < boundingBox.Height))
					{
						boundingBox.Height -= lineHeight;
					}
				}
				fontAscender = ascentHeight;
			}
		}

		// Delete the temporary device context.
		::DeleteDC(screenDC);
	}

	// Constrain the text's bounding box if needed.
	int maxTextureSize = Rtt::Display::GetMaxTextureSize();
	if (width <= 0)
	{
		width = (int)std::ceil(boundingBox.Width);			// Round up.
	}
	if (width < 4)
	{
		width = 4;
	}
	if ((width & 0x3) != 0)
	{
		width = (width + 3) & -4;
	}
	if (height <= 0)
	{
		height = (int)std::ceil(boundingBox.Height);		// Round up.
	}
	if (height < 1)
	{
		height = 1;
	}
	
	// Constrain the bitmap width and height to OpenGL's max texture bounds.
	// This will clip the text if the bounding box exceeds the bitmap, which is the intent.
	width = std::min(width, maxTextureSize);
	height = std::min(height, maxTextureSize);
	baselineOffset = height * 0.5f - fontAscender;

	// Draw the text to a bitmap.
	Gdiplus::Bitmap bitmap(width, height, PixelFormat24bppRGB);
	Gdiplus::Graphics graphics(&bitmap);
	Gdiplus::SolidBrush solidBrush(Gdiplus::Color::White);
	graphics.Clear(Gdiplus::Color::Black);
	if (fontSettings.GetPixelSize() > 20.0f)
	{
		// Render text with GDI+ anti-aliasing for large font sizes. (Don't use for small sizes. Will look blurry.)
		// Must be grid-fitted in order to pixel align characters. (Prevents blurry edges on random characters.)
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
	}
	else
	{
		// Render text with Microsoft ClearType for small font sizes. Looks best with small font sizes.
		// Note: Do not use for large font sizes or else the text will be pixelated in the vertical direction.
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
	}

	// use whole bitmap as canvas for drawString
	boundingBox.X = 0;
	boundingBox.Y = 0;
	boundingBox.Width = (float) width; 
	boundingBox.Height = (float) height;

	graphics.DrawString(
			stringConverter.GetUTF16(), stringConverter.GetLength(),
			gdiFontPointer.get(), boundingBox, &textFormat, &solidBrush);
	
	// Convert the 24-bit color bitmap to an 8-bit grayscaled alpha mask.
	if (bitmap.GetLastStatus() == Gdiplus::Ok)
	{
		Gdiplus::BitmapData srcData;
		Gdiplus::Rect rect(0, 0, width, height);
		status = bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &srcData);
		if (Gdiplus::Ok == status)
		{
			fData = new char[width * height];
			if (fData)
			{
				BYTE *dst = (BYTE*)fData;
				const BYTE *src = (const BYTE*)srcData.Scan0;
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width; x++)
					{
						int grayscaledColor =
								(11 * (int)src[0]) +	// Blue
								(59 * (int)src[1]) +	// Green
								(30 * (int)src[2]);		// Red
						grayscaledColor /= 100;
						*dst = (BYTE)grayscaledColor;
						dst++;
						src += 3;
					}
				}
				fWidth = width;
				fHeight = height;
			}
			status = bitmap.UnlockBits(&srcData);
		}
	}
}
#else

WinTextBitmap::WinTextBitmap(
	Interop::RuntimeEnvironment& environment, const char str[], const WinFont& font,
	int width, int height, const Interop::Graphics::HorizontalAlignment& alignment, Real& baselineOffset)
	: fData(NULL)
	, fWidth(0)
	, fHeight(0)
{
	// Convert the given UTF-8 text to UTF-16.
	WinString stringConverter;
	stringConverter.SetUTF8(str);

	auto fontAscender = .0f;

	// Attempt to load the requested font.
	Interop::Graphics::FontSettings fontSettings;
	fontSettings.CopyFrom(font, Gdiplus::UnitPixel);
	auto gdiFontPointer = environment.GetFontServices().LoadUsing(fontSettings);
	if (!gdiFontPointer)
	{
		Rtt_ASSERT(0);
		return;
	}

	// create in-memory HDC
	HDC hdc = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hdc);
	Rtt_ASSERT(hMemDC != NULL);

//	const char* fontName = font.Name();
//	int nBold = font.IsBold() ? 700 : 400;		// see details https://msdn.microsoft.com/en-us/library/windows/desktop/dd183499(v=vs.85).aspx
//	HFONT hFont = CreateFontA(ceilf(font.Size()*1.8), 0, 0, 0, nBold, font.IsItalic(), 0, 0, 0, 0, 0, PROOF_QUALITY, 0, fontName);

	// clone font
	Gdiplus::Graphics screenGraphics(hdc);
	LOGFONT a_stLogFont;
	gdiFontPointer->GetLogFontW(&screenGraphics, &a_stLogFont);
	a_stLogFont.lfQuality = PROOF_QUALITY;

	HFONT hFont = CreateFontIndirect(&a_stLogFont);
	HGDIOBJ hFontOld = SelectObject(hMemDC, hFont);

	UINT flags = DT_NOPREFIX | DT_WORDBREAK | DT_NOCLIP | DT_EDITCONTROL;
	const char* align = alignment.GetCoronaStringId();
	if (strcmp(align, "left") == 0)
	{
		flags |= DT_LEFT;
	}
	else
	if (strcmp(align, "right") == 0)
	{
		flags |= DT_RIGHT;
	}
	else
	{
		flags |= DT_CENTER;
	}

	// Measure bounding box if width or/and height not provided

	RECT boundingBox;
	SetRect(&boundingBox, 0, 0, width, height);
	int h = DrawTextW(hMemDC, stringConverter.GetUTF16(), stringConverter.GetLength(), &boundingBox, width > 0 ? flags : DT_CALCRECT | DT_NOPREFIX);
	if (h == 0)
	{
		Rtt_LogException("Failed to measure bounding box for: '%s'\n", stringConverter.GetUTF8());
		FreeBits();
		return;
	}
	boundingBox.bottom = h;

	// Remove the line height from the bottom line, if applied.
	auto fontFamilyPointer = environment.GetFontServices().GetFamilyFrom(*gdiFontPointer.get());
	if (fontFamilyPointer)
	{
		auto fontStyle = gdiFontPointer->GetStyle();
		auto emHeight = fontFamilyPointer->GetEmHeight(fontStyle);
		if (emHeight > 0)
		{
			auto emScale = gdiFontPointer->GetSize() / (Gdiplus::REAL)emHeight;
			auto ascentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellAscent(fontStyle) * emScale;
//			auto descentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellDescent(fontStyle) * emScale;
//			auto fontHeight = ascentHeight + descentHeight;
//			if (boundingBox.bottom > fontHeight)
//			{
//				auto totalHeight = (Gdiplus::REAL)fontFamilyPointer->GetLineSpacing(fontStyle) * emScale;
//				auto lineHeight = totalHeight - fontHeight;
//				if ((lineHeight > 0) && (lineHeight < boundingBox.bottom))
//				{
//					boundingBox.bottom -= lineHeight;
//				}
//			}
			fontAscender = ascentHeight;
		}
	}

	// Constrain the text's bounding box if needed.
	if (width <= 0)
	{
		width = (int) ceilf(boundingBox.right);			// Round up.
	}
	if (width < 4)
	{
		width = 4;
	}
	if ((width & 0x3) != 0)
	{
		width = (width + 3) & -4;
	}
	if (height <= 0)
	{
		height = (int) ceilf(boundingBox.bottom);		// Round up.
	}
	if (height < 1)
	{
		height = 1;
	}

	// Constrain the bitmap width and height to OpenGL's max texture bounds.
	// This will clip the text if the bounding box exceeds the bitmap, which is the intent.
	int maxTextureSize = Rtt::Display::GetMaxTextureSize();
	width = min(width, maxTextureSize);
	height = min(height, maxTextureSize);
	baselineOffset = height * 0.5f - fontAscender;

	fWidth = width;
	fHeight = height;

	BITMAPINFO bitmapInfo;
	memset(&bitmapInfo, 0, sizeof(BITMAPINFO));
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = -height;	//  top-down
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = 0;
	bitmapInfo.bmiHeader.biSizeImage = 0;

	void* m_pBits = NULL;
	HBITMAP hBitmap = CreateDIBSection(hMemDC, &bitmapInfo, DIB_RGB_COLORS, &m_pBits, NULL, NULL);
	Rtt_ASSERT(hBitmap != NULL);

	auto hBitmapOld = SelectObject(hMemDC, hBitmap);

	SetBkMode(hMemDC, TRANSPARENT);
	SetBkColor(hMemDC, RGB(0, 0, 0));
	SetTextColor(hMemDC, 0x00FFFFFF);

	SetRect(&boundingBox, 0, 0, width, height);
	h = DrawTextW(hMemDC, stringConverter.GetUTF16(), stringConverter.GetLength(), &boundingBox, flags);
	if (h != 0)
	{
		fData = new char[width * height];
		BYTE *dst = (BYTE*)fData;
		const BYTE *src = (const BYTE*)m_pBits;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				// rgb ==> grayscale
				*dst++ = 0.2989f * src[0] + 0.5870f * src[1] + 0.1140f * src[2];
				src += 4;
			}
		}
	}
	else
	{
		Rtt_LogException("Failed to draw text: '%s'\n", stringConverter.GetUTF8());
		FreeBits();
	}

	// restore selected objects and release new ones to delete
	SelectObject(hMemDC, hFontOld);
	SelectObject(hMemDC, hBitmapOld);

	DeleteObject(hBitmap);
	DeleteObject(hFont);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hdc);
}

#endif

WinTextBitmap::~WinTextBitmap()
{
	FreeBits();
}

void
WinTextBitmap::FreeBits() const
{
	delete fData;
	fData = NULL;
}

U32 
WinTextBitmap::Width() const
{
	return fWidth;
}

U32 
WinTextBitmap::Height() const
{
	return fHeight;
}

PlatformBitmap::Format 
WinTextBitmap::GetFormat() const
{
	return PlatformBitmap::kMask;
}

void
WinTextBitmap::Lock()
{
}

void
WinTextBitmap::Unlock()
{
}

const void * 
WinTextBitmap::GetBits( Rtt_Allocator* context ) const
{
	return fData; 
}

} // namespace Rtt
