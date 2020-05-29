//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <pch.h>
#include "Vector.h"


namespace AL {

#pragma region Constructors/Destructors
Vector::Vector()
:	fX(0),
	fY(0),
	fZ(0)
{
}

Vector::Vector(float x, float y, float z)
:	fX(x),
	fY(y),
	fZ(z)
{
}

Vector::~Vector()
{
}

#pragma endregion


#pragma region Public Methods
float Vector::GetX() const
{
	return fX;
}

float Vector::GetY() const
{
	return fY;
}

float Vector::GetZ() const
{
	return fZ;
}

X3DAUDIO_VECTOR Vector::ToX3DAUDIO_VECTOR() const
{
	X3DAUDIO_VECTOR xaudioVector;
	xaudioVector.x = fX;
	xaudioVector.y = fY;
	xaudioVector.z = fZ;
	return xaudioVector;
}

#pragma endregion


#pragma region Public Static Functions
Vector Vector::From(const X3DAUDIO_VECTOR &value)
{
	return Vector(value.x, value.y, value.z);
}

#pragma endregion

}	// namespace AL
