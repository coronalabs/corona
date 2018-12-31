// ----------------------------------------------------------------------------
// 
// Position.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include <x3daudio.h>


namespace AL {

/// <summary>Provides the x, y, and z coordinates of a single point in 3 dimensional space.</summary>
class Position
{
	public:
		/// <summary>Creates a new position initialized to (0,0,0).</summary>
		Position();

		/// <summary>Creates a new 3 dimensional position with the given values.</summary>
		/// <param name="x">The x-axis coordinate.</param>
		/// <param name="y">The y-axis coordinate.</param>
		/// <param name="z">The z-axis coordinate.</param>
		Position(float x, float y, float z);

		/// <summary>Destroys this object.</summary>
		virtual ~Position();

		/// <summary>Gets the position's x-axis coordinate in 3 dimensionsal space.</summary>
		/// <returns>Returns the x-axis coordinate.</returns>
		float GetX() const;

		/// <summary>Gets the position's y-axis coordinate in 3 dimensionsal space.</summary>
		/// <returns>Returns the y-axis coordinate.</returns>
		float GetY() const;

		/// <summary>Gets the position's z-axis coordinate in 3 dimensionsal space.</summary>
		/// <returns>Returns the z-axis coordinate.</returns>
		float GetZ() const;

		/// <summary>
		///  <para>Converts this position to an X3DAUDIO_VECTOR structure.</para>
		///  <para>Note that the X3DAudio system often uses an X3DAUDIO_VECTOR to represent positions, not just vectors.</para>
		/// </summary>
		/// <returns>Returns an X3DAUDIO_VECTOR structure.</returns>
		X3DAUDIO_VECTOR ToX3DAUDIO_VECTOR() const;

		/// <summary>
		///  <para>Creates an AL::Position object from the given X3DAudio vector object's magnitudes.</para>
		///  <para>Note that the X3DAudio system often uses an X3DAUDIO_VECTOR to represent positions, not just vectors.</para>
		/// </summary>
		/// <param name="value">The X3DAudio vector to copy the x, y, and z components from.</param>
		/// <returns>
		///  Returns a position object containing a copy of the x, y, and z components provided by the given X3DAudio vector.
		/// </returns>
		static Position From(const X3DAUDIO_VECTOR &value);

	private:
		/// <summary>The x-axis coordinate.</summary>
		float fX;

		/// <summary>The y-axis coordinate.</summary>
		float fY;

		/// <summary>The z-axis coordinate.</summary>
		float fZ;
};

}	// namespace AL
