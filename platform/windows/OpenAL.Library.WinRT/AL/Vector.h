//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <x3daudio.h>


namespace AL {

/// <summary>Represents a 3 dimensional vector providing the magnitudes along the x, y, and z axes.</summary>
class Vector
{
	public:
		/// <summary>Creates a new vector initialized to &lt;0,0,0&gt;.</summary>
		Vector();

		/// <summary>Creates a new vector with the given values.</summary>
		/// <param name="x">The vector's magnitude along the x-axis direction.</param>
		/// <param name="y">The vector's magnitude along the y-axis direction.</param>
		/// <param name="z">The vector's magnitude along the z-axis direction.</param>
		Vector(float x, float y, float z);

		/// <summary>Destroys this object.</summary>
		virtual ~Vector();

		/// <summary>Gets the vector's magnitude along the x-axis direction.</summary>
		/// <returns>Returns the x-axis magnitude of the vector.</returns>
		float GetX() const;

		/// <summary>Gets the vector's magnitude along the y-axis direction.</summary>
		/// <returns>Returns the y-axis magnitude of the vector.</returns>
		float GetY() const;

		/// <summary>Gets the vector's magnitude along the z-axis direction.</summary>
		/// <returns>Returns the z-axis magnitude of the vector.</returns>
		float GetZ() const;

		/// <summary>Converts this vector to an X3DAUDIO_VECTOR structure.</summary>
		/// <returns>Returns an X3DAUDIO_VECTOR structure.</returns>
		X3DAUDIO_VECTOR ToX3DAUDIO_VECTOR() const;

		/// <summary>Creates an AL::Vector object from the given X3DAudio vector object's magnitudes.</summary>
		/// <param name="value">The X3DAudio vector to copy the magnitudes from.</param>
		/// <returns>Returns a vector object containing a copy of the magnitudes provided by the given X3DAudio vector.</returns>
		static Vector From(const X3DAUDIO_VECTOR &value);

	private:
		/// <summary>The vector's magnitude along the x-axis direction.</summary>
		float fX;

		/// <summary>The vector's magnitude along the y-axis direction.</summary>
		float fY;

		/// <summary>The vector's magnitude along the z-axis direction.</summary>
		float fZ;
};

}	// namespace AL
