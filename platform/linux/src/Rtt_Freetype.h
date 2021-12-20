//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef RTT_FREETYPE_H
#define RTT_FREETYPE_H

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Assert.h"
#include "Rtt_LinuxContainer.h"
#include <ft2build.h>
#include <freetype/freetype.h>
#include <string>
#include <map>
#include <vector>

namespace Rtt
{
	// helper
	inline int p2(int n) { int k = 1; while (k < n) k <<= 1; return k; }

	// 8-bit alpha image.
	struct alpha : public ref_counted
	{
		alpha(int width, int height)
			: m_width(width)
			, m_height(height)
			, m_bpp(1)
		{
			m_data = (U8*) malloc(width * height);
			m_pitch = width;
		}

		~alpha()
		{
			free(m_data);
		};

		int m_width;
		int m_height;
		U8* m_data;
		int m_pitch;
		int m_bpp;
	};

	struct glyph_entity
	{
		glyph_entity() :
			m_image(NULL),
			m_width(0),
			m_height(0),
			m_top(0),
			m_left(0),
			m_advance(0)
		{
		}

		~glyph_entity()
		{
			free(m_image);
		}

		U8 *m_image;
		int m_width;
		int m_height;
		int m_top;
		int m_left;
		int m_advance;
	};

	struct face_entity : public ref_counted
	{
		FT_Face m_face;
		std::map<U64, glyph_entity*> m_ge;	// <code, glyph_entity>

		face_entity(FT_Face face) :
			m_face(face)
		{
			assert(face);
		}

		~face_entity()
		{
			FT_Done_Face(m_face);
			for (std::map<U64, glyph_entity*>::iterator it = m_ge.begin(); it != m_ge.end(); ++it)
			{
				delete it->second;
			}
		}
	};

	struct glyph_freetype_provider  : public ref_counted
	{
		glyph_freetype_provider(const char *pathToApp);
		~glyph_freetype_provider();

		smart_ptr<alpha> render_string(const std::string &str, const char *alignment, const std::string &fontname,
		                               bool is_bold, bool is_italic, int fontsize, const std::vector<int> &xleading, const std::vector<int> &yleading,
		                               int w, int h, bool multiline, float xscale, float yscale, float *baseline);

		static const char *getFace(const char *path);
		static bool getMetrics(const char *path, float size, float *ascent, float *descent, float *height, float *leading);

	private:
		struct rect
		{
			rect(int w, int h) : width(w), height(h) {}
			int width;
			int height;
		};

		glyph_entity *load_char_image(face_entity *fe, Uint16 code, int fontsize, float xscale);
		glyph_entity *get_glyph_entity(face_entity *fe, Uint16 code, int fontsize, float xscale);
		rect getBoundingBox(face_entity *fe, std::vector<Uint32> &ch, int fontsize, int vertAdvance, float xscale, int boxw, int boxh);
		int draw_line(alpha *im, face_entity *fe, const std::vector<Uint32> &ch, int i1, int i2, int *pen_x, int pen_y, const char *alignment, int boxw, int fontsize, float xscale);
		face_entity* get_face_entity(const std::string& fontname,	bool is_bold, bool is_italic);
		Uint32	decode_next_unicode_character(const char **utf8_buffer);
		std::map<std::string, smart_ptr<face_entity>> m_face_entity;
		static FT_Library	m_lib;
		float m_scale;
		static std::string m_base_dir;
	};

	glyph_freetype_provider *getGlyphProvider();
	void setGlyphProvider(glyph_freetype_provider *gp);
}

#endif
