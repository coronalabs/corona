//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Types.h"
#include "Rtt_BitmapUtils.h"
#include "Rtt_Math.h"
#include <png.h>
#include <jpeglib.h>
#include <cstring>		// for memcpy
#include <memory>
#include <SDL.h>

namespace bitmapUtil
{

	struct JpegErrorMgr
	{
		struct jpeg_error_mgr pub;	// "public" fields
		jmp_buf setjmp_buffer;	// for return to caller
	};
	typedef struct JpegErrorMgr* jpegErrorMgr;

	void jpgErrorHandler(j_common_ptr cinfo)
	{
		// cinfo->err really points to a jpegErroMgr struct, so coerce pointer 
		jpegErrorMgr myerr = (jpegErrorMgr)cinfo->err;

		// Always display the message. 
		(*cinfo->err->output_message) (cinfo);

		// Return control to the setjmp point 
		longjmp(myerr->setjmp_buffer, 1);
	}

	uint8_t* loadJPG(FILE* infile, int& w, int& h)
	{
		struct jpeg_decompress_struct cinfo;
		struct JpegErrorMgr jerr;

		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = jpgErrorHandler;

		if (setjmp(jerr.setjmp_buffer))
		{
			jpeg_destroy_decompress(&cinfo);
			return NULL;
		}

		jpeg_create_decompress(&cinfo);
		jpeg_stdio_src(&cinfo, infile);

		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);

		int row_stride = cinfo.output_width * cinfo.output_components;
		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)	((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
		int i = 0;
		uint8_t* im = (uint8_t*)malloc(row_stride * cinfo.output_height);
		uint8_t* p = im;
		w = cinfo.output_width;
		h = cinfo.output_height;
		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(p, buffer[0], row_stride);
			p += row_stride;
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return im;
	}

	// get pixel value from SDL surface
	Uint32 getSurfacePixel(SDL_Surface* surface, int x, int y)
	{
		int bpp = surface->format->BytesPerPixel;

		// Here p is the address to the pixel we want to retrieve
		Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
		switch (bpp)
		{
		case 1:
			return *p;
		case 2:
			return *(Uint16*)p;
		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		case 4:
			return (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3] : p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
		default:
			return 0;       // shouldn't happen, but avoids warnings
		}
	}

	bool	saveJPG(const char* filename, uint8_t* data, int width, int height, Rtt::PlatformBitmap::Format format, float jpegQuality)
	{
		FILE* outfile = fopen(filename, "wb");
		if (!outfile)
			return false;

		struct jpeg_compress_struct cinfo;
		struct JpegErrorMgr jerr;
		JSAMPROW row_pointer[1];        // pointer to JSAMPLE row[s] 
		int     row_stride;          // physical row width in image buffer 

		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = jpgErrorHandler;

		if (setjmp(jerr.setjmp_buffer))
		{
			jpeg_destroy_compress(&cinfo);
			return false;
		}

		// Now we can initialize the JPEG compression object
		jpeg_create_compress(&cinfo);
		jpeg_stdio_dest(&cinfo, outfile);
		cinfo.image_width = width;
		cinfo.image_height = height;
		jpeg_set_quality(&cinfo, Rtt::Clamp((int)(jpegQuality * 100), 1, 100), TRUE);

		row_stride = width * 3; // JSAMPLEs per row in image_buffer 
		cinfo.input_components = 3;       // # of color components per pixel 
		cinfo.in_color_space = JCS_RGB;       // colorspace of input image 

		jpeg_set_defaults(&cinfo);
		jpeg_start_compress(&cinfo, TRUE);

		std::unique_ptr<uint8_t> rgb;
		switch (format)
		{
		case Rtt::PlatformBitmap::Format::kRGB:
			break;

		case Rtt::PlatformBitmap::Format::kRGBA:
		{
			// convert to RGB
			rgb.reset((uint8_t*)malloc(width * height * 3));
			uint8_t* src = data;
			uint8_t* dst = rgb.get();
			for (int i = 0; i < width * height; i++)
			{
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;
				src++;
			}
			data = rgb.get();
			break;
		}
		case Rtt::PlatformBitmap::Format::kABGR:
		case Rtt::PlatformBitmap::Format::kARGB:
			Rtt_ASSERT(0); //todo
			break;

		case Rtt::PlatformBitmap::Format::kBGRA:
		{
			// convert to RGB
			rgb.reset((uint8_t*)malloc(width * height * 3));
			uint8_t* src = data;
			uint8_t* dst = rgb.get();
			for (int i = 0; i < width * height; i++)
			{
				// hmmmm, actually src contains ARGB format!
				src++;
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;
			}
			data = rgb.get();
			break;
		}
		}

		while (cinfo.next_scanline < cinfo.image_height)
		{
			row_pointer[0] = &data[cinfo.next_scanline * row_stride];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		fclose(outfile);
		jpeg_destroy_compress(&cinfo);
		return true;
	}

	//
	// bmp
	//

	uint8_t* loadBMP(const char* path, int& w, int& h, Rtt::PlatformBitmap::Format& format)
	{
		SDL_Surface* img = SDL_LoadBMP(path);
		if (img)
		{
			w = img->w;
			h = img->h;

			int size = w * h * 4;
			uint8_t* im = (uint8_t*)malloc(size);
			memset(im, 0, size);

			uint8_t* dst = im;
			SDL_LockSurface(img);
			for (int y = 0; y < img->h; y++)
			{
				for (int x = 0; x < img->w; x++)
				{
					Uint32 pixel = getSurfacePixel(img, x, y);
					SDL_GetRGBA(pixel, img->format, dst, dst + 1, dst + 2, dst + 3);

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
			SDL_UnlockSurface(img);
			SDL_FreeSurface(img);

			format = Rtt::PlatformBitmap::Format::kRGBA;
			return im;
		}
	}

	//
	// png
	//

	uint8_t* loadPNG(FILE* fp, int& w, int& h)
	{
		png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png == NULL)
		{
			return NULL;
		}

		png_infop info = png_create_info_struct(png);
		if (info == NULL)
		{
			return NULL;
		}

		if (setjmp(png_jmpbuf(png)))
		{
			return NULL;
		}

		png_init_io(png, fp);
		png_read_info(png, info);

		w = png_get_image_width(png, info);
		h = png_get_image_height(png, info);
		int color_type = png_get_color_type(png, info);
		int bit_depth = png_get_bit_depth(png, info);

		if (bit_depth == 16)
			png_set_strip_16(png);

		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png);

		// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
		if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
			png_set_expand_gray_1_2_4_to_8(png);

		if (png_get_valid(png, info, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(png);

		// These color_type don't have an alpha channel then fill it with 0xff.
		if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(png);

		png_read_update_info(png, info);

		png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * h);
		for (int y = 0; y < h; y++)
		{
			row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
		}
		png_read_image(png, row_pointers);

		int pitch = png_get_rowbytes(png, info);
		uint8_t* im = (uint8_t*)malloc(pitch * h);
		uint8_t* dst = im;
		for (int y = 0; y < h; y++)
		{
			png_bytep row = row_pointers[y];
			memcpy(dst, row, pitch);
			dst += pitch;
			free(row_pointers[y]);
		}
		free(row_pointers);
		png_destroy_read_struct(&png, &info, NULL);

		return im;
	}

	void pngWriteFunc(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		FILE* f = (FILE*)png_get_io_ptr(png_ptr); // was png_ptr->io_ptr
		fwrite(data, length, 1, f);
	}

	bool	savePNG(const char* filename, uint8_t* data, int width, int height, Rtt::PlatformBitmap::Format format)
		// Writes a 24 or 32-bit color image in .png format, to the
		// given output stream.  Data should be in [RGB or RGBA...] byte order.
	{
		int bpp = Rtt::PlatformBitmap::BytesPerPixel(format);
		if (bpp != 3 && bpp != 4)
		{
			//		printf("png writer: bpp must be 3 or 4\n");
			return false;
		}

		FILE* out = fopen(filename, "wb");
		if (out == NULL)
		{
			//		printf("png writer: can't create %s\n", filename);
			return false;
		}

		png_structp	png_ptr;
		png_infop	info_ptr;

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL)
		{
			// @@ log error here!
			fclose(out);
			return false;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL)
		{
			// @@ log error here!
			png_destroy_write_struct(&png_ptr, NULL);
			fclose(out);
			return false;
		}

		png_init_io(png_ptr, out);
		png_set_write_fn(png_ptr, (png_voidp)out, pngWriteFunc, NULL);
		png_set_IHDR(png_ptr, info_ptr, width, height, 8, bpp == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		if (format == Rtt::PlatformBitmap::Format::kBGRA)
		{
			png_set_swap_alpha(png_ptr);
		}
		//	 png_set_bgr(png_ptr);

		png_write_info(png_ptr, info_ptr);
		for (int y = 0; y < height; y++)
		{
			png_write_row(png_ptr, data + (width * bpp) * y);
		}

		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(out);
		return true;
	}
}

