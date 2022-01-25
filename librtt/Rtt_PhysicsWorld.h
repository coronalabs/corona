//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PhysicsWorld_H__
#define _Rtt_PhysicsWorld_H__

// ----------------------------------------------------------------------------

class b2Body;
class b2DebugDraw;
class b2World;
class b2DestructionListener;

namespace Rtt
{

class b2GLESDebugDraw;
class PhysicsContactListener;
class Runtime;
class Renderer;

// ----------------------------------------------------------------------------

class PhysicsWorld
{
	public:
		enum
		{
			// Physics-related
			kIsWorldRunning						= 0x01,
			kCollisionListenerExists			= 0x02,
			kPreCollisionListenerExists			= 0x04,
			kPostCollisionListenerExists		= 0x08,
			kParticleCollisionListenerExists	= 0x10,
		};

		typedef U32 Properties;

	public:
		PhysicsWorld( Rtt_Allocator& allocator );
		~PhysicsWorld();

	public:
		void Initialize( float frameInterval );

	public:
		void WillDestroyDisplay();

	public:
		void StartWorld( Runtime& runtime, bool noSleep );
		void PauseWorld();
		void StopWorld();
		b2World* GetWorld() const { return fWorld; }
		b2Body* GetGroundBody() const { return fGroundBody; }

	public:
		Rtt_Allocator *Allocator() const { return & fAllocator; }
		bool IsProperty( Properties mask ) const { return (fProperties & mask) != 0; }
		void ToggleProperty( Properties mask ) { fProperties ^= mask; }
		void SetProperty( Properties mask, bool value );

		// Default is 30 (content) pixels per meter so the range [3, 300] pixels
		// maps to [0.1, 10] meters (the optimal length scale range for Box2D)
		Real GetPixelsPerMeter() const { return fPixelsPerMeter; }
		Real GetMetersPerPixel() const { return ( 1.0f / fPixelsPerMeter ); }
		void SetPixelsPerMeter( Real newValue ) { fPixelsPerMeter = newValue; }
		S32 GetVelocityIterations() const { return fVelocityIterations; }
		void SetVelocityIterations( S32 newValue ) { fVelocityIterations = newValue; }
		S32 GetPositionIterations() const { return fPositionIterations; }
		void SetPositionIterations( S32 newValue ) { fPositionIterations = newValue; }

		float GetTimeStep() const { return fTimeStep; }
		void SetTimeStep( float newValue );

		float GetTimeScale() const { return fTimeScale; }
		void SetTimeScale( float newValue ) { fTimeScale = newValue; }

		int GetNumSteps() const { return fNumSteps; }
		void SetNumSteps( S32 newValue ) { fNumSteps = newValue; }


	public:
		void SetReportCollisionsInContentCoordinates( bool enabled );
		bool GetReportCollisionsInContentCoordinates() const;

		void SetLuaAssertEnabled( bool enabled );
		bool GetLuaAssertEnabled() const;

		void SetAverageCollisionPositions( bool enabled );
		bool GetAverageCollisionPositions() const;

	public:
		void DebugDraw( Renderer &renderer ) const;

	public:
		void StepWorld( double elapsedMS );

	private:
		Rtt_Allocator& fAllocator;
		b2GLESDebugDraw *fWorldDebugDraw;
		b2DestructionListener *fWorldDestructionListener;
		PhysicsContactListener *fWorldContactListener;

		U32 fProperties;
		b2World *fWorld;
		Real fPixelsPerMeter;
		b2Body *fGroundBody;
		S32 fVelocityIterations;
		S32 fPositionIterations;
		float fFrameInterval;
		float fTimeStep;
		float fTimeScale;
		float fTimePrevious;
		float fTimeRemainder;

		S32 fNumSteps;

		//! false: Contact points are reported in local-space.
		//! true: Contact points are reported in content-space.
		bool fReportCollisionsInContentCoordinates;

		//! false: Rtt_LuaAssert are disabled.
		//! true: Rtt_LuaAssert are enabled.
		bool fLuaAssertEnabled;

		//! It's common for Box2D to report multiple contact points during
		//! a single iteration of the simulation.
		//!
		//! How a set of contact points should be handled is game-specific.
		//! Therefore, we have to provide the ability to either get the first
		//! point from the set, or the average of the set.
		//!
		//! By default, we return only the first point.
		//!
		//! false: The point of contact reported is the first one reported by Box2D. The order is arbitrary.
		//! true: The point of contact reported is the average of all contact points.
		bool fAverageCollisionPositions;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PhysicsWorld_H__
