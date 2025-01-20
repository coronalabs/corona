//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////



#include "PngReader.h"

#if defined( Rtt_DEBUG ) && defined( Rtt_ANDROID_ENV )
#include <android/log.h>
#endif

bool
PngReader::Check(const Rtt::Data<char> & array)
{
	const int bytesToCheck = 4;

	return(!png_sig_cmp((png_byte *)array.Get(), (png_size_t)0, bytesToCheck));
}

void
PngReader::ErrorFunc(png_structp png_ptr, png_const_charp msg)
{
#if defined( Rtt_DEBUG ) && defined( Rtt_ANDROID_ENV )
	__android_log_print(ANDROID_LOG_INFO, "Corona", "PngReader::ErrorFunc %s", msg);
#endif
}

void
PngReader::WarningFunc(png_structp png_ptr, png_const_charp msg)
{
#if defined( Rtt_DEBUG ) && defined( Rtt_ANDROID_ENV )
	__android_log_print(ANDROID_LOG_INFO, "Corona", "PngReader::WarningFunc %s", msg);
#endif
}

png_voidp
PngReader::MallocFunc( png_structp png_ptr, png_size_t size )
{
	PngReader * reader = (PngReader *) png_ptr->io_ptr;
	
	return Rtt_MALLOC( reader->myAllocator, size );
}

void
PngReader::FreeFunc( png_structp png_ptr, png_voidp ptr )
{
//	PngReader * reader = (PngReader *) png_ptr->io_ptr;

	Rtt_FREE( ptr );
}

void 
PngReader::ReadFunc(png_structp png_ptr, png_bytep data, png_size_t length)
{
	PngReader * reader = (PngReader *) png_ptr->io_ptr;
	
	if ( (reader->myBytePosition + length) > reader->myBytes->Length() ) {
		png_error(png_ptr, "Read error");
		return;
	}
	
	const char *	src = reader->myBytes->Get() + reader->myBytePosition;

	memcpy(data, src, length);

	reader->myBytePosition += length;
}

int 
PngReader::Width() const
{
	return myWidth;
}

int
PngReader::Height() const
{
	return myHeight;
}

static void RGBtoRGBA(char *dst, const char *src, int width )
{
	for ( int i = 0; i < width; i++ ) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = 0xff;

		dst += 4;
		src += 3;
	}
}

/**
 * Decode a PNG buffer into an array
 */
bool
PngReader::Decode(const Rtt::Data<char> & srcBytes,AndroidImage & image)
{
	if ( !Check(srcBytes ) )
		return false;

	myBytes = &srcBytes;

	png_structp png_ptr;
	png_infop info_ptr;

   png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING,
		NULL, ErrorFunc, WarningFunc,
		NULL, MallocFunc, FreeFunc );

	if (png_ptr == NULL)
		return false;

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return false;
	}

	png_set_read_fn(png_ptr, (void *)this, ReadFunc);

	png_read_png(png_ptr, info_ptr, 
		PNG_TRANSFORM_GRAY_TO_RGB | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, 
		NULL);

	myWidth = png_get_image_width(png_ptr, info_ptr);
	myHeight = png_get_image_height(png_ptr, info_ptr);

	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	int num_channels = png_get_channels(png_ptr, info_ptr);
	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
	
	image.Init( myWidth, myHeight );

	if (bit_depth == 8 && num_channels == 4) {
		for (int i = 0; i < myHeight; ++i) {
			memcpy(image.Get() + i * myWidth * 4, row_pointers[i], myWidth * 4);
		}
	} else if (bit_depth == 8 && num_channels == 3) {
		for (int i = 0; i < myHeight; ++i) {
			RGBtoRGBA(image.Get() + i * myWidth * 4, (const char *) row_pointers[i], myWidth );
		}
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

	return true;
}
