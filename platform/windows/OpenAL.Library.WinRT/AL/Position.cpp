//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <pch.h>
#include "Position.h"


namespace AL {

#pragma region Constructors/Destructors
Position::Position()
:	fX(0),
	fY(0),
	fZ(0)
{
}

Position::Position(float x, float y, float z)
:	fX(x),
	fY(y),
	fZ(z)
{
}

Position::~Position()
{
}

#pragma endregion


#pragma region Public Methods
float Position::GetX() const
{
	return fX;
}

float Position::GetY() const
{
	return fY;
}

float Position::GetZ() const
{
	return fZ;
}

X3DAUDIO_VECTOR Position::ToX3DAUDIO_VECTOR() const
{
	X3DAUDIO_VECTOR xaudioVector;
	xaudioVector.x = fX;
	xaudioVector.y = fY;
	xaudioVector.z = fZ;
	return xaudioVector;
}

#pragma endregion


#pragma region Public Static Functions
Position Position::From(const X3DAUDIO_VECTOR &value)
{
	return Position(value.x, value.y, value.z);
}

#pragma endregion

}	// namespace AL
