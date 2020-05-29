//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Renderer_Rtt_Matrix_Renderer_H__
#define _Renderer_Rtt_Matrix_Renderer_H__

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Real.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Define row order. OpenGL is column-major, DirectX is row-major.
#define COLUMN_MAJOR_MATRIX 1

// Create a 3x3 translation matrix using the given offsets.
void Translation3x3( Real x, Real y, Real* result );

// Create a 3x3 scale matrix using the given scale factors.
void Scale3x3( Real x, Real y, Real* result );

// Create a 3x3 rotation matrix using the given angle, in degrees.
void Rotation3x3( Real angle, Real* result );

// Multiply the given 3x3 matrices together. Note, this function
// assumes that the result matrix has already been allocated.
void Multiply3x3( const Real* m0, const Real* m1, Real* result );

// Multiply the given 4x4 matrices together. Note, this function
// assumes that the result matrix has already been allocated.
void Multiply4x4( const Real* m0, const Real* m1,  Real* result );

// Multiply the given 3 component vector and 3x3 matrix. Note, this
// function assumes the result vector has already been allocated.
void MultiplyVec3Mat3( const Real* v, const Real* m, Real* result );

// Multiply the given 4 component vector and 4x4 matrix. Note, this
// function assumes the result vector has already been allocated.
void MultiplyVec4Mat4( const Real* v, const Real* m, Real* result );

// Converts the the given 4 component vector, assumed to be in clip
// space, through NDC, and into 2 component window coordinates.
void ClipToWindow( const Real* input, S32 viewportX, S32 viewportY, Real* result );

//
void CreateViewMatrix( Real ex, Real ey, Real ez, // Eye position.
	                   Real cx, Real cy, Real cz, // Target position.
					   Real ux, Real uy, Real uz, // Up vector.
					   Real* result );

//
void CreateOrthoMatrix( Real left, Real right, Real bottom, Real top, Real zNear, Real zFar, Real* result );

//
void CreatePerspectiveMatrix( Real fovy, Real aspectRatio, Real zNear, Real zFar, Real* result );

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Renderer_Rtt_Matrix_Renderer_H__
