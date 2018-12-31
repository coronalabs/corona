#ifndef _SmoothPolygon_H__
#define _SmoothPolygon_H__

// ----------------------------------------------------------------------------

#ifdef __cplusplus

// ----------------------------------------------------------------------------

#include "Rtt_PhysicsTypes.h"

// ----------------------------------------------------------------------------

// Note: We could add an option to center the output of MakeSmoothPolygon().
// Currently, the origin of the result is the first vertex of the first edge
// encountered.
b2Vec2Vector MakeSmoothPolygon( const unsigned char *buffer,
								int subregion_start_x_in_pixels,
								int subregion_start_y_in_pixels,
								int subregion_width_in_pixels,
								int subregion_height_in_pixels,
								int total_width_in_pixels,
								int total_height_in_pixels,
								float epsilon,
								int alphaChannelOffset );

// ----------------------------------------------------------------------------

#endif // __cplusplus

// ----------------------------------------------------------------------------

#endif // _SmoothPolygon_H__
