//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_Freetype.h"
#include "Rtt_LinuxUtils.h"
#include "default.ttf.h"
#include <unordered_map>

static std::unordered_map<std::string, bool> s_fontname_printed;		// for not repeating log

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
					if (img[i + channel] == 0) fprintf(f, "  ");
					else fprintf(f, "%02X", img[i + channel]);
				}
				fprintf(f, " ");
			}
			fprintf(f, "\n");
		}
		fclose(f);
	}
}
#else
#define printBitmap(...)
#endif

namespace Rtt
{

	//
	//	glyph provider implementation
	//

	static smart_ptr<glyph_freetype_provider> sGlyphProvider;

	glyph_freetype_provider* getGlyphProvider()
	{
		return sGlyphProvider.get();
	}
	void setGlyphProvider(glyph_freetype_provider* gp)
	{
		sGlyphProvider = gp;
	}

	glyph_freetype_provider::glyph_freetype_provider(const char* pathToApp)
		: m_scale(0)
	{
		m_base_dir = pathToApp;
		int	error = FT_Init_FreeType(&m_lib);
		if (error != 0)
		{
			Rtt_LogException("Failed to init FreeType, error = %d\n", error);
		}

		if (m_base_dir.size() == 0)
		{
			m_base_dir = '/';
		}
		if (m_base_dir[m_base_dir.size() - 1] != '/')
		{
			m_base_dir += '/';
		}
	}

	glyph_freetype_provider::~glyph_freetype_provider()
	{
		m_face_entity.clear();

		int error = FT_Done_FreeType(m_lib);
		if (error)
		{
			Rtt_LogException("failed to close FreeType, error = %d\n", error);
		}
		m_lib = NULL;
	}

	face_entity* glyph_freetype_provider::get_face_entity(const std::string& fontname, bool is_bold, bool is_italic)
	{
		// first try to find from hash
		auto it = m_face_entity.find(fontname);
		if (it != m_face_entity.end())
		{
			return it->second;
		}

		FT_Face face = NULL;
		std::string url = m_base_dir + fontname;
		FT_New_Face(m_lib, url.c_str(), 0, &face);
		if (face == NULL)
		{
			// try .ttf
			url = m_base_dir + fontname + ".ttf";
			FT_New_Face(m_lib, url.c_str(), 0, &face);
			if (face == NULL)
			{
				// try .otf
				url = m_base_dir + fontname + ".otf";
				FT_New_Face(m_lib, url.c_str(), 0, &face);
				if (face == NULL)
				{
					// try default
					url = m_base_dir + "default.ttf";
					FT_New_Face(m_lib, url.c_str(), 0, &face);
					if (face == NULL)
					{
						if (fontname.size() > 0 && s_fontname_printed.find(fontname) == s_fontname_printed.end())
						{
							s_fontname_printed[fontname] = true;
							Rtt_Log("WARNING: %s not found, using embedded font \n", fontname.c_str());
						}
						FT_New_Memory_Face(m_lib, default_ttf, sizeof(default_ttf), 0, &face);
					}
				}
			}
		}

		if (is_bold)
		{
			face->style_flags |= FT_STYLE_FLAG_BOLD;
		}

		if (is_italic)
		{
			face->style_flags |= FT_STYLE_FLAG_ITALIC;
		}

		face_entity* fe = new face_entity(face);
		m_face_entity[fontname] = fe;
		return fe;
	}

	glyph_entity* glyph_freetype_provider::load_char_image(face_entity* fe, Uint16 code, int fontsize, float xscale)
	{
		// form hash key
		U64 xs = (Uint32)(xscale * 0x10000);
		U64 key = (xs << 32) | (fontsize << 16) | code;

		// try to find stored image
		glyph_entity* ge = NULL;
		FT_GlyphSlot  slot = fe->m_face->glyph;
		auto it = fe->m_ge.find(key);
		if (it == fe->m_ge.end())
		{
			FT_Matrix transform = { (FT_Fixed)(xscale * 0x10000), 0, 0, 0x10000 };
			FT_Set_Transform(fe->m_face, &transform, NULL);

			if (FT_Load_Char(fe->m_face, code, FT_LOAD_RENDER) == 0)
			{
				ge = new glyph_entity();
				ge->m_width = fe->m_face->glyph->bitmap.width;
				ge->m_height = fe->m_face->glyph->bitmap.rows;
				ge->m_image = (Uint8*)malloc(ge->m_width * ge->m_height);
				ge->m_left = slot->bitmap_left;
				ge->m_top = slot->bitmap_top;
				ge->m_advance = slot->advance.x >> 6;

				memcpy(ge->m_image, fe->m_face->glyph->bitmap.buffer, ge->m_width * ge->m_height);

				// keep image
				fe->m_ge[key] = ge;
			}
		}
		return ge;
	}

	glyph_entity* glyph_freetype_provider::get_glyph_entity(face_entity* fe, Uint16 code, int fontsize, float xscale)
	{
		// form hash key
		U64 xs = (Uint32)(xscale * 0x10000);
		U64 key = (xs << 32) | (fontsize << 16) | code;

		auto it = fe->m_ge.find(key);
		return it == fe->m_ge.end() ? NULL : it->second;
	}

	glyph_freetype_provider::rect glyph_freetype_provider::getBoundingBox(face_entity* fe, std::vector<Uint32>& ch, int fontsize, int vertAdvance, float xscale, int boxw, int boxh)
	{
		rect r(0, vertAdvance);

		int i = 0;
		int w = 0;
		int iBegin = 0;
		int iSpace = -1;
		while (i < ch.size())
		{
			Uint32 code = ch[i];
			if (code == 32)
			{
				if (iBegin == i)
				{
					iBegin++;
					continue;
				}
				iSpace = i;		// save last space position
			}

			glyph_entity* ge = get_glyph_entity(fe, code, fontsize, xscale);
			if (code == 0 || code == 10 || code == 13 || ge == NULL)
			{
				if (w > r.width)
				{
					r.width = w;
				}
				w = 0;
				r.height += vertAdvance;
				iBegin = i + 1;
				iSpace = -1;
				i = iBegin;
				continue;
			}

			// unlimited bounds, so we can grow as much as want
			if (boxw == 0)
			{
				w += ge->m_advance;
				i++;
				continue;
			}

			// limited bounds
			if (w + ge->m_advance <= boxw)
			{
				w += ge->m_advance;
				i++;
			}
			else
			{
				if (iSpace >= 0)
				{
					// back to iSpace
					i--;
					for (; i >= iSpace; i--)
					{
						ge = get_glyph_entity(fe, ch[i], fontsize, xscale);
						w -= ge->m_advance;
					}

					if (iBegin < iSpace + 1)
					{
						iBegin = iSpace + 1;
					}
					else
					{
						iBegin = iSpace - 1;
					}
					w = 0;
					iSpace = -1;
					i = iBegin;
					r.height += vertAdvance;
					continue;
				}

				// char on 'i' position will be placed in new line
				if (i == 0)
				{
					// not enough room for 1 char
					break;
				}
				else
				{
					w = 0;
					r.height += vertAdvance;
					iBegin = i;
					iSpace = -1;
				}
			}
		}
		if (w > r.width)	r.width = w;
		if (boxw > 0)	r.width = boxw;
		return r;
	}

	int glyph_freetype_provider::draw_line(alpha* im, face_entity* fe, const std::vector<Uint32>& ch,
		int i1, int i2, int* pen_x, int pen_y, const char* alignment, int boxw, int fontsize, float xscale)
	{
		int line_width = 0;
		for (int i = i1; i < i2; i++)
		{
			glyph_entity* ge = get_glyph_entity(fe, ch[i], fontsize, xscale);
			if (ge)
			{
				line_width += ge->m_advance;
			}
		}

		// align
		if (strcmp(alignment, "right") == 0)
		{
			*pen_x = boxw - line_width - 4;		// hack, чуть справа отступ
		}
		else if (strcmp(alignment, "center") == 0)
		{
			*pen_x = (boxw - line_width) / 2;
		}

		// draw line
		for (int i = i1; i < i2; i++)
		{
			glyph_entity* ge = get_glyph_entity(fe, ch[i], fontsize, xscale);
			if (ge == NULL)
			{
				continue;
			}

			int h = ge->m_height;
			int w = ge->m_width;

			// now, draw to our target surface
			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					int dy = pen_y + y - ge->m_top;
					int dx = *pen_x + x + ge->m_left;

					if (dy < 0 || dy >= im->m_height)
					{
						break;
					}

					if (dx < 0 || dx >= im->m_pitch)
					{
						continue;
					}

					int k = dy * im->m_pitch + dx;
					im->m_data[k] = ge->m_image[y * w + x];

				}
			}
			// increment pen position
			*pen_x += ge->m_advance;
		}
		return line_width;
	}

	smart_ptr<alpha> glyph_freetype_provider::render_string(const std::string& str, const char* alignment, const std::string& fontname,
		bool is_bold, bool is_italic, int fontsize, const std::vector<int>& xleading, const std::vector<int>& yleading,
		int boxw, int boxh, bool multiline, float xscale, float yscale, float* baseline)
	{
		face_entity* fe = get_face_entity(fontname, is_bold, is_italic);
		if (fe == NULL)
		{
			return NULL;
		}

		FT_Face face = fe->m_face;
		FT_Error rc = FT_Set_Pixel_Sizes(face, 0, fontsize);

		*baseline = int(face->size->metrics.height * 0.5f - face->size->metrics.ascender) >> 6;

		// get the default line spacing, note that it is measured in 64ths of a pixel
//		int line_spacing = face->height >> 6;
		// get the scaled line spacing (for 48pt), also measured in 64ths of a pixel
//		int scaled_line_spacing = face->size->metrics.height >> 6;
		int vertAdvance = face->size->metrics.height >> 6; // scaled_line_spacing;

		// load char images
		std::vector<Uint32> ch;
		Uint32	code = 1;
		const char* p = str.c_str();
		const char* end = p + str.size();
		while (code && p < end)
		{
			code = decode_next_unicode_character(&p);

			// hack, replace Tab on Four Spaces
			if (code == 9)	// Tab
			{
				code = 32;
				ch.push_back(code);
				ch.push_back(code);
				ch.push_back(code);
			}
			ch.push_back(code);

			if (code != 0 && code != 10 && code != 13)
			{
				load_char_image(fe, code, fontsize, xscale);
			}
		}

		multiline = true;

		rect r = getBoundingBox(fe, ch, fontsize, vertAdvance, xscale, boxw, boxh);
		if (boxw == 0)
		{
			boxw = r.width;
		}
		if (boxh == 0)
		{
			boxh = r.height;
		}

		// it's needs for corona ?
		if ((boxw & 0x3) != 0)
		{
			boxw = (boxw + 3) & -4;
		}

		alpha* im = new alpha(boxw, boxh);
		memset(im->m_data, 0, im->m_pitch * im->m_height);

		int rowindex = 0;
		int pen_x0 = 0; // xleading[rowindex];
		int pen_x = pen_x0;
		int pen_y_dyna = (fe->m_face->size->metrics.height + fe->m_face->size->metrics.descender) >> 6;
		int pen_y = yleading.size() > rowindex ? yleading[rowindex] : pen_y_dyna;

		// draw

		int i;
		int j;
		int i1 = 0;		// begin of line
		FT_GlyphSlot  slot = fe->m_face->glyph;
		int lw = 0;		// line width
		for (i = 0; i < ch.size(); i++)
		{
			code = ch[i];
			if (code == 0 || code == 10 || code == 13)
			{
				draw_line(im, fe, ch, i1, i, &pen_x, pen_y, alignment, boxw, fontsize, xscale);
				i1 = i + 1;
				i = i1 - 1;		// потомц что прибавится в конце цикла
				lw = 0;

				rowindex++;
				if (rowindex < xleading.size())
				{
					pen_x0 = xleading[rowindex];
				}

				if (multiline)
				{
					pen_x = pen_x0;
					if (rowindex < xleading.size())
					{
						//						pen_y = yleading.size() > rowindex ? yleading[rowindex] : fe->m_face->size->metrics.height >> 6;
						pen_y = yleading.size() > rowindex ? yleading[rowindex] : vertAdvance;
					}
					else
					{
						pen_y += vertAdvance;
					}
				}
				continue;
			}

			// только для ствт полей перевод кареток
			glyph_entity* ge = get_glyph_entity(fe, code, fontsize, xscale);
			if (ge)
			{
				lw += ge->m_advance;
				if (lw <= (boxw - pen_x0))
				{
					continue;
				}

				// назад до пробела
				for (j = i; j >= i1 && ch[j] != 32; j--)
				{
					glyph_entity* je = get_glyph_entity(fe, ch[j], fontsize, xscale);
					lw -= je->m_advance;
				}

				if (j >= i1)
				{
					// есть пробел
					// j указывает на пробел вывести вклюяач пробел
					draw_line(im, fe, ch, i1, j + 1, &pen_x, pen_y, alignment, boxw, fontsize, xscale);
					i1 = j + 1;
				}
				else
				{
					// пробела нет.. всю строку выводить от i1 до i
					if (i1 == i)
					{
						// нет места даже для 1 символа.. 1 все таки вывести
						i++;
					}
					draw_line(im, fe, ch, i1, i, &pen_x, pen_y, alignment, boxw, fontsize, xscale);
					i1 = i;
				}

				i = i1 - 1;		// потомц что прибавится в конце цикла
				lw = 0;
				if (multiline)
				{
					pen_x = pen_x0;
					pen_y += vertAdvance;
				}

			}
		}

		// draw last line
		draw_line(im, fe, ch, i1, ch.size(), &pen_x, pen_y, alignment, boxw, fontsize, xscale);
		return im;
	}

	Uint32	glyph_freetype_provider::decode_next_unicode_character(const char** utf8_buffer)
	{
		Uint32	uc;
		char	c;

		// Security considerations:
		//
		// If we hit a zero byte, we want to return 0 without stepping
		// the buffer pointer past the 0.
		//
		// If we hit an "overlong sequence"; i.e. a character encoded
		// in a longer multibyte string than is necessary, then we
		// need to discard the character.  This is so attackers can't
		// disguise dangerous characters or character sequences --
		// there is only one valid encoding for each character.
		//
		// If we decode characters { 0xD800 .. 0xDFFF } or { 0xFFFE,
		// 0xFFFF } then we ignore them; they are not valid in UTF-8.

		// This isn't actually an invalid character; it's a valid char that
		// looks like an inverted question mark.
#define INVALID 0x0FFFD

#define FIRST_BYTE(mask, shift)		\
	uc = (c & (mask)) << (shift);

#define NEXT_BYTE(shift)						\
	c = **utf8_buffer;						\
	if (c == 0) return 0; /* end of buffer, do not advance */	\
	if ((c & 0xC0) != 0x80) return INVALID; /* standard check */	\
	(*utf8_buffer)++;						\
	uc |= (c & 0x3F) << shift;

		c = **utf8_buffer;
		if (c == 0) return 0;	// End of buffer.  Do not advance.

		(*utf8_buffer)++;
		if ((c & 0x80) == 0) return (Uint32)c;	// Conventional 7-bit ASCII.

		// Multi-byte sequences.
		if ((c & 0xE0) == 0xC0)
		{
			// Two-byte sequence.
			FIRST_BYTE(0x1F, 6);
			NEXT_BYTE(0);
			if (uc < 0x80) return INVALID;	// overlong
			return uc;
		}
		else if ((c & 0xF0) == 0xE0)
		{
			// Three-byte sequence.
			FIRST_BYTE(0x0F, 12);
			NEXT_BYTE(6);
			NEXT_BYTE(0);
			if (uc < 0x800) return INVALID;	// overlong
			if (uc >= 0x0D800 && uc <= 0x0DFFF) return INVALID;	// not valid ISO 10646
			if (uc == 0x0FFFE || uc == 0x0FFFF) return INVALID;	// not valid ISO 10646
			return uc;
		}
		else if ((c & 0xF8) == 0xF0)
		{
			// Four-byte sequence.
			FIRST_BYTE(0x07, 18);
			NEXT_BYTE(12);
			NEXT_BYTE(6);
			NEXT_BYTE(0);
			if (uc < 0x010000) return INVALID;	// overlong
			return uc;
		}
		else if ((c & 0xFC) == 0xF8)
		{
			// Five-byte sequence.
			FIRST_BYTE(0x03, 24);
			NEXT_BYTE(18);
			NEXT_BYTE(12);
			NEXT_BYTE(6);
			NEXT_BYTE(0);
			if (uc < 0x0200000) return INVALID;	// overlong
			return uc;
		}
		else if ((c & 0xFE) == 0xFC)
		{
			// Six-byte sequence.
			FIRST_BYTE(0x01, 30);
			NEXT_BYTE(24);
			NEXT_BYTE(18);
			NEXT_BYTE(12);
			NEXT_BYTE(6);
			NEXT_BYTE(0);
			if (uc < 0x04000000) return INVALID;	// overlong
			return uc;
		}
		else
		{
			// Invalid.
			return INVALID;
		}
	}

	const char* glyph_freetype_provider::getFace(const char* path)
	{
		static std::string res;
		FT_Face face = NULL;
		std::string url = m_base_dir + path;
		FT_New_Face(m_lib, url.c_str(), 0, &face);
		if (face)
		{
			res = face->family_name;
			res += ' ';
			res += face->style_name;
			FT_Done_Face(face);
			return res.c_str();
		}
		return NULL;
	}

	bool glyph_freetype_provider::getMetrics(const char* path, float size, float* ascent, float* descent, float* height, float* leading)
	{
		FT_Face face = NULL;
		std::string url = m_base_dir + path;
		FT_New_Face(m_lib, url.c_str(), 0, &face);
		if (face == NULL)
		{
			url += ".ttf";
			FT_New_Face(m_lib, url.c_str(), 0, &face);
		}

		if (face == NULL)
		{
			return false;
		}

		FT_Set_Pixel_Sizes(face, 0, size);

		*ascent = face->size->metrics.ascender >> 6;
		*descent = face->size->metrics.descender >> 6;
		*height = face->size->metrics.height >> 6;
		*leading = *height - *ascent - *descent;

		FT_Done_Face(face);
		return true;
	}


}
