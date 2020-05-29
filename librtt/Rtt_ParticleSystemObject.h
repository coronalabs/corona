//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ParticleSystemObject_H__
#define _Rtt_ParticleSystemObject_H__

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_DisplayTypes.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderResource.h"

#include "Core/Rtt_Real.h"
#include "Renderer/Rtt_RenderData.h"
#include "Rtt_MUpdatable.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Core/Rtt_SharedPtr.h"
#include "Rtt_PhysicsTypes.h"

#include "Box2D/Box2D.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
class LuaUserdataProxy;
class PhysicsWorld;
class TextureResource;

// ----------------------------------------------------------------------------

// The ParticleSystemObject is the base representation of all closed shapes drawn on
// the screen.
class ParticleSystemObject : public DisplayObject, public MUpdatable
{
	Rtt_CLASS_NO_COPIES( ParticleSystemObject )

public:
	typedef ParticleSystemObject Self;
	typedef DisplayObject Super;

public:
	ParticleSystemObject();
	~ParticleSystemObject();

	//! \Return true for success. False for failure.
	bool Initialize( lua_State *L,
						Display &display );

public:
	// MDrawable
	virtual void Draw( Renderer& renderer ) const;
	virtual void GetSelfBounds( Rect& rect ) const;
	virtual void Prepare( const Display& display );

public:
	// MUpdatable
	void QueueUpdate(){ fShouldUpdate = true; }

public:
	virtual bool HitTest( Real contentX, Real contentY );
	virtual ShaderResource::ProgramMod GetProgramMod() const;

	virtual bool CanCull() const;

public:
	virtual const LuaProxyVTable& ProxyVTable() const;

public:

	void ApplyForce( lua_State *L );
	void ApplyLinearImpulse( lua_State *L );
	void CreateGroup( lua_State *L );
	void CreateParticle( lua_State *L );
	int DestroyParticlesInShape( lua_State *L );
	int QueryRegion( lua_State *L );
	int RayCast( lua_State *L );

	void SetParticleRenderRadiusInContentUnits( float r ){ fRenderRadiusInPixels.x = fRenderRadiusInPixels.y = r; }
	float GetParticleRenderRadiusInContentUnits() const { return fRenderRadiusInPixels.x; }

	b2ParticleSystem *GetB2ParticleSystem() const { Rtt_ASSERT( fParticleSystem ); return fParticleSystem; }

private:

	void _InitializeFromLua_ParticleSystemDef( lua_State *L,
												b2ParticleSystemDef &particleSystemDef );
	void _InitializeFromLua_ParticleDef( lua_State *L,
											b2ParticleDef &particleDef );
	void _InitializeFromLua_ParticleGroupDef( lua_State *L,
												b2ParticleGroupDef &particleGroupDef,
												b2PolygonShape &polygonDef,
												b2CircleShape &circleDef,
												b2PolygonShapePtrVector &polyVec );

	void _CopyAllParticlesFromParticleSystemToVertexData();
	void _CopyParticleFromParticleSystemToVertexData( b2Vec2 &base_position,
														b2ParticleColor &color,
														Geometry::Vertex *output_vertices );

	void _ResizeRenderBuffers();

	void _Update( const Display &display );
	void _Cleanup();

	int _CommonRayCast( lua_State *L,
						b2RayCastCallback *callback );

	// MUpdatable
	bool fShouldUpdate;

	//// Simulation.
	//
	float fWorldScaleInPixelsPerMeter;
	float fWorldScaleInMetersPerPixel;
	b2Vec2 fRenderRadiusInPixels;
	b2World *fWorld;
	const PhysicsWorld *fPhysics;

	b2ParticleSystem *fParticleSystem;
	//
	////

	//// Rendering.
	//
	// When we refactor the code to support non-default shaders,
	// we'll replace fTextureResource and fShader by BitmapPaint.

	SharedPtr< TextureResource > fTextureResource;

	RenderData fData;

	// We DON'T own fShader, but we CAN'T make it "const" because
	// we're calling Prepare() on it, which ISN'T a "const" function.
	Shader *fShader;
	//
	////
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ParticleSystemObject_H__
