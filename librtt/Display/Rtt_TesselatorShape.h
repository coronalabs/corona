//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TesselatorShape_H__
#define _Rtt_TesselatorShape_H__

#include "Display/Rtt_Tesselator.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Transform;

// ----------------------------------------------------------------------------

class TesselatorShape : public Tesselator
{
	public:
		typedef Tesselator Super;

	public:
		TesselatorShape();

	public:
		virtual void GenerateFill( ArrayVertex2& outVertices ) = 0;
		virtual void GenerateFillTexture( ArrayVertex2& outTexCoords, const Transform& t ) = 0;
		virtual void GenerateFillIndices( ArrayIndex&  outVertices ) {};
		virtual void GenerateStrokeTexture( ArrayVertex2& outTexCoords, int numVertices );
		virtual Geometry::PrimitiveType GetFillPrimitive() const;

		virtual U32 FillVertexCount() const = 0;

		virtual bool SetSelfBounds( Real width, Real height );

	public:
		// For now, assume normalization happens around the center of the texture
		void Normalize( ArrayVertex2& vertices );
		bool ShouldNormalize() const;
		void ResetNormalization();

		void SetNormalizationScaleX( Real newValue ) { fScaleX = newValue; }
		void SetNormalizationScaleY( Real newValue ) { fScaleY = newValue; }

	protected:
		Real GetScaleX() const { return fScaleX; }
		Real GetScaleY() const { return fScaleY; }

	private:
		Real fScaleX;
		Real fScaleY;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TesselatorShape_H__
