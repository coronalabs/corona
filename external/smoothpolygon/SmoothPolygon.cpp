#include "SmoothPolygon.h"

// ----------------------------------------------------------------------------

#include <algorithm> // std::reverse().
#include <stdlib.h>

// ----------------------------------------------------------------------------

# define ENABLE_DEBUG_PRINT	( 0 )
#
# if ENABLE_DEBUG_PRINT
#
#	define DEBUG_PRINT( ... )	printf( __VA_ARGS__ )
#
# else // Not ENABLE_DEBUG_PRINT
#
#	define DEBUG_PRINT( ... )
#
# endif // ENABLE_DEBUG_PRINT

// ----------------------------------------------------------------------------

namespace // anonymous namespace.
{

// ----------------------------------------------------------------------------

	enum DirectionType
	{
		DirectionNone,
		DirectionUp,
		DirectionLeft,
		DirectionDown,
		DirectionRight,
	};

	struct PngBuffer {
		const unsigned char *buffer;
		int width;
		int height;
		void Decode( const unsigned char *buffer_,
						int width_,
						int height_ )
		{
			buffer = buffer_;
			width = width_;
			height = height_;
		}
		bool IsValid(int x, int y) {
			if (x < 0 || x >= width || y < 0 || y >= height) {
				return false;
			}
			return true;
		}
		
		// NOTE: This was added so that we could account for different pixel buffer formats
		unsigned int Alpha(int x, int y, int channelIndex)
		{
			if (!IsValid(x, y)) {
				return 0;
			}

			return buffer[4 * y * width + 4 * x + channelIndex];
		}

		// This was from the original, but it was totally broken
		// b/c it hard-coded the rgba channel indices.
		/*
		void At(int x, int y, unsigned int &r, unsigned int &g, unsigned int &b, unsigned int &a) {
			if (!IsValid(x, y)) {
				return;
			}
			 r = buffer[4 * y * width + 4 * x + 0]; //red
			 g = buffer[4 * y * width + 4 * x + 1]; //green
			 b = buffer[4 * y * width + 4 * x + 2]; //blue
			 a = buffer[4 * y * width + 4 * x + 3]; //alpha
		}
		*/
	};

	class MarchingSquare {
	public:
		void Process( const unsigned char *buffer,
						int width,
						int height,
						b2Vec2Vector *vertices,
						int alphaChannelIndex );

	private:
		bool findStartPosition(int& startX, int& startY);
		bool isPixelSolid(int x, int y);
		void step(int x, int y);
		void walk(int startX, int startY);
	private:
		PngBuffer	m_png;
		int			m_previousStep;
		int			m_nextStep;
		b2Vec2Vector *m_result;
		int			m_alphaChannelIndex;
	};

	void MarchingSquare::Process( const unsigned char *buffer,
									int width,
									int height,
									b2Vec2Vector *vertices,
									int alphaChannelIndex )
	{
		m_png.Decode( buffer,
						width,
						height );
		m_alphaChannelIndex = alphaChannelIndex;
		m_result = vertices;
		int startX, startY;
		if( ! findStartPosition(startX, startY) )
		{
			return;
		}
		walk(startX, startY);
	}

	bool MarchingSquare::findStartPosition( int& startX, int& startY )
	{
		for (int y=0; y<m_png.height; y++) {
			for (int x=0; x<m_png.width; x++) {
				if (isPixelSolid(x, y)) {
					startX = x;
					startY = y;
					return true;
				}
			}
		}
		return false;
	}

	bool MarchingSquare::isPixelSolid( int x, int y )
	{
		if (!m_png.IsValid(x, y)) {
			return false;
		}
		unsigned int a = m_png.Alpha(x, y, m_alphaChannelIndex);

		if (a > 0) {
			return true;
		}
		return false;
	}

	void MarchingSquare::step( int x, int y )
	{
		bool upLeft = isPixelSolid(x-1, y-1);
		bool upRight = isPixelSolid(x, y-1);
		bool downLeft = isPixelSolid(x-1, y);
		bool downRight = isPixelSolid(x, y);

		m_previousStep = m_nextStep;

		int state = 0;
		if (upLeft) {
			state |= 1;
		}
		if (upRight) {
			state |= 2;
		}
		if (downLeft) {
			state |= 4;
		}
		if (downRight) {
			state |= 8;
		}

		switch (state) {
		case 1: m_nextStep = DirectionUp; break;
		case 2: m_nextStep = DirectionRight; break;
		case 3: m_nextStep = DirectionRight; break;
		case 4: m_nextStep = DirectionLeft; break;
		case 5: m_nextStep = DirectionUp; break;
		case 6: 
			{
				if (m_previousStep == DirectionUp) {
					m_nextStep = DirectionLeft;
				} else {
					m_nextStep = DirectionRight;
				}
			}
			break;
		case 7: m_nextStep = DirectionRight; break;
		case 8: m_nextStep = DirectionDown; break;
		case 9: 
			{
				if (m_previousStep == DirectionRight) {
					m_nextStep = DirectionUp;
				} else {
					m_nextStep = DirectionDown;
				}
			}
			break;
		case 10: m_nextStep = DirectionDown; break;
		case 11: m_nextStep = DirectionDown; break;
		case 12: m_nextStep = DirectionLeft; break;
		case 13: m_nextStep = DirectionUp; break;
		case 14: m_nextStep = DirectionLeft; break;

		default:
			m_nextStep = DirectionNone;
			break;
		}
	}

	void MarchingSquare::walk( int startX, int startY )
	{
		if (startX < 0) 
			startX = 0;
		if (startX > m_png.width)
			startX = m_png.width;
		if (startY < 0)
			startY = 0;
		if (startY > m_png.height)
			startY = m_png.height;

		int x = startX;
		int y = startY;
		while (true) {
			step(x, y);
			if (m_png.IsValid(x, y)) {
				m_result->push_back( b2Vec2( (float)x, (float)y ) );
			}
			switch (m_nextStep)
			{
			case DirectionUp: y--; break;
			case DirectionLeft: x--; break;
			case DirectionDown: y++; break;
			case DirectionRight: x++; break;
			default:
				break;
			}
			if (x == startX && y == startY) {
				break;
			}
		}
	}

	float findPerpendicularDistance(b2Vec2 p, b2Vec2 p1, b2Vec2 p2) {
		float result = 0.0f;
		if (p1.x == p2.x) {
			result = fabsf(p.x - p1.x);
		} else {
			float slope = (p2.y - p1.y) / (p2.x - p1.x);
			float intercept = p1.y - (slope * p1.x);
			result = fabs(slope * p.x - p.y + intercept) / sqrt(pow(slope, 2) + 1.0f);
		}
		return result;
	}

	// This implements the "RamerâDouglasâPeucker" algorithm for reducing the
	// number of points in a curve that is approximated by a series of points.
	// See:
	// http://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
	b2Vec2Vector RDP( const b2Vec2Vector& points, float epsilon )
	{
		if (points.size() < 3)
			return points;
		b2Vec2 firstPoint = points[0];
		b2Vec2 lastPoint = points[points.size() - 1];
		int index = -1;
		float dist = 0.0f;

		for (size_t i=1; i<points.size()-1; i++) {
			float cDist = findPerpendicularDistance(points[i], firstPoint, lastPoint);
			if (cDist > dist) {
				dist = cDist;
				index = (int) i;
			}
		}
		if (dist > epsilon) {
			b2Vec2Vector l1, l2;
			l1.insert(l1.end(), points.begin(), points.begin() + index + 1);
			l2.insert(l2.end(), points.begin() + index, points.end());
			b2Vec2Vector r1 = RDP(l1, epsilon);
			b2Vec2Vector r2 = RDP(l2, epsilon);
			r1.pop_back();
			r1.insert(r1.end(), r2.begin(), r2.end());
			return r1;
		} else {
			b2Vec2Vector ret;
			ret.push_back(firstPoint);
			ret.push_back(lastPoint);
			return ret;
		}
		return b2Vec2Vector();
	}

// ----------------------------------------------------------------------------

void getProperBuffer( const unsigned char *input_buffer,
						int subregion_start_x_in_pixels,
						int subregion_start_y_in_pixels,
						int subregion_width_in_pixels,
						int subregion_height_in_pixels,
						int total_width_in_pixels,
						int total_height_in_pixels,
						float epsilon,
						unsigned char **output_buffer,
						int &output_buffer_width_in_pixels,
						int &output_buffer_height_in_pixels )
{
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	//
	// IMPORTANT: This function has TWO purpose:
	//
	// (1) Support CoronaSDK ImageSheets by allowing the specification of subregions.
	//
	// (2) Working around a problem where an edge in the very last pixel of
	// the input_buffer (lower right corner of the texture) would be ignored.
	//
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	// 4: bytes per pixel (RGBA).
	size_t input_buffer_read_position_in_bytes = ( 4 * ( subregion_start_x_in_pixels + ( subregion_start_y_in_pixels * total_width_in_pixels ) ) );
	size_t output_buffer_write_position_in_bytes = 0;

	output_buffer_width_in_pixels = ( subregion_width_in_pixels + 1 );
	output_buffer_height_in_pixels = ( subregion_height_in_pixels + 1 );

	// 4: bytes per pixel (RGBA).
	size_t subregion_width_in_bytes = ( subregion_width_in_pixels * 4 );
	size_t skip_input_bytes_to_next_row = ( ( total_width_in_pixels * 4 ) - subregion_width_in_bytes );

	// 4: bytes per pixel (RGBA).
	*output_buffer = (unsigned char *)malloc( output_buffer_width_in_pixels * output_buffer_height_in_pixels * 4 );
	{
		for( int y = 0;
				y < subregion_height_in_pixels;
				++y )
		{
			// Copy the input buffer to the output buffer.
			{
				memcpy( ( *output_buffer + output_buffer_write_position_in_bytes ),
						( input_buffer + input_buffer_read_position_in_bytes ),
						subregion_width_in_bytes );

				output_buffer_write_position_in_bytes += subregion_width_in_bytes;
				input_buffer_read_position_in_bytes += subregion_width_in_bytes;
			}

			// Clear the last column.
			{
				// 4: bytes per pixel (RGBA).
				memset( ( *output_buffer + output_buffer_write_position_in_bytes ),
						0,
						4 );

				// 4: bytes per pixel (RGBA).
				output_buffer_write_position_in_bytes += 4;
			}

			// Set the read position to the begining
			// of the next row in the input buffer.
			input_buffer_read_position_in_bytes += skip_input_bytes_to_next_row;
		}

		// Clear the last row of the output buffer.
		// 4: bytes per pixel (RGBA).
		memset( ( *output_buffer + output_buffer_write_position_in_bytes ),
				0,
				( output_buffer_width_in_pixels * 4 ) );
	}
}

// ----------------------------------------------------------------------------

} // anonymous namespace.

// ----------------------------------------------------------------------------

b2Vec2Vector MakeSmoothPolygon( const unsigned char *buffer,
								int subregion_start_x_in_pixels,
								int subregion_start_y_in_pixels,
								int subregion_width_in_pixels,
								int subregion_height_in_pixels,
								int total_width_in_pixels,
								int total_height_in_pixels,
								float epsilon,
								int alphaChannelOffset )
{
	int proper_buffer_width_in_pixels = 0;
	int proper_buffer_height_in_pixels = 0;
	unsigned char *proper_buffer = NULL;

	getProperBuffer( buffer,
						subregion_start_x_in_pixels,
						subregion_start_y_in_pixels,
						subregion_width_in_pixels,
						subregion_height_in_pixels,
						total_width_in_pixels,
						total_height_in_pixels,
						epsilon,
						&proper_buffer,
						proper_buffer_width_in_pixels,
						proper_buffer_height_in_pixels );

	MarchingSquare ms;

	b2Vec2Vector tmp;

	ms.Process( proper_buffer,
				proper_buffer_width_in_pixels,
				proper_buffer_height_in_pixels,
				&tmp,
				alphaChannelOffset );

	free( proper_buffer );

	b2Vec2Vector result = RDP(tmp, epsilon);

	// It's NECESSARY to reverse the order of the vertices.
	// This is to match the output of MakeSmoothPolygon()
	// with the vertex order expected by b2Separator.
	//
	// Counter-clockwise -> Clockwise vertex order.
	std::reverse( result.begin(),
					result.end() );

#	if ENABLE_DEBUG_PRINT

		DEBUG_PRINT( "%s() found %d edges:\n",
						__FUNCTION__,
						result.size() );

		// Print all vertices part of the outline.
		{
			for( size_t i = 0;
					i < result.size();
					++i )
			{
				DEBUG_PRINT( "%03d %3.1f %3.1f\n",
								i,
								result[ i ].x,
								result[ i ].y );
			}
		}

#	endif // ENABLE_DEBUG_PRINT

	return result;
}
