//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_EmitterObject_H__
#define _Rtt_EmitterObject_H__

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_DisplayTypes.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderResource.h"

#include "Core/Rtt_Real.h"
#include "Renderer/Rtt_RenderData.h"
#include "Rtt_MUpdatable.h"
#include "Core/Rtt_SharedPtr.h"

#include "Rtt_Matrix.h"
#include "Box2D/Box2D.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
class LuaUserdataProxy;
class TextureResource;
struct EmitterObjectParticle;

#define EMITTER_ABSOLUTE_PARENT ((GroupObject*)-1)
// ----------------------------------------------------------------------------

// The EmitterObject is the base representation of all closed shapes drawn on
// the screen.
class EmitterObject : public DisplayObject, public MUpdatable
{
	Rtt_CLASS_NO_COPIES( EmitterObject )

public:
	typedef EmitterObject Self;
	typedef DisplayObject Super;

public:
	EmitterObject();
	~EmitterObject();

	//! \Return true for success. False for failure.
	bool Initialize( lua_State *L, Display &display );

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
	//// Simulation (tweakable from Lua).
	//
	// JSON attribute: "absolutePosition"
	GroupObject* GetAbsolutePosition() const { return fAbsolutePositionParent; };
	void SetAbsolutePosition( GroupObject* b ){ fAbsolutePositionParent = b; };

	// JSON attribute: "gravityx"
	// JSON attribute: "gravityy"
	b2Vec2 &GetGravity(){ return fGravity; };
	const b2Vec2 &GetGravity() const { return fGravity; };

	// JSON attribute: "startColorRed"
	// JSON attribute: "startColorGreen"
	// JSON attribute: "startColorBlue"
	// JSON attribute: "startColorAlpha"
	Vector4 &GetStartColor(){ return fStartColor; };
	const Vector4 &GetStartColor() const { return fStartColor; };

	// JSON attribute: "startColorVarianceRed"
	// JSON attribute: "startColorVarianceGreen"
	// JSON attribute: "startColorVarianceBlue"
	// JSON attribute: "startColorVarianceAlpha"
	Vector4 &GetStartColorVariance(){ return fStartColorVariance; };
	const Vector4 &GetStartColorVariance() const { return fStartColorVariance; };

	// JSON attribute: "finishColorRed"
	// JSON attribute: "finishColorGreen"
	// JSON attribute: "finishColorBlue"
	// JSON attribute: "finishColorAlpha"
	Vector4 &GetFinishColor(){ return fFinishColor; };
	const Vector4 &GetFinishColor() const { return fFinishColor; };

	// JSON attribute: "finishColorVarianceRed"
	// JSON attribute: "finishColorVarianceGreen"
	// JSON attribute: "finishColorVarianceBlue"
	// JSON attribute: "finishColorVarianceAlpha"
	Vector4 &GetFinishColorVariance(){ return fFinishColorVariance; };
	const Vector4 &GetFinishColorVariance() const { return fFinishColorVariance; };

	// JSON attribute: "startParticleSize"
	float GetStartParticleSize() const { return fStartParticleSize; };
	void SetStartParticleSize( float s ){ fStartParticleSize = s; };

	// JSON attribute: "startParticleSizeVariance"
	float GetStartParticleSizeVariance() const { return fStartParticleSizeVariance; };
	void SetStartParticleSizeVariance( float v ){ fStartParticleSizeVariance = v; };

	// JSON attribute: "finishParticleSize"
	float GetFinishParticleSize() const { return fFinishParticleSize; };
	void SetFinishParticleSize( float s ){ fFinishParticleSize = s; };

	// JSON attribute: "finishParticleSizeVariance"
	float GetFinishParticleSizeVariance() const { return fFinishParticleSizeVariance; };
	void SetFinishParticleSizeVariance( float v ){ fFinishParticleSizeVariance = v; };

	// JSON attribute: "maxRadius"
	float GetMaxRadius() const { return fMaxRadius; };
	void SetMaxRadius( float r ){ fMaxRadius = r; };

	// JSON attribute: "maxRadiusVariance"
	float GetMaxRadiusVariance() const { return fMaxRadiusVariance; };
	void SetMaxRadiusVariance( float v ){ fMaxRadiusVariance = v; };

	// JSON attribute: "minRadius"
	float GetMinRadius() const { return fMinRadius; };
	void SetMinRadius( float r ){ fMinRadius = r; };

	// JSON attribute: "minRadiusVariance"
	float GetMinRadiusVariance() const { return fMinRadiusVariance; };
	void SetMinRadiusVariance( float v ){ fMinRadiusVariance = v; };

	// JSON attribute: "rotatePerSecond"
	float GetRotateDegreesPerSecond() const { return fRotateDegreesPerSecond; };
	void SetRotateDegreesPerSecond( float r ){ fRotateDegreesPerSecond = r; };

	// JSON attribute: "rotatePerSecondVariance"
	float GetRotateDegreesPerSecondVariance() const { return fRotateDegreesPerSecondVariance; };
	void SetRotateDegreesPerSecondVariance( float v ){ fRotateDegreesPerSecondVariance = v; };

	// JSON attribute: "rotationStart"
	float GetRotationStart() const { return fRotationStart; };
	void SetRotationStart( float v ){ fRotationStart = v; };

	// JSON attribute: "rotationStartVariance"
	float GetRotationStartVariance() const { return fRotationStartVariance; };
	void SetRotationStartVariance( float v ){ fRotationStartVariance = v; };

	// JSON attribute: "rotationEnd"
	float GetRotationEnd() const { return fRotationEnd; };
	void SetRotationEnd( float r ){ fRotationEnd = r; };

	// JSON attribute: "rotationEndVariance"
	float GetRotationEndVariance() const { return fRotationEndVariance; };
	void SetRotationEndVariance( float v ){ fRotationEndVariance = v; };

	// JSON attribute: "speed"
	float GetSpeed() const { return fSpeed; };
	void SetSpeed( float v ){ fSpeed = v; };

	// JSON attribute: "speedVariance"
	float GetSpeedVariance() const { return fSpeedVariance; };
	void SetSpeedVariance( float v ){ fSpeedVariance = v; };

	float GetEmissionRateInParticlesPerSeconds() const { return fEmissionRateInParticlesPerSeconds; };
	void SetEmissionRateInParticlesPerSeconds( float v );

	// JSON attribute: "radialAcceleration"
	float GetRadialAcceleration() const { return fRadialAcceleration; };
	void SetRadialAcceleration( float v ){ fRadialAcceleration = v; };

	// JSON attribute: "radialAccelVariance"
	float GetRadialAccelerationVariance() const { return fRadialAccelerationVariance; };
	void SetRadialAccelerationVariance( float v ){ fRadialAccelerationVariance = v; };

	// JSON attribute: "tangentialAcceleration"
	float GetTangentialAcceleration() const { return fTangentialAcceleration; };
	void SetTangentialAcceleration( float v ){ fTangentialAcceleration = v; };

	// JSON attribute: "tangentialAccelVariance"
	float GetTangentialAccelerationVariance() const { return fTangentialAccelerationVariance; };
	void SetTangentialAccelerationVariance( float v ){ fTangentialAccelerationVariance = v; };

	// JSON attribute: "sourcePositionVariancex"
	// JSON attribute: "sourcePositionVariancey"
	b2Vec2 &GetSourcePositionVariance(){ return fSourcePositionVariance; };
	const b2Vec2 &GetSourcePositionVariance() const { return fSourcePositionVariance; };

	// JSON attribute: "angle"
	float GetRotationInDegrees() const { return fRotationInDegrees; };
	void SetRotationInDegrees( float v ){ fRotationInDegrees = v; };

	// JSON attribute: "angleVariance"
	float GetRotationInDegreesVariance() const { return fRotationInDegreesVariance; };
	void SetRotationInDegreesVariance( float v ){ fRotationInDegreesVariance = v; };

	// JSON attribute: "particleLifespan"
	float GetParticleLifespanInSeconds() const { return fParticleLifespanInSeconds; };
	void SetParticleLifespanInSeconds( float v );

	// JSON attribute: "particleLifespanVariance"
	float GetParticleLifespanInSecondsVariance() const { return fParticleLifespanInSecondsVariance; };
	void SetParticleLifespanInSecondsVariance( float v ){ fParticleLifespanInSecondsVariance = v; };

	// JSON attribute: "duration"
	float GetDuration() const { return fDuration; };
	void SetDuration( float v ){ fDuration = v; };
	//
	////

	int GetMaxParticles() const { return fMaxParticles; };

	////
	//
	enum Mapping
	{
		kMapping_Legacy = 0, // Default. Fixed size across all scales.
		kMapping_Rescale, // Rescale particles to absolutePosition.
		kMapping_RescaleX, // Uniform particle rescale, using x-axis as measure.
		kMapping_RescaleY, // Uniform particle rescale, using y-axis as measure.
		kMapping_RescaleMin, // Uniform particle rescale, using shorter axis as measure.
		kMapping_RescaleMax, // Uniform particle rescale, using longer axis as measure.
		kMapping_RescaleMean, // Uniform particle rescale, using average of axis lengths.

		kMapping_Count
	};

	Mapping GetMapping() const { return fMapping; }

	static const char* GetStringForMapping( Mapping mapping );
	static Mapping GetMappingForString( const char* string, Mapping def );

	////
	//
	enum State
	{
		kState_Playing, // Default. Emits particles.
		kState_Stopped, // Doesn't emit particles. Lets currently active particles die off.
		kState_Paused, // Freezes everything in place.

		kState_Count
	};

	int GetState() const { return fState; }

	static const char *GetStringForState( int state );

	void Start();
	void Stop();
	void Pause();
	//
	////

	inline void TransformParticlePosition(const Matrix &spawnTimeTransform, Vertex2 &v) const;

	bool ValidateEmitterParent();

private:

	void _AddParticle(const Matrix &spawnTimeTransform);
	void _RemoveParticle( int particleIndex );
	void _Update( const Display &display );
	void _Cleanup();

	// MUpdatable
	bool fShouldUpdate;

	//// Simulation (tweakable from Lua).
	//
	GroupObject* fAbsolutePositionParent;
	b2Vec2 fGravity;

	Vector4 fStartColor;
	Vector4 fStartColorVariance;
	Vector4 fFinishColor;
	Vector4 fFinishColorVariance;

	float fStartParticleSize;
	float fStartParticleSizeVariance;
	float fFinishParticleSize;
	float fFinishParticleSizeVariance;

	float fMaxRadius;
	float fMaxRadiusVariance;
	float fMinRadius;
	float fMinRadiusVariance;
	float fRotateDegreesPerSecond;
	float fRotateDegreesPerSecondVariance;
	float fRotationStart;
	float fRotationStartVariance;
	float fRotationEnd;
	float fRotationEndVariance;

	float fSpeed;
	float fSpeedVariance;

	float fEmissionRateInParticlesPerSeconds;

	float fRadialAcceleration;
	float fRadialAccelerationVariance;
	float fTangentialAcceleration;
	float fTangentialAccelerationVariance;

	b2Vec2 fSourcePositionVariance;

	float fRotationInDegrees;
	float fRotationInDegreesVariance;

	float fParticleLifespanInSeconds;
	float fParticleLifespanInSecondsVariance;
	//
	////

	//// Simulation.
	//
	int fEmitterType;

	int fMaxParticles;
	float fDuration;

	int fBlendFuncSource;
	int fBlendFuncDestination;
	float fEmitCounter;
	float fElapsedTime;

	std::string fTextureFileName;

	friend struct EmitterObjectParticle;
	//! We're also using this to determine if Initialize() has been called.
	EmitterObjectParticle *fParticles;

	int fParticleCount;
	
	Mapping fMapping;
	State fState;

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

#endif // _Rtt_EmitterObject_H__
