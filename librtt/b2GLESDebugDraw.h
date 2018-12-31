/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* iPhone port by Simon Oliver - http://www.simonoliver.com - http://www.handcircus.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef RENDER_H
#define RENDER_H

#include "Box2D/Box2D.h"

#include "Renderer/Rtt_RenderData.h"

struct b2AABB;

// ----------------------------------------------------------------------------

namespace Rtt
{
// ----------------------------------------------------------------------------

class Display;
class PhysicsWorld;
class Renderer;
class Shader;

// ----------------------------------------------------------------------------

// This class implements debug drawing callbacks that are invoked
// inside b2World::Step.
class b2GLESDebugDraw : public b2Draw
{
	public:
		b2GLESDebugDraw( Display &display );
		virtual ~b2GLESDebugDraw();

	protected:
		void Begin( const PhysicsWorld& physics, Renderer &renderer );
		void End();

	public:
		void DrawDebugData( const PhysicsWorld& physics, Renderer &renderer );

	protected:
		void DrawShape( b2Fixture* fixture, const b2Transform& xf, const b2Color& color);
		void DrawJoint( b2Joint* joint );
		void DrawParticleSystem( const b2ParticleSystem& system );

	public:
		// b2Draw.

		virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

		virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);


		virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);

		virtual void DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count);

		void DrawParticlesOffset( const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count, const b2Vec2 *offset );

		virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);

		virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);

		virtual void DrawTransform(const b2Transform& xf);

		virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);

		virtual void DrawString(int x, int y, const char* string, ...);

		virtual void DrawAABB(b2AABB* aabb, const b2Color& color);

	public:

		void DrawCircle( bool fill_body,
							const b2Vec2& center,
							float32 radius,
							const b2Vec2 *optionalAxis,
							const b2Color& color,
							const b2Vec2 *optionalOffset );

	private:

		void _SetVerticesUsed( int32 vertexCount );

		void _DrawPolygon( bool fill_body,
							const b2Vec2* vertices,
							int32 vertexCount,
							const b2Color& color );

		//! fRenderer and fScale are only valid between Begin() and End().
		Renderer *fRenderer;
		float fPixelsPerMeter;
		float fMetersPerPixel;

		RenderData fData;
		Shader *fShader;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif
