//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#ifdef Rtt_PHYSICS

#include "Rtt_LuaLibPhysics.h"

#include "Display/Rtt_ClosedPath.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_ImageFrame.h"
#include "Display/Rtt_ImageSheetPaint.h"
#include "Display/Rtt_LineObject.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Display/Rtt_ShapeObject.h"
#include "Display/Rtt_ShapePath.h"
#include "Display/Rtt_TesselatorPolygon.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Rtt_DisplayObjectExtensions.h"
#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_MPlatform.h"
#include "Rtt_PhysicsContact.h"
#include "Rtt_PhysicsJoint.h"
#include "Rtt_PhysicsTypes.h"
#include "Rtt_PhysicsWorld.h"
#include "Rtt_Runtime.h"
#include "b2Separator.h"
#include "b2GLESDebugDraw.h"

#include "Rtt_ParticleSystemObject.h"

#include "CoronaLua.h"

#include <list>

# define ENABLE_DEBUG_PRINT	( 0 )
#
# if ENABLE_DEBUG_PRINT
#
#	define DEBUG_PRINT( ... )	Rtt_Log( __VA_ARGS__ )
#
# else // Not ENABLE_DEBUG_PRINT
#
#	define DEBUG_PRINT( ... )
#
# endif // ENABLE_DEBUG_PRINT

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

bool
LuaLibPhysics::IsWorldLocked( lua_State *L, const char caller[] )
{
    bool result = false;
    
    const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
    b2World *world = physics.GetWorld();
    
    if ( ! world )
    {
        CoronaLuaError(L, "physics.start() must be called before %s", caller);
        result = true; // Behave as if locked to avoid accessing NULL physics world
    }
    // Emit 'errorMsg' if attempting to access World while locked
    else if ( world->IsLocked() )
    {
        CoronaLuaError(L, "%s cannot be called when the world is locked and in the middle of number crunching, such as during a collision event", caller);
        result = true;
    }
    
    return result;
}

bool
LuaLibPhysics::IsWorldValid( lua_State *L, const char caller[] )
{
    bool result = true;
    const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
    b2World *world = physics.GetWorld();
    
    if ( world == NULL )
    {
        CoronaLuaError(L, "physics.start() must be called before %s", caller);
        result = false;
    }

    return result;
}

static const U32 kGroundBodyData[1] = { 8675309 };

const void*
LuaLibPhysics::GetGroundBodyUserdata()
{
	return kGroundBodyData;
}

static int
start( lua_State *L )
{
	Runtime *runtime = LuaContext::GetRuntime( L );
	PhysicsWorld& physics = runtime->GetPhysicsWorld();

	bool noSleep = lua_toboolean( L, 1 );
	physics.StartWorld( * runtime, noSleep );

	Rtt_ASSERT( physics.GetWorld() );

	return 0;
}


static int
pause( lua_State *L )
{
    if (LuaLibPhysics::IsWorldValid(L, "physics.pause()"))
    {
        PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
        physics.PauseWorld();
    }

	return 0;
}


static int
stop( lua_State *L )
{
	bool result = ! LuaLibPhysics::IsWorldLocked( L, "physics.stop()" );

	if ( result )
	{
		PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		physics.StopWorld();
	}

	// return whether succeeded or not
	lua_pushboolean( L, result );
	return 1;
}


static int
setGravity( lua_State *L )
{
	int curArg = 1;

	if ( lua_isnumber( L, curArg )
		 && lua_isnumber( L, curArg+1 ) )
	{
        if (LuaLibPhysics::IsWorldValid(L, "physics.setGravity()"))
        {
			const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
			b2World *world = physics.GetWorld();

			world->SetGravity( b2Vec2( luaL_toreal( L, curArg ),
										luaL_toreal( L, ( curArg + 1 ) ) ) );
        }
	}
    else
    {
        CoronaLuaError(L, "physics.setGravity() requires 2 parameters (number, number)");
    }
	return 0;
}


static int
getGravity( lua_State *L )
{
	const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
	b2World *world = physics.GetWorld();

    if (LuaLibPhysics::IsWorldValid(L, "physics.getGravity()"))
    {
        b2Vec2 gravity = world->GetGravity();

        lua_pushnumber( L, Rtt_FloatToReal( gravity.x ) );
        lua_pushnumber( L, Rtt_FloatToReal( gravity.y ) );
        
        return 2;
    }
    else
    {
        return 0;
    }
}

static int
SetReportCollisionsInContentCoordinates( lua_State *L )
{
    if (! lua_isboolean(L, 1))
    {
        CoronaLuaError(L, "physics.setReportCollisionsInContentCoordinates() requires 1 parameter (boolean)");
        
        return 0;
    }
    
	PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

	physics.SetReportCollisionsInContentCoordinates( lua_toboolean( L, 1 ) );

	return 0;
}

static int
GetReportCollisionsInContentCoordinates( lua_State *L )
{
	const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

	lua_pushboolean( L, physics.GetReportCollisionsInContentCoordinates() );

	return 1;
}

static int
SetDebugErrorsEnabled( lua_State *L )
{
	PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

	physics.SetLuaAssertEnabled( lua_toboolean( L, 1 ) );

	return 0;
}

static int
GetDebugErrorsEnabled( lua_State *L )
{
	const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

	lua_pushboolean( L, physics.GetLuaAssertEnabled() );

	return 1;
}

namespace // anonymous namespace.
{
	void add_hit_to_table_of_raycasting_results( lua_State *L,
													float pixels_per_meter,
													size_t &result_index,
													DisplayObject *optional_display_object,
													const b2Vec2& point,
													const b2Vec2& normal,
													const float32 fraction )
	{
		if( ! result_index )
		{
			// We're handling the first result.
			lua_newtable( L );
		}

		// Meters to pixels.
		b2Vec2 p_in_pixels( point );
		p_in_pixels *= pixels_per_meter;

		// Add the result.
		//
		// results[n].object : The DisplayObject of this hit.
		// results[n].position.x : The X coordinate in content-space of this hit.
		// results[n].position.y : The Y coordinate in content-space of this hit.
		// results[n].normal.x : The X coordinate in content-space of this hit.
		// results[n].normal.y : The Y coordinate in content-space of this hit.
		// results[n].fraction : The fraction (0..1), along the ray, where the hit is located.
		{
			// We can use lua_newtable() here, but we know exactly how
			// many records we'll put in the table (4). So we use
			// lua_createtable() for better performance.
			lua_createtable( L, 0, 4 );
			{
				// 1st element: Hit object.
				if ( optional_display_object )
				{
					optional_display_object->GetProxy()->PushTable( L );
				}
				else
				{
					lua_pushnil( L );
				}
				lua_setfield( L, -2, "object" );

				// 2nd element: Hit position.
				//
				// We can use lua_newtable() here, but we know exactly how
				// many records we'll put in the table (2). So we use
				// lua_createtable() for better performance.
				lua_createtable( L, 0, 2 );
				{
					// -2 : This is used to refer to the table that was
					// created above, using lua_createtable().
					//
					// lua_createtable() pushes a table to the top of the
					// stack (at position "-1"). Then lua_pushnumber()
					// pushes a number to the top of the stack (the new
					// element at position "-1"). Now the table is right
					// below the top of the stack (at position "-2").
					//
					// An alternative is to use lua_gettop() immediately
					// after the creation of the above table. The index
					// returned will refer to the table, and can be passed
					// to every calls of lua_setfield() instead of "-2".
					lua_pushnumber( L, p_in_pixels.x );
					lua_setfield( L, -2, "x" );

					lua_pushnumber( L, p_in_pixels.y );
					lua_setfield( L, -2, "y" );
				}
				lua_setfield( L, -2, "position" );

				// 3rd element: Hit surface normal.
				//
				// We can use lua_newtable() here, but we know exactly how
				// many records we'll put in the table (2). So we use
				// lua_createtable() for better performance.
				lua_createtable( L, 0, 2 );
				{
					// "normal" is a unit vector (vector of length one)
					// without units (just a direction).
					lua_pushnumber( L, normal.x );
					lua_setfield( L, -2, "x" );

					lua_pushnumber( L, normal.y );
					lua_setfield( L, -2, "y" );
				}
				lua_setfield( L, -2, "normal" );

				// 4th element: Hit fraction along the ray.
				{
					lua_pushnumber( L, fraction );
					lua_setfield( L, -2, "fraction" );
				}

				// Important: If we modify the total number of items
				// pushed on this Lua table, we HAVE to update the
				// number of pre-allocated records in the
				// lua_createtable() call above.
			}

			// Lua is one-based, so the first result must be at index 1.
			++result_index;
			lua_rawseti( L, -2, (int) result_index );
		}
	}

	class AnyHitAlongRay : public b2RayCastCallback
	{
	public:

		AnyHitAlongRay( lua_State *L )
		: b2RayCastCallback()
		, fL( L )
		{
		}

		virtual ~AnyHitAlongRay()
		{
		}

		// fixture the fixture hit by the ray
		// point the point of initial intersection
		// normal the normal vector at the point of intersection
		float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			// Skip over objects that have been marked for deletion but have not yet been deleted from Box2D.
			if ( ! (DisplayObject *)fixture->GetBody()->GetUserData() )
				return -1;
			
			size_t result_index = 0;
			add_hit_to_table_of_raycasting_results( fL,
													LuaContext::GetRuntime( fL )->GetPhysicsWorld().GetPixelsPerMeter(),
													result_index,
													(DisplayObject *)fixture->GetBody()->GetUserData(),
													point,
													normal,
													fraction );

			// We only need one result, regardless of its "fraction",
			// therefore return 0.0f to immediately stop RayCast().
			return 0.0f;
		}

		lua_State *fL;
	};

	class ClosestHitAlongRay : public b2RayCastCallback
	{
	public:

		ClosestHitAlongRay( lua_State *L )
		: b2RayCastCallback()
		, fL( L )
		, fInitialTopIndexOfLuaStack( lua_gettop( fL ) )
		, fPixelsPerMeter( LuaContext::GetRuntime( fL )->GetPhysicsWorld().GetPixelsPerMeter() )
		{
		}

		virtual ~ClosestHitAlongRay()
		{
		}

		// fixture the fixture hit by the ray
		// point the point of initial intersection
		// normal the normal vector at the point of intersection
		float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			// Skip over objects that have been marked for deletion but have not yet been deleted from Box2D.
			if ( ! (DisplayObject *)fixture->GetBody()->GetUserData() )
				return -1;
			
			// We only want to return a single hit in the result table.
			// We reset the top of the Lua stack to always write the result
			// in the same position on the stack.
			lua_settop( fL, fInitialTopIndexOfLuaStack );

			size_t result_index = 0;
			add_hit_to_table_of_raycasting_results( fL,
													fPixelsPerMeter,
													result_index,
													(DisplayObject *)fixture->GetBody()->GetUserData(),
													point,
													normal,
													fraction );

			// "fraction" represents the fraction along the ray where the
			// hit is located. By returning "fraction", we clip the next
			// queries to this position. This has the effect of moving the
			// end point closer and closer to the starting point. This also
			// has the effect of giving us the closest possible hit point
			// from the starting point.
			return fraction;
		}

		lua_State *fL;
		int fInitialTopIndexOfLuaStack;
		float fPixelsPerMeter;
	};

	class UnsortedHitsAlongRay : public b2RayCastCallback
	{
	public:

		UnsortedHitsAlongRay( lua_State *L )
		: b2RayCastCallback()
		, fL( L )
		, fResultIndex( 0 )
		, fPixelsPerMeter( LuaContext::GetRuntime( fL )->GetPhysicsWorld().GetPixelsPerMeter() )
		{
		}

		virtual ~UnsortedHitsAlongRay()
		{
		}

		// fixture the fixture hit by the ray
		// point the point of initial intersection
		// normal the normal vector at the point of intersection
		float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			// Skip over objects that have been marked for deletion but have not yet been deleted from Box2D.
			if ( ! (DisplayObject *)fixture->GetBody()->GetUserData() )
				return -1;
			
			add_hit_to_table_of_raycasting_results( fL,
													fPixelsPerMeter,
													fResultIndex,
													(DisplayObject *)fixture->GetBody()->GetUserData(),
													point,
													normal,
													fraction );

			// Continue RayCast() until all possible results are found.
			return 1.0f;
		}

		lua_State *fL;
		size_t fResultIndex;
		float fPixelsPerMeter;
	};

	struct hit
	{
		// This is used to hold a "raw" (unscaled) result as reported by
		// ReportFixture(). We sort them by their "fraction" in PushResultsToLua().
		// We scale them by the pixels_per_meter ratio in add_hit_to_table_of_raycasting_results().

		hit( DisplayObject *display_object_,
				b2Vec2 point_,
				b2Vec2 normal_,
				float32 fraction_ )
		: display_object( display_object_ )
		, point( point_ )
		, normal( normal_ )
		, fraction( fraction_ )
		{
		}

		DisplayObject *display_object;
		b2Vec2 point;
		b2Vec2 normal;
		float32 fraction;

		bool operator < ( const hit &other ) const
		{
			return ( fraction < other.fraction );
		}
	};

	typedef std::list< hit > ListHit;
	typedef std::list< hit >::iterator ListHitIter;

	class SortedHitsAlongRay : public b2RayCastCallback
	{
	public:

		SortedHitsAlongRay( lua_State *L )
		: b2RayCastCallback()
		, fL( L )
		, fListHit()
		{
		}

		virtual ~SortedHitsAlongRay()
		{
		}

		// fixture the fixture hit by the ray
		// point the point of initial intersection
		// normal the normal vector at the point of intersection
		float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			// Skip over objects that have been marked for deletion but have not yet been deleted from Box2D.
			if ( ! (DisplayObject *)fixture->GetBody()->GetUserData() )
				return -1;
			
			// We don't want to push the result to Lua immediately because
			// we want to sort them by distance from the starting point.
			fListHit.push_back( hit( (DisplayObject *)fixture->GetBody()->GetUserData(),
										point,
										normal,
										fraction ) );

			// Continue RayCast() until all possible results are found.
			return 1.0f;
		}

		bool PushResultsToLua()
		{
			if( fListHit.empty() )
			{
				// There's no result.
				// Nothing to do.
				return false;
			}

			// Sort the results from closest to farthest.
			fListHit.sort();

			float pixels_per_meter = LuaContext::GetRuntime( fL )->GetPhysicsWorld().GetPixelsPerMeter();

			size_t result_index = 0;

			for( ListHitIter iter = fListHit.begin();
					iter != fListHit.end();
					++iter )
			{
				add_hit_to_table_of_raycasting_results( fL,
														pixels_per_meter,
														result_index,
														iter->display_object,
														iter->point,
														iter->normal,
														iter->fraction );
			}

			return true;
		}

		lua_State *fL;
		ListHit fListHit;
	};
} // anonymous namespace.

static int
common_ray_cast( lua_State *L,
					b2RayCastCallback *callback )
{
	const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
	b2World *world = physics.GetWorld();

	b2Vec2 from_in_meters( lua_tonumber( L, 1 ), lua_tonumber( L, 2 ) );
	b2Vec2 to_in_meters( lua_tonumber( L, 3 ), lua_tonumber( L, 4 ) );

	// Pixels to meters.
	float meters_per_pixels = ( 1.0f / physics.GetPixelsPerMeter() );
	from_in_meters *= meters_per_pixels;
	to_in_meters *= meters_per_pixels;

	// Important: If any results are found, "callback" will leave
	// a table at the top of the Lua stack. This table at the top
	// of the Lua stack is the result we return from this function.
	//
	// Exception: For SortedHitsAlongRay, the results are accumulated
	// so they can be sorted before they're pushed to the Lua stack.
	int top_index_before_RayCast = lua_gettop( L );
	world->RayCast( callback, from_in_meters, to_in_meters );

	// Any hits returned by RayCast() are pushed into a table that's
	// on the stack. We want to return true if we're returning a result.
	// Therefore we can compare the top index of the Lua stack before
	// and after RayCast() to know if we're returning a table of hits.
	return ( top_index_before_RayCast != lua_gettop( L ) );
}

static int
RayCast( lua_State *L )
{
	const char *behavior = lua_tostring( L, 5 );
    
    if (LuaLibPhysics::IsWorldValid(L, "physics.RayCast()"))
    {
        if( ! Rtt_StringCompare( "any", behavior ) )
        {
            AnyHitAlongRay callback( L );

            return common_ray_cast( L,
                                    &callback );
        }
        else if( ! Rtt_StringCompare( "unsorted", behavior ) )
        {
            UnsortedHitsAlongRay callback( L );

            return common_ray_cast( L,
                                    &callback );
        }
        else if( ! Rtt_StringCompare( "sorted", behavior ) )
        {
            SortedHitsAlongRay callback( L );

            common_ray_cast( L,
                                &callback );

            // This leaves a table at the top of the Lua stack. This table is
            // the result we return from this function.
            return callback.PushResultsToLua();
        }
        else // if( ! Rtt_StringCompare( "closest", behavior ) )
        {
            // This MUST be the "else" case because it's the default option.

            ClosestHitAlongRay callback( L );

            return common_ray_cast( L,
                                    &callback );
        }
    }
    else
    {
        return 0;
    }
}

static int
ReflectRay( lua_State *L )
{
	/*
	The third expected parameter is a "hit" table entry as returned by the ray casting functions:

		local reflected_ray_direction_x, reflected_ray_direction_y = physics.reflectRay( ray_from_x, ray_from_y, hits[1] )

			hit.object
			hit.position.x
			hit.position.y
			hit.normal.x
			hit.normal.y
	*/

    if (! lua_isnumber(L, 1) || ! lua_isnumber(L, 2) || ! lua_istable(L, 3))
    {
        CoronaLuaError(L, "physics.reflectRay() requires 3 parameters (number, number, table)");
        
        return 0;
    }

	b2Vec2 A_position( lua_tonumber( L, 1 ), lua_tonumber( L, 2 ) );

	b2Vec2 B_position;
	{
		// 3: A "hit" table entry as returned by the ray casting functions.
		lua_getfield( L, 3, "position" );
		{
			lua_getfield( L, -1, "x" );
			B_position.x = lua_tonumber( L, -1 );
			lua_pop( L, 1 );

			lua_getfield( L, -1, "y" );
			B_position.y = lua_tonumber( L, -1 );
			lua_pop( L, 1 );
		}
		lua_pop( L, 1 );
	}

	b2Vec2 normal;
	{
		// 3: A "hit" table entry as returned by the ray casting functions.
		lua_getfield( L, 3, "normal" );
		{
			lua_getfield( L, -1, "x" );
			normal.x = lua_tonumber( L, -1 );
			lua_pop( L, 1 );

			lua_getfield( L, -1, "y" );
			normal.y = lua_tonumber( L, -1 );
			lua_pop( L, 1 );
		}
		lua_pop( L, 1 );
	}

	b2Vec2 reflection;
	{
		// This is a variant of the standard formula:
		// I - 2.0 * dot(N, I) * N;

		b2Vec2 incoming( B_position - A_position );

		b2Vec2 incoming_along_normal( normal );
		incoming_along_normal *= b2Dot( incoming, normal );

		b2Vec2 incoming_along_plane( incoming - incoming_along_normal );

		reflection = incoming_along_plane;
		reflection -= incoming_along_normal;

		reflection.Normalize();
	}

	// Return the result.
	lua_pushnumber( L, Rtt_FloatToReal( reflection.x ) );
	lua_pushnumber( L, Rtt_FloatToReal( reflection.y ) );

	// 2: X and Y of the normalized reflected input vector.
	return 2;
}

namespace // anonymous namespace.
{
	class HitsInRegion : public b2QueryCallback
	{
	public:

		HitsInRegion( lua_State *L )
		: b2QueryCallback()
		, fL( L )
		, fInitialTopIndexOfLuaStack( lua_gettop( fL ) )
		, fPixelsPerMeter( LuaContext::GetRuntime( fL )->GetPhysicsWorld().GetPixelsPerMeter() )
		, fResultCount( 0 )
		{
		}

		virtual ~HitsInRegion()
		{
		}

		// fixture overlapping the region.
		bool ReportFixture( b2Fixture* fixture )
		{
			// Skip over objects that have been marked for deletion but have not yet been deleted from Box2D.
			if ( ! (DisplayObject *)fixture->GetBody()->GetUserData() )
				return true;
			
			if( ! fResultCount )
			{
				// We're handling the first result.
				lua_newtable( fL );
			}

			// Add the result.
			{
				// Hit object.
				DisplayObject *optional_display_object = (DisplayObject *)fixture->GetBody()->GetUserData();

				if ( optional_display_object )
				{
					optional_display_object->GetProxy()->PushTable( fL );
				}
				else
				{
					lua_pushnil( fL );
				}

				// Lua is one-based, so the first result must be at index 1.
				++fResultCount;
				lua_rawseti( fL, -2, fResultCount );
			}

			// true: Continue the query.
			return true;
		}

		lua_State *fL;
		int fInitialTopIndexOfLuaStack;
		float fPixelsPerMeter;
		int fResultCount;
	};
} // anonymous namespace.

static int
QueryRegion( lua_State *L )
{
    if (! lua_isnumber(L, 1) || ! lua_isnumber(L, 2) || ! lua_isnumber(L, 3) || ! lua_isnumber(L, 4))
    {
        CoronaLuaError(L, "physics.queryRegion() requires 4 parameters (number, number, number, number)");
        
        return 0;
    }
    
    if (LuaLibPhysics::IsWorldValid(L, "physics.queryRegion()"))
    {
        const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
        b2World *world = physics.GetWorld();

        b2AABB aabb;
        aabb.lowerBound.Set( lua_tonumber( L, 1 ), lua_tonumber( L, 2 ) );
        aabb.upperBound.Set( lua_tonumber( L, 3 ), lua_tonumber( L, 4 ) );

        // Pixels to meters.
        float meters_per_pixels = ( 1.0f / physics.GetPixelsPerMeter() );
        aabb.lowerBound *= meters_per_pixels;
        aabb.upperBound *= meters_per_pixels;

        HitsInRegion callback( L );

        // Important: If any results are found, "callback" will leave
        // a table at the top of the Lua stack. This table at the top
        // of the Lua stack is the result we return from this function.
        int top_index_before_QueryAABB = lua_gettop( L );
        world->QueryAABB( &callback, aabb );

        // Any hits returned by QueryAABB() are pushed into a table that's
        // on the stack. We want to return true if we're returning a result.
        // Therefore we can compare the top index of the Lua stack before
        // and after QueryAABB() to know if we're returning a table of hits.
        return ( top_index_before_QueryAABB != lua_gettop( L ) );
    }
    else
    {
        return 0;
    }
}

static int
SetAverageCollisionPositions( lua_State *L )
{
    if (! lua_isboolean(L, 1))
    {
        CoronaLuaError(L, "physics.setAverageCollisionPositions() requires 1 parameter (boolean)");
        
        return 0;
    }
    
	PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

	physics.SetAverageCollisionPositions( lua_toboolean( L, 1 ) );

	return 0;
}

static int
GetAverageCollisionPositions( lua_State *L )
{
	const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();

	lua_pushboolean( L, physics.GetAverageCollisionPositions() );

	return 1;
}

static int
setScale( lua_State *L )
{
    if (! lua_isnumber(L, 1))
    {
        CoronaLuaError(L, "physics.setScale() requires 1 parameter (number)");
        
        return 0;
    }

	PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
	physics.SetPixelsPerMeter( luaL_toreal( L, 1 ) );

	return 0;
}

// Creates a b2Body with no fixtures
static b2Body*
CreateBody( const PhysicsWorld& physics, DisplayObject *o )
{
	b2Body *result = NULL;

	b2World *world = physics.GetWorld();
	if ( world )
	{
		b2BodyDef bd;
		bd.type = b2_dynamicBody; // default (settable with "bodyType" attribute)
		bd.userData = o;

		Real scale = physics.GetPixelsPerMeter();

		Real x = o->GetGeometricProperty( kOriginX );
		Real y = o->GetGeometricProperty( kOriginY );
		Real rotation = o->GetGeometricProperty( kRotation );

		x = Rtt_RealDiv( x, scale );
		y = Rtt_RealDiv( y, scale);
		b2Vec2 pos( x, y );

		Real angle = Rtt_RealDegreesToRadians( rotation );

		result = world->CreateBody( & bd ); Rtt_ASSERT( result );
		result->SetTransform( pos, angle );
	}

	return result;
}

static const char kStaticBodyType[] = "static";
static const char kKinematicBodyType[] = "kinematic";
// NOT USED: static const char kDynamicBodyType[] = "dynamic";

static void
InitializeFixturePhysicsDefaults( b2FixtureDef &outFixtureDef,
									const b2Shape *optional_shapeDef )
{
	// "optional_shapeDef" is optional because, in the context
	// of "InitializeFixtureUsing_Outline()", it's set in
	// "b2Separator::SeparateAndCreateFixtures()".
	outFixtureDef.shape = optional_shapeDef;

	// Set sensible defaults
	// IMPORTANT: These defaults are overridden by InitializeFixtureFromLua().
	outFixtureDef.density = 0.01f; // previous default was zero, but that did odd things in Box2D dynamic bodies (contrary to documentation?)
	outFixtureDef.friction = 0.3f;
	outFixtureDef.restitution = 0.5f;
	outFixtureDef.isSensor = false;
}

static void
InitializeFixtureFromLua( lua_State *L,
							b2FixtureDef &outFixtureDef,
							const b2Shape *optional_shapeDef,
							int lua_arg_index )
{
	InitializeFixturePhysicsDefaults( outFixtureDef, optional_shapeDef );

	if( ! lua_istable( L, lua_arg_index ) )
	{
		// Nothing to do.
		return;
	}

	lua_getfield( L, lua_arg_index, "density" );
	float density = (float) lua_tonumber( L, -1 );
	if ( density > 0.0f )
	{
		outFixtureDef.density = density;
	}
	lua_pop( L, 1 );

	// If not supplied, we assume a default of 0
	lua_getfield( L, lua_arg_index, "friction" );
	float friction = (float) lua_tonumber( L, -1 );
	if (friction >= 0.0f )
	{
		outFixtureDef.friction = friction;
	}
	lua_pop( L, 1 );

	// If not supplied, we assume a default of 0
	lua_getfield( L, lua_arg_index, "bounce" ); // renamed from "restitution" for developer clarity
	float restitution = (float) lua_tonumber( L, -1 );
	if (restitution >= 0.0f )
	{
		outFixtureDef.restitution = restitution;
	}
	lua_pop( L, 1 );

	// If not supplied, we assume a default of false
	lua_getfield( L, lua_arg_index, "isSensor" );
	outFixtureDef.isSensor = (bool)lua_toboolean( L, -1 );
	lua_pop( L, 1 );

	lua_getfield( L, lua_arg_index, "filter" );
	if ( lua_istable( L, -1 ) )
	{
		b2Filter& filter = outFixtureDef.filter;

		lua_getfield( L, -1, "categoryBits" );
		if ( ! lua_isnil( L, -1 ) )
		{
			uint16 categoryBits = (uint16)lua_tonumber( L, -1 );
			filter.categoryBits = categoryBits;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "maskBits" );
		if ( ! lua_isnil( L, -1 ) )
		{
			uint16 maskBits = (uint16)lua_tonumber( L, -1 );
			filter.maskBits = maskBits;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "groupIndex" );
		if ( ! lua_isnil( L, -1 ) )
		{
			int16 groupIndex = (int16)lua_tonumber( L, -1 );
			filter.groupIndex = groupIndex;
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );
}

static const char kDistanceJointType[] = "distance";
static const char kPivotJointType[] = "pivot";
static const char kPistonJointType[] = "piston";
static const char kFrictionJointType[] = "friction";
static const char kWeldJointType[] = "weld"; // note: has no type-specific methods
static const char kWheelJointType[] = "wheel"; // combines a piston and a pivot joint, like a wheel on a shock absorber
static const char kPulleyJointType[] = "pulley";
static const char kTouchJointType[] = "touch";
static const char kGearJointType[] = "gear";
static const char kRopeJointType[] = "rope";

/*
static b2JointType
JointTypeForString( const char *value )
{
	b2JointType result = e_unknownJoint;

	if ( 0 == strcmp( value, kDistanceJointType ) )
	{
		result = e_distanceJoint;
	}
	else if ( 0 == strcmp( value, kPivotJointType ) )
	{
		result = e_revoluteJoint;
	}
	else if ( 0 == strcmp( value, kPistonJointType ) )
	{
		result = e_prismaticJoint;
	}
	else if ( 0 == strcmp( value, kFrictionJointType ) )
	{
		result = e_frictionJoint;
	}
	else if ( 0 == strcmp( value, kWeldJointType ) )
	{
		result = e_weldJoint;
	}
	else if ( 0 == strcmp( value, kWheelJointType ) )
	{
		result = e_wheelJoint;
	}
	else if ( 0 == strcmp( value, kPulleyJointType ) )
	{
		result = e_pulleyJoint;
	}
	else if ( 0 == strcmp( value, kTouchJointType ) )
	{
		result = e_mouseJoint;
	}
	else if ( 0 == strcmp( value, kGearJointType ) )
	{
		result = e_gearJoint;
	}
	else if ( 0 == strcmp( value, kRopeJointType ) )
	{
		result = e_ropeJoint;
	}

	return result;
}
*/

static int
CreateAndPushJoint(
	const ResourceHandle< lua_State >& luaStateHandle,
	const PhysicsWorld& physics,
	const b2JointDef& jointDef )
{
	int result = 0;
	b2World *world = physics.GetWorld();
	if ( world )
	{
		// Following is ownership graph:

		// (1) b2World owns joint,
		b2Joint *joint = world->CreateJoint( & jointDef );

		// (2) Lua owns wrapper (which has a weak pointer to joint),
		UserdataWrapper *wrapper = Rtt_NEW(
			runtime.Allocator(),
			UserdataWrapper( luaStateHandle, joint, PhysicsJoint::kMetatableName ) );

		// and (3) joint has a weak back reference to wrapper.
		joint->SetUserData( wrapper );

		result = wrapper->Push();
	}

	return result;
}

static Real
luaL_torealphysics( lua_State *L, int index, Real scale )
{
	Real result = luaL_toreal( L, index );
	result = Rtt_RealDiv( result, scale );
	return result;
}

static int
newJoint( lua_State *L )
{
	int result = 0;

	if ( ! LuaLibPhysics::IsWorldLocked( L, "physics.newJoint()" ) )
	{
		const char *jointType = luaL_checkstring( L, 1 );

		PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		Real scale = physics.GetPixelsPerMeter();

		Runtime& runtime = * LuaContext::GetRuntime( L );
		const ResourceHandle< lua_State >& luaStateHandle = runtime.VMContext().LuaState();
		// There are lots of joint types, each with different constructor formats
		// Hopefully some of the repetitive code below can be rolled up later

		// [1] Verify display objects
		DisplayObject* o1 = (DisplayObject*)LuaProxy::GetProxyableObject( L, 2 );
		DisplayObjectExtensions *e1 = o1->GetExtensions();
		DisplayObject* o2 = NULL;
		DisplayObjectExtensions *e2 = NULL;

		Rtt_WARN_SIM_PROXY_TYPE( L, 2, DisplayObject );
		int badArgIndex = 0;
		if ( ! e1 ) { badArgIndex = 2; }

		// Non-touch joints take a second display object
		if ( strcmp( kTouchJointType, jointType ) != 0 )
		{
			o2 = (DisplayObject*)LuaProxy::GetProxyableObject( L, 3 );
			e2 = o2->GetExtensions();

			Rtt_WARN_SIM_PROXY_TYPE( L, 3, DisplayObject );
			if ( ! e2 )
            {
                badArgIndex = 3;
            }
            else
            {
                // All joint types other than touch take two bodies which must not be
                // the same object (Box2D asserts)
                b2Body *body1 = e1->GetBody();
                b2Body *body2 = e2->GetBody();
                
                if (body1 == body2)
                {
                    CoronaLuaError(L, "physics.newJoint() object1 and object2 cannot be the same object");
                    
                    return 0;
                }
            }
        }

		// [2] Create joints
		if ( badArgIndex > 0 ) // On error, badArgIndex indicates the Lua index of the bad argument
		{
			CoronaLuaError(L, "physics.newJoint() requires argument %d to have a physics body attached to the display object", badArgIndex );
		}
		else if ( strcmp( kDistanceJointType, jointType ) == 0 )
		{
			b2Body *body1 = e1->GetBody();
			b2Body *body2 = e2->GetBody();

			Real px = luaL_torealphysics( L, 4, scale );
			Real py = luaL_torealphysics( L, 5, scale );

			Real qx = luaL_torealphysics( L, 6, scale );
			Real qy = luaL_torealphysics( L, 7, scale );

			b2DistanceJointDef jointDef;

			b2Vec2 point1( px, py );
			b2Vec2 point2( qx, qy );

			jointDef.Initialize( body1, body2, point1, point2 );

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}

		else if ( strcmp( kPivotJointType, jointType ) == 0 )
		{
			b2Body *body1 = e1->GetBody();
			b2Body *body2 = e2->GetBody();

			Real px = luaL_torealphysics( L, 4, scale );
			Real py = luaL_torealphysics( L, 5, scale );

			b2RevoluteJointDef jointDef;

			b2Vec2 point1( px, py );

			jointDef.Initialize( body1, body2, point1 );

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}

		else if ( strcmp( kPistonJointType, jointType ) == 0 )
		{
			b2Body *body1 = e1->GetBody();
			b2Body *body2 = e2->GetBody();

			Real px = luaL_torealphysics( L, 4, scale );
			Real py = luaL_torealphysics( L, 5, scale );

			// Don't scale the axis vector
			Real axisX = luaL_toreal( L, 6 );
			Real axisY = luaL_toreal( L, 7 );

			b2PrismaticJointDef jointDef;

			b2Vec2 anchor( px, py );
			b2Vec2 axis( axisX, axisY );
			axis.Normalize();

			jointDef.Initialize( body1, body2, anchor, axis );

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}

		else if ( strcmp( kFrictionJointType, jointType ) == 0 )
		{
			b2Body *body1 = e1->GetBody();
			b2Body *body2 = e2->GetBody();

			Real px = luaL_torealphysics( L, 4, scale );
			Real py = luaL_torealphysics( L, 5, scale );

			b2FrictionJointDef jointDef;

			b2Vec2 point1( px, py );

			jointDef.Initialize( body1, body2, point1 );

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}

		else if ( strcmp( kWeldJointType, jointType ) == 0 )
		{
			b2Body *body1 = e1->GetBody();
			b2Body *body2 = e2->GetBody();

			Real px = luaL_torealphysics( L, 4, scale );
			Real py = luaL_torealphysics( L, 5, scale );

			b2WeldJointDef jointDef;

			b2Vec2 point1( px, py );

			jointDef.Initialize( body1, body2, point1 );

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}

		else if ( strcmp( kWheelJointType, jointType ) == 0 )
		{
			b2Body *body1 = e1->GetBody();
			b2Body *body2 = e2->GetBody();

			Real px = luaL_torealphysics( L, 4, scale );
			Real py = luaL_torealphysics( L, 5, scale );

			Real qx = luaL_torealphysics( L, 6, scale );
			Real qy = luaL_torealphysics( L, 7, scale );

			// TODO
			b2WheelJointDef jointDef;

			b2Vec2 point( px, py );
			b2Vec2 axis( qx, qy );

			jointDef.Initialize( body1, body2, point, axis );

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}

		else if ( strcmp( kPulleyJointType, jointType ) == 0 )
		{
			b2Body *body1 = e1->GetBody();
			b2Body *body2 = e2->GetBody();

			Real px = luaL_torealphysics( L, 4, scale );
			Real py = luaL_torealphysics( L, 5, scale );

			Real qx = luaL_torealphysics( L, 6, scale );
			Real qy = luaL_torealphysics( L, 7, scale );

			Real rx = luaL_torealphysics( L, 8, scale );
			Real ry = luaL_torealphysics( L, 9, scale );

			Real sx = luaL_torealphysics( L, 10, scale );
			Real sy = luaL_torealphysics( L, 11, scale );

			Rtt_Real ratio = 1.0;

			if ( lua_isnumber( L, 12 ) )
			{
				ratio = luaL_toreal( L, 12 );
			}

			b2PulleyJointDef jointDef;

			b2Vec2 fixedAnchor1 = b2Vec2( px, py );
			b2Vec2 fixedAnchor2 = b2Vec2( qx, qy );
			b2Vec2 bodyAnchor1 = b2Vec2( rx, ry );
			b2Vec2 bodyAnchor2 = b2Vec2( sx, sy );

			jointDef.Initialize( body1, body2, fixedAnchor1, fixedAnchor2, bodyAnchor1, bodyAnchor2, ratio );

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}

		else if ( strcmp( kTouchJointType, jointType ) == 0 )
		{
			float px = luaL_torealphysics( L, 3, scale );
			float py = luaL_torealphysics( L, 4, scale );

			b2MouseJointDef jointDef;

			b2Vec2 targetPoint( px, py );

			// Unlike the other b2JointDef subclasses, b2MouseJointDef has no Initialize() method.
			// Instead, we set the struct attributes directly (the relevant reference code is the Test class in the Box2D testbed).

			b2Body *body = e1->GetBody();

			jointDef.bodyA = physics.GetGroundBody();
			jointDef.bodyB = body;
			jointDef.target = targetPoint;
			jointDef.maxForce = 1000.f * body->GetMass();
			body->SetAwake( true );

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}

		else if ( strcmp( kGearJointType, jointType ) == 0 )
		{
			b2Body *body1 = e1->GetBody();
			b2Body *body2 = e2->GetBody();

			b2GearJointDef jointDef;

			jointDef.bodyA = body1;
			jointDef.bodyB = body2;

			jointDef.joint1 = PhysicsJoint::GetJoint( L, 4 );
			jointDef.joint2 = PhysicsJoint::GetJoint( L, 5 );

			jointDef.ratio = luaL_toreal( L, 6 );

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}

		else if ( strcmp( kRopeJointType, jointType ) == 0 )
		{
			b2Body *body1 = e1->GetBody();
			b2Body *body2 = e2->GetBody();

			Real ax = luaL_torealphysics( L, 4, scale );
			Real ay = luaL_torealphysics( L, 5, scale );

			Real bx = luaL_torealphysics( L, 6, scale );
			Real by = luaL_torealphysics( L, 7, scale );

			b2RopeJointDef jointDef;

			jointDef.bodyA = body1;
			jointDef.bodyB = body2;

			jointDef.localAnchorA = b2Vec2( ax, ay );
			jointDef.localAnchorB = b2Vec2( bx, by );

			jointDef.maxLength = (body1->GetPosition() - body2->GetPosition()).Length();

			result = CreateAndPushJoint( luaStateHandle, physics, jointDef );
		}
        else
        {
            CoronaLuaError(L, "physics.newJoint() unknown joint type '%s'", jointType);
        }
	}

	return result;
}

// physics.newParticleSystem( [params_table] )
static int
newParticleSystem( lua_State *L )
{
	if( LuaLibPhysics::IsWorldLocked( L, "physics.newParticleSystem()" ) )
	{
		return 0;
	}

	Runtime& runtime = * LuaContext::GetRuntime( L );
	Display& display = runtime.GetDisplay();

	if( display.ShouldRestrict( Display::kPhysicsNewParticleSystem ) )
	{
		return 0;
	}

	PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
	b2World *world = physics.GetWorld();
	if( ! world )
	{
		return 0;
	}

	ParticleSystemObject *pso = Rtt_NEW( runtime.Allocator(),
											ParticleSystemObject( /* C'TOR PARAMS HERE, IF ANY!!! */ ) );
	if( pso->Initialize( L, display ) )
	{
		return LuaLibDisplay::AssignParentAndPushResult( L, display, pso, NULL );
	}
	else
	{
		Rtt_DELETE( pso );

		luaL_error( L,
					"Invalid ParticleSystemObject." );

        return 0;
	}

	return 0;
}

// addBody() helpers.

static void _FixtureCreator( b2Body *body,
								b2FixtureDef *fixtureDef,
								int &fixtureIndex )
{
	b2Fixture *fixture = body->CreateFixture( fixtureDef );

	// Store fixture index in the fixture's userData
	// This is a low-overhead way to reidentify specific fixtures
	// by index during Lua collision events; the fixture indexes
	// for each body are returned as integer values in "event.element1"
	// and "event.element2" in the collision event. Also note that
	// Corona uses the term "body element" to mean Box2D's "fixture",
	// when speaking of multi-element bodies.
	//
	// See user documentation here:
	// http://developer.coronalabs.com/content/game-edition-collision-detection#Collisions_with_multi-element_bodies

	// an ordered index for all fixtures, ranging from 1 to n
	fixture->SetUserData( (void *)(intptr_t)fixtureIndex++ );
}

static bool
InitializeFixtureUsing_Rectangle( lua_State *L,
									int lua_arg_index,
									int &fixtureIndex,
									b2Vec2 &center_in_pixels,
									DisplayObject *display_object,
									b2Body *body,
									float meter_per_pixels_scale )
{
	DEBUG_PRINT( "%s\n", __FUNCTION__ );

	// This MUST be a rectangular shape.

	Real halfW = ( display_object->GetGeometricProperty( kWidth ) * 0.5f );
	Real halfH = ( display_object->GetGeometricProperty( kHeight ) * 0.5f );

	// Convert to meters.
	halfW *= meter_per_pixels_scale;
	halfH *= meter_per_pixels_scale;

	b2FixtureDef fixtureDef;

	b2PolygonShape polygonDef;

	polygonDef.SetAsBox( halfW,
							halfH,
							( center_in_pixels * meter_per_pixels_scale ),
							0.0f );

	InitializeFixtureFromLua( L,
								fixtureDef,
								&polygonDef,
								lua_arg_index );

	_FixtureCreator( body,
						&fixtureDef,
						fixtureIndex );

	return true;
}

static void _ArrayVertex2_to_b2Vec2Vector( ArrayVertex2 &v_array_in,
											b2Vec2Vector &v_array_out,
											b2Vec2 &center_in_pixels,
											float meter_per_pixels_scale )
{
	unsigned int numVertices = v_array_in.Length();

	v_array_out.resize( numVertices );

	for( unsigned int i = 0;
			i < numVertices;
			++i )
	{
		Vertex2 &v_in = v_array_in[ i ];
		b2Vec2 &v_out = v_array_out[ i ];

		v_out.Set( v_in.x, v_in.y );

		v_out += center_in_pixels;

		// Convert to meters.
		v_out *= meter_per_pixels_scale;
	}
}

static void _FromLua_to_b2Vec2Vector( lua_State *L,
										b2Vec2Vector &vertexList )
{
	int table_index = lua_gettop( L );

	int numVertices = (int)( lua_objlen( L, table_index ) / 2 );
	vertexList.resize( numVertices );

	for( int i = 0;
			i < numVertices;
			++i )
	{
		// Get all the points from Lua.

		int n = ( ( i * 2 ) + 1 );

		b2Vec2 &v = vertexList[ i ];

		// Lua is one-based, so the first element must be at index 1.
		lua_rawgeti( L, table_index, n );

		// Lua is one-based, so the second element must be at index 2.
		lua_rawgeti( L, table_index, ( n + 1 ) );

		v.Set( luaL_toreal( L, -2 ),
				luaL_toreal( L, -1 ) );

		lua_pop( L, 2 );

		DEBUG_PRINT( "Outline from Lua: %f, %f\n",
						v.x,
						v.y );
	}
}

static void _Transform_b2Vec2Vector( b2Vec2Vector &vertexList,
										const b2Vec2 &center_in_pixels,
										float meter_per_pixels_scale )
{
	int numVertices = (int)vertexList.size();

	for( int i = 0;
			i < numVertices;
			++i )
	{
		b2Vec2 &v = vertexList[ i ];

		// Translate.
		v += center_in_pixels;

		// Convert to meters.
		v *= meter_per_pixels_scale;

		DEBUG_PRINT( "Outline from Lua: %f, %f\n",
						v.x,
						v.y );
	}
}

static bool
InitializeFixtureUsing_StaticLine( lua_State *L,
									int lua_arg_index,
									int &fixtureIndex,
									b2Vec2 &center_in_pixels,
									DisplayObject *display_object,
									b2Body *body,
									float meter_per_pixels_scale )
{
	// This might be a static line.

	if( &LuaLineObjectProxyVTable::Constant() == &display_object->GetProxy()->Delegate() )
	{
		DEBUG_PRINT( "%s\n", __FUNCTION__ );

		// It's a LineObject.

		// We only support static lines.
		body->SetType( b2_staticBody );

		LineObject *line_object = static_cast< LineObject * >( display_object );
		OpenPath &path = static_cast< OpenPath& >( line_object->GetPath() );

		// Get a chain geometry from a polygon's tesselator.
		//
		// This creates a body that's massless and doesn't
		// behave as expected when applied to a dynamic object
		// (b2_kinematicBody). That's why we only allow static
		// objects (b2_staticBody) to be created this way.

		b2Vec2Vector vertexList;
		_ArrayVertex2_to_b2Vec2Vector( path.GetVertices(),
										vertexList,
										center_in_pixels,
										meter_per_pixels_scale );
		if( vertexList.size() < 2 )
		{
			CoronaLuaError( L, "physics.addBody() with a LineObject requires at least 2 vertices." );

			// true: No one else should handle this because
			// this is ONLY meant to be a LineObject.
			return true;
		}

		b2FixtureDef fixtureDef;

		b2ChainShape chainDef;
		chainDef.CreateChain( &vertexList[ 0 ],
								(int)vertexList.size() );

		InitializeFixtureFromLua( L,
									fixtureDef,
									&chainDef,
									lua_arg_index );

		_FixtureCreator( body,
							&fixtureDef,
							fixtureIndex );

		return true;
	}

	return false;
}

static bool
InitializeFixtureUsing_ArbitraryPolygonalShape( lua_State *L,
												int lua_arg_index,
												int &fixtureIndex,
												b2Vec2 &center_in_pixels,
												DisplayObject *display_object,
												b2Body *body,
												float meter_per_pixels_scale )
{
	// This might be an arbitrary polygonal shape.

	if( ( body->GetType() == b2_staticBody ) &&
		( &LuaShapeObjectProxyVTable::Constant() == &display_object->GetProxy()->Delegate() ) )
	{
		DEBUG_PRINT( "%s\n", __FUNCTION__ );

		// This is a static ShapeObject.

		ShapeObject *shape_object = static_cast< ShapeObject * >( display_object );
		ShapePath &path = static_cast< ShapePath& >( shape_object->GetPath() );

		if( path.GetTesselator()->GetType() == Tesselator::kType_Polygon )
		{
			// Get a chain geometry from a polygon's tesselator.
			//
			// This creates a body that's massless and doesn't
			// behave as expected when applied to a dynamic object
			// (b2_kinematicBody). That's why we only allow static
			// objects (b2_staticBody) to be created this way.

			TesselatorPolygon *tesselator_polygon = (TesselatorPolygon *)path.GetTesselator();

			b2Vec2Vector vertexList;
			_ArrayVertex2_to_b2Vec2Vector( tesselator_polygon->GetContour(),
											vertexList,
											center_in_pixels,
											meter_per_pixels_scale );

			b2FixtureDef fixtureDef;

			b2ChainShape chainDef;
			chainDef.CreateLoop( &vertexList[ 0 ],
									(int)vertexList.size() );

			InitializeFixtureFromLua( L,
										fixtureDef,
										&chainDef,
										lua_arg_index );

			_FixtureCreator( body,
								&fixtureDef,
								fixtureIndex );

			return true;
		}
	}

	return false;
}

static bool
InitializeFixtureUsing_Outline( lua_State *L,
								int lua_arg_index,
								int &fixtureIndex,
								b2Vec2 &center_in_pixels,
								DisplayObject *display_object,
								b2Body *body,
								float meter_per_pixels_scale )
{
	lua_getfield( L, lua_arg_index, "outline" );
	if( lua_istable( L, -1 ) )
	{
		DEBUG_PRINT( "%s\n", __FUNCTION__ );

		// This is an outline.

		// Load the outline.
		b2Vec2Vector vertexList;
		_FromLua_to_b2Vec2Vector( L,
									vertexList );

		// Tessellate the outline.
		b2Separator sep;

		// Validate.
		int error = sep.Validate( vertexList );
		if( error )
		{
			CoronaLuaError( L, "physics.addBody() : invalid \"outline\"" );

			DEBUG_PRINT( "b2Separator::Validate() : %d\nNote: 0 is ok. 1 is overlapping lines. 2 is counter-clockwise points. 3 is both 1 and 2.\n",
							error );

			lua_pop( L, 1 );
			// true: No one else should handle this because
			// this is ONLY meant to be an "outline".
			return true;
		}

		b2FixtureDef fixtureDef;

		InitializeFixtureFromLua( L,
									fixtureDef,
									NULL,
									lua_arg_index );

		// This translation is used to center the
		// outline around the DisplayObject.
		b2Vec2 translate;
		b2Vec2 scale;
		{
			float display_object_width = display_object->GetGeometricProperty( kWidth );
			float display_object_height = display_object->GetGeometricProperty( kHeight );

			// This is a ShapeObject.
			Rtt_ASSERT( &LuaShapeObjectProxyVTable::Constant() == &display_object->GetProxy()->Delegate() );
			ShapeObject *shape_object = static_cast< ShapeObject * >( display_object );
			ShapePath &path = static_cast< ShapePath& >( shape_object->GetPath() );
			Paint *fill = path.GetFill();
			Rtt_ASSERT( fill );

			int texture_width = 0;
			int texture_height = 0;
			{
				const ImageSheetPaint *image_sheet_paint = static_cast< const ImageSheetPaint * >( fill->AsPaint( Paint::kImageSheet ) );
				if( image_sheet_paint )
				{
					// It's an ImageSheet, so we only want the size of the
					// subregion of the texture that's actually rendered.
					const ImageFrame *image_frame = image_sheet_paint->GetImageFrame();

					texture_width = image_frame->GetPixelW();
					texture_height = image_frame->GetPixelH();
				}
				else if ( fill->AsPaint( Paint::kBitmap ) )
				{
					// It's NOT an ImageSheet, so we want the size of the
					// full texture.
					Texture *texture = fill->GetTexture();
					Rtt_ASSERT( texture );

					texture_width = texture->GetWidth();
					texture_height = texture->GetHeight();
				}
				else
				{
					texture_width = display_object_width;
					texture_height = display_object_height;
				}
			}

			translate.Set( texture_width * -0.5f, texture_height * -0.5f );

			translate += center_in_pixels;

			scale.Set(
				( display_object_width / texture_width ) * meter_per_pixels_scale,
				( display_object_height / texture_height ) * meter_per_pixels_scale );

			DEBUG_PRINT( "%s():\n"
							"meter_per_pixels_scale: %f\n"
							"display_object_width: %f\n"
							"display_object_height: %f\n"
							"texture_width: %d\n"
							"texture_height: %d\n"
							"translate.x: %f\n"
							"translate.y: %f\n"
							"scale.x: %f\n"
							"scale.y: %f\n",
							__FUNCTION__,
							meter_per_pixels_scale,
							display_object_width,
							display_object_height,
							texture_width,
							texture_height,
							translate.x,
							translate.y,
							scale.x,
							scale.y );
		}

		bool ok = sep.SeparateAndCreateFixtures( body,
													&fixtureDef,
													fixtureIndex,
													_FixtureCreator,
													vertexList,
													translate,
													scale );
		if( ! ok )
		{
			DEBUG_PRINT( "SeparateAndCreateFixtures() failed to add any fixtures." );

			// This is ONLY meant to be an "outline", so
			// we'll let it return true in all cases.
		}

		lua_pop( L, 1 );
		return true;
	}

	lua_pop( L, 1 );
	return false;
}

static bool
InitializeFixtureUsing_Radius( lua_State *L,
								int lua_arg_index,
								int &fixtureIndex,
								b2Vec2 &center_in_pixels,
								DisplayObject *display_object,
								b2Body *body,
								float meter_per_pixels_scale )
{
	lua_getfield( L, lua_arg_index, "radius" );
	if ( lua_isnumber( L, -1 ) )
	{
		DEBUG_PRINT( "%s\n", __FUNCTION__ );

		b2FixtureDef fixtureDef;

		// This is a circular shape :: TODO: add optional x,y properties for offset circles? (for multi-shape case)
		b2CircleShape circleDef;
		circleDef.m_radius = Rtt_REAL_16TH; // default to 1/16th of a meter
		Real radius = Rtt_FloatToReal( lua_tonumber( L, -1 ) );
		radius *= meter_per_pixels_scale; // Convert to meters.
		if ( radius < Rtt_REAL_0 )
		{
			radius = Rtt_REAL_16TH;
		}
		circleDef.m_radius = Rtt_RealToFloat( radius );
		
		circleDef.m_p = ( center_in_pixels * meter_per_pixels_scale );

		InitializeFixtureFromLua( L,
									fixtureDef,
									&circleDef,
									lua_arg_index );

		_FixtureCreator( body,
							&fixtureDef,
							fixtureIndex );

		lua_pop( L, 1 );
		return true;
	}

	lua_pop( L, 1 );
	return false;
}

static bool
InitializeFixtureUsing_Chain( lua_State *L,
								int lua_arg_index,
								int &fixtureIndex,
								b2Vec2 &center_in_pixels,
								DisplayObject *display_object,
								b2Body *body,
								float meter_per_pixels_scale )
{
	lua_getfield( L, lua_arg_index, "connectFirstAndLastChainVertex" );
	bool connectFirstAndLastChainVertex = ( lua_isboolean( L, -1 ) &&
											lua_toboolean( L, -1 ) );
	lua_pop( L, 1 );

	lua_getfield( L, lua_arg_index, "chain" );
	if ( lua_istable( L, -1 ) )
	{
		DEBUG_PRINT( "%s\n", __FUNCTION__ );

		// This is an arbitrary chain
		b2Vec2Vector vertexList;
		_FromLua_to_b2Vec2Vector( L,
									vertexList );

		// Convert to meters.
		_Transform_b2Vec2Vector( vertexList,
									center_in_pixels,
									meter_per_pixels_scale );

		b2FixtureDef fixtureDef;

		b2ChainShape chainDef;

		if( connectFirstAndLastChainVertex )
		{
			if( vertexList.size() >= 3 )
			{
				chainDef.CreateLoop( &vertexList[ 0 ],
										(int)vertexList.size() );
			}
			else
			{
				CoronaLuaError( L, "physics.addBody() with a \"chain\" requires at least 3 vertices." );

				lua_pop( L, 1 );
				// true: No one else should handle this because
				// this is ONLY meant to be a "chain".
				return true;
			}
		}
		else
		{
			if( vertexList.size() >= 2 )
			{
				chainDef.CreateChain( &vertexList[ 0 ],
										(int)vertexList.size() );
			}
			else
			{
				CoronaLuaError( L, "physics.addBody() with a \"chain\" requires at least 3 vertices." );

				lua_pop( L, 1 );
				// true: No one else should handle this because
				// this is ONLY meant to be a "chain".
				return true;
			}
		}

		InitializeFixtureFromLua( L,
									fixtureDef,
									&chainDef,
									lua_arg_index );

		_FixtureCreator( body,
							&fixtureDef,
							fixtureIndex );

		lua_pop( L, 1 );
		return true;
	}

	lua_pop( L, 1 );
	return false;
}

static bool
InitializeFixtureUsing_Box( lua_State *L,
							int lua_arg_index,
							int &fixtureIndex,
							b2Vec2 &center_in_pixels,
							DisplayObject *display_object,
							b2Body *body,
							float meter_per_pixels_scale )
{
	lua_getfield( L, lua_arg_index, "box" );
	if ( lua_istable( L, -1 ) )
	{
		DEBUG_PRINT( "%s\n", __FUNCTION__ );

		Real pixels_per_meter_scale = ( 1.0f / meter_per_pixels_scale );

		lua_getfield( L, -1, "halfWidth" );
		float halfW = luaL_torealphysics( L, -1, pixels_per_meter_scale );
		lua_pop( L, 1 );

		lua_getfield( L, -1, "halfHeight" );
		float halfH = luaL_torealphysics( L, -1, pixels_per_meter_scale );
		lua_pop( L, 1 );

		bool hasCenter = true;
		//float angle = 0.f;

		lua_getfield( L, -1, "x" );
		hasCenter &= ( lua_type( L, -1 ) == LUA_TNUMBER );
		Real x = luaL_torealphysics( L, -1, pixels_per_meter_scale );
		lua_pop( L, 1 );

		lua_getfield( L, -1, "y" );
		hasCenter &= ( lua_type( L, -1 ) == LUA_TNUMBER );
		Real y = luaL_torealphysics( L, -1, pixels_per_meter_scale );
		lua_pop( L, 1 );

		lua_getfield( L, -1, "angle" );
		Real radians = Rtt_RealDegreesToRadians( luaL_toreal( L, -1 ) );
		lua_pop( L, 1 );

		b2FixtureDef fixtureDef;

		b2PolygonShape polygonDef;

		if( hasCenter )
		{
			center_in_pixels.Set( x, y );
		}

		polygonDef.SetAsBox( halfW,
								halfH,
								center_in_pixels,
								radians );

		InitializeFixtureFromLua( L,
									fixtureDef,
									&polygonDef,
									lua_arg_index );

		_FixtureCreator( body,
							&fixtureDef,
							fixtureIndex );

		lua_pop( L, 1 );
		return true;
	}

	lua_pop( L, 1 );
	return false;
}

static bool
InitializeFixtureUsing_Shape( lua_State *L,
								int lua_arg_index,
								int &fixtureIndex,
								b2Vec2 &center_in_pixels,
								DisplayObject *display_object,
								b2Body *body,
								float meter_per_pixels_scale )
{
	lua_getfield( L, lua_arg_index, "shape" );
	if ( lua_istable( L, -1 ) )
	{
		DEBUG_PRINT( "%s\n", __FUNCTION__ );

		// This is an arbitrary polygonal shape.
		b2Vec2Vector vertexList;
		_FromLua_to_b2Vec2Vector( L,
									vertexList );
		if( vertexList.size() < 3 )
		{
			CoronaLuaError( L, "physics.addBody() with a \"shape\" requires at least 3 vertices." );

			lua_pop( L, 1 );
			// true: No one else should handle this because
			// this is ONLY meant to be a "shape".
			return true;
		}

		// Convert to meters.
		_Transform_b2Vec2Vector( vertexList,
									center_in_pixels,
									meter_per_pixels_scale );

		b2FixtureDef fixtureDef;

		b2PolygonShape polygonDef;

		bool ok = polygonDef.Set( &vertexList[ 0 ],
									(int)vertexList.size() );
		if( ok )
		{
			InitializeFixtureFromLua( L,
										fixtureDef,
										&polygonDef,
										lua_arg_index );

			_FixtureCreator( body,
								&fixtureDef,
								fixtureIndex );
		}
		else
		{
			CoronaLuaError( L, "physics.addBody() of a \"shape\" with no area, or nearly no area, has been rejected." );

			// This is ONLY meant to be a "shape", so
			// we'll let it return true in all cases.
		}

		lua_pop( L, 1 );
		return true;
	}

	lua_pop( L, 1 );
	return false;
}

static bool
add_b2Body_to_DisplayObject( lua_State *L,
								DisplayObject *display_object,
								int numArgs )
{
	const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
	float meter_per_pixels_scale = physics.GetMetersPerPixel();

	b2Vec2 center_in_pixels;
	if ( display_object->ShouldOffsetWithAnchor() )
	{
		Vertex2 offset = display_object->GetAnchorOffset();

		center_in_pixels.Set( offset.x,
								offset.y );
	}
	else
	{
		center_in_pixels.SetZero();
	}

	// Trimming is not per se connected to adjusting the group's center; for
	// backward compatibility purposes, though, we probably don't want to just
	// blindly apply this fix. It was identified along with some sprite trimming
	// issues, thus the check for that default, but something like "correctGroupBody"
	// might be more appropriate if we wanted fine-grained control.
	DisplayDefaults & defaults = LuaContext::GetRuntime( L )->GetDisplay().GetDefaults();
	bool isTrimCorrected = defaults.IsImageSheetFrameTrimCorrected();
	
	if ( isTrimCorrected && display_object->AsGroupObject() )
	{
		Rect bounds;
		display_object->GetSelfBounds( bounds );

		Vertex2 center;
		bounds.GetCenter( center );
		center_in_pixels.x += center.x;
		center_in_pixels.y += center.y;
	}
	
	b2World *world = physics.GetWorld();
	b2Body *body = CreateBody( physics, display_object );

	int firstFixtureArg = 2;

	if ( lua_isstring( L, 2 ) )
	{
		++firstFixtureArg;

		const char *bodyType = lua_tostring( L, 2 );
		b2BodyType t = b2_dynamicBody; // default
		if ( bodyType )
		{
			if ( strcmp( kStaticBodyType, bodyType ) == 0 )
			{
				t = b2_staticBody;
			}
			else if ( strcmp( kKinematicBodyType, bodyType ) == 0 )
			{
				t = b2_kinematicBody;
			}
		}
		body->SetType( t );
	}

	if ( ! lua_istable( L, firstFixtureArg ) )
	{
		//THIS SHOULD BE REPLACED, AS MUCH AS POSSIBLE,
		//BY "InitializeFixtureUsing_Rectangle()"!!!!!!!!!!!!!!!

		// This code is hit when we use addBody() this way:
		//		physics.addBody( my_ball0, "dynamic" )

		// No body elements are declared at all, so we should fall back to default material values for the (implicit) single body element.
		// This should be the same behavior as the default case in the loop below: the body boundaries will be a rectangle that
		// snaps to the boundaries of the display object. The result is that declaring no elements is now the same as declaring an
		// empty table, which is the desired outcome -- see Fogbugz #1671.

		b2FixtureDef fixtureDef;
		b2PolygonShape polygonDef;

		Real halfW = Rtt_RealDiv2( display_object->GetGeometricProperty( kWidth ) );
		Real halfH = Rtt_RealDiv2( display_object->GetGeometricProperty( kHeight ) );

		// Convert to meters.
		halfW *= meter_per_pixels_scale;
		halfH *= meter_per_pixels_scale;

		polygonDef.SetAsBox( halfW,
								halfH,
								( center_in_pixels * meter_per_pixels_scale ),
								0.0f );

		InitializeFixturePhysicsDefaults( fixtureDef, &polygonDef );

		// Store fixture index in the fixture's userData
		// This is a low-overhead way to reidentify specific fixtures by index during Lua collision events; the fixture indexes
		// for each body are returned as integer values in "event.element1" and "event.element2" in the collision event. Also
		// note that Corona uses the term "body element" to mean Box2D's "fixture", when speaking of multi-element bodies.
		// See user documentation here: http://developer.coronalabs.com/content/game-edition-collision-detection#Collisions_with_multi-element_bodies

		b2Fixture *fixture = body->CreateFixture( & fixtureDef );

		// an ordered index for all fixtures, ranging from 1 to n
		intptr_t fixtureIndex = 1;

		fixture->SetUserData( (void *)fixtureIndex );
	}
	else
	{
		// This code is hit when we use addBody() this way:
		//		physics.addBody( my_ball0, "dynamic", {} )

		// Iterate over (1 to n) body elements

		int fixtureIndex = 1;

		for ( int lua_arg_index = firstFixtureArg;
				lua_arg_index <= numArgs;
				++lua_arg_index )
		{
			// Initialize the first type encountered.
			// The order of these calls is IMPORTANT.
			( InitializeFixtureUsing_Shape( L,
											lua_arg_index,
											fixtureIndex,
											center_in_pixels,
											display_object,
											body,
											meter_per_pixels_scale ) ||
				InitializeFixtureUsing_Box( L,
											lua_arg_index,
											fixtureIndex,
											center_in_pixels,
											display_object,
											body,
											meter_per_pixels_scale ) ||
				InitializeFixtureUsing_Chain( L,
											lua_arg_index,
											fixtureIndex,
											center_in_pixels,
											display_object,
											body,
											meter_per_pixels_scale ) ||
				InitializeFixtureUsing_Radius( L,
											lua_arg_index,
											fixtureIndex,
											center_in_pixels,
											display_object,
											body,
											meter_per_pixels_scale ) ||
				InitializeFixtureUsing_Outline( L,
											lua_arg_index,
											fixtureIndex,
											center_in_pixels,
											display_object,
											body,
											meter_per_pixels_scale ) ||
				InitializeFixtureUsing_ArbitraryPolygonalShape( L,
											lua_arg_index,
											fixtureIndex,
											center_in_pixels,
											display_object,
											body,
											meter_per_pixels_scale ) ||
				InitializeFixtureUsing_StaticLine( L,
											lua_arg_index,
											fixtureIndex,
											center_in_pixels,
											display_object,
											body,
											meter_per_pixels_scale ) ||
				InitializeFixtureUsing_Rectangle( L,
											lua_arg_index,
											fixtureIndex,
											center_in_pixels,
											display_object,
											body,
											meter_per_pixels_scale ) );
		} // end loop over fixtureArgs
	}

	if ( display_object->InitializeExtensions( physics.Allocator() ) )
	{
		DisplayObjectExtensions *extensions = display_object->GetExtensions();
		extensions->SetBody( body, *world );

		return true;
	}

	return false;
}

// physics.addBody( displayObject, [ bodyType,] bodyElement1 [, bodyElement2, ... ] )
//
// Parameters:
// 		bodyType is a string: "kinematic", "static", "dynamic"
// 		bodyElement1 is a table with following properties:
//			* density: number > 0
//			* friction: number > 0
//			* bounce: number
//			* An optional shape definition consisting of one of the following:
//				+ shape: array ( x1,y1, x2,y2, ... )
//				+ box: table:
//					- halfWidth (required)
//					- halfHeight (required)
//					- x, y, angle (optional)
//				+ radius: number > 0
// Note:
//	* If no shape definition is supplied then the shape defaults to DisplayObject's bounding box.
//	* If supplied, then the precedence order is: 'shape', 'box', 'radius'
//
static int
addBody( lua_State *L )
{
	if( LuaLibPhysics::IsWorldLocked( L, "physics.addBody()" ) )
	{
		lua_pushboolean( L, false );
		return 1;
	}

	// Assume the actual addBody fails (and set to true below if success)
	bool result = false;

	int numArgs = lua_gettop( L );

	DisplayObject *o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

	if( o &&
		Rtt_VERIFY( ! o->GetExtensions() ) )
	{
		result |= add_b2Body_to_DisplayObject( L, o, numArgs );
	}

	Rtt_ASSERT( lua_gettop( L ) == numArgs );

	// return whether succeeded or not
	lua_pushboolean( L, result );
	return 1;
}

static int
removeBody( lua_State *L )
{
	bool result = ! LuaLibPhysics::IsWorldLocked( L, "physics.removeBody()" );

	if ( result )
	{
		result = false;

		DisplayObject *o = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

		Rtt_WARN_SIM_PROXY_TYPE( L, 1, DisplayObject );

		if ( o )
		{
			result = ( NULL != o->GetExtensions() );

			if (result)
            {                
                o->RemoveExtensions();
            }
            else
            {
                CoronaLuaWarning(L, "physics.removeBody() given a display object that is not a physics object");
            }
		}
	}

	// return whether succeeded or not
	lua_pushboolean( L, result );
	return 1;
}

static const char kNormalDrawName[] = "normal"; // default state
static const char kHybridDrawName[] = "hybrid";
static const char kDebugDrawName[] = "debug";

static int
setDrawMode( lua_State *L )
{
	const char *drawType = lua_tostring( L, 1 );

	Runtime& runtime = * LuaContext::GetRuntime( L );
	Display& display = runtime.GetDisplay();

	if ( Rtt_StringCompare( kNormalDrawName, drawType ) == 0 )
	{
		display.SetDrawMode( Display::kInvalidateDrawMode );
	}
	else if ( Rtt_StringCompare( kHybridDrawName, drawType ) == 0 )
	{
		display.SetDrawMode( Display::kPhysicsHybridDrawMode );
	}
	else if ( Rtt_StringCompare( kDebugDrawName, drawType ) == 0 )
	{
		display.SetDrawMode( Display::kPhysicsDebugDrawMode );
	}
    else
    {
        CoronaLuaError(L, "physics.setDrawMode() parameter must be one of '%s', '%s' or '%s'",
                       kNormalDrawName, kHybridDrawName, kDebugDrawName);
    }

	return 0;
}

static int
setVelocityIterations( lua_State *L )
{
	if ( lua_isnumber( L, 1 ) )
	{
		PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		S32 velocityIterations = (S32) lua_tointeger( L, 1 );
		physics.SetVelocityIterations( velocityIterations );
	}
    else
    {
        CoronaLuaError(L, "physics.setVelocityIterations() requires 1 parameter (number)");
    }

	return 0;
}

static int
setPositionIterations( lua_State *L )
{
	if ( lua_isnumber( L, 1 ) )
	{
		PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		S32 positionIterations = (S32) lua_tointeger( L, 1 );
		physics.SetPositionIterations( positionIterations );
	}
    else
    {
        CoronaLuaError(L, "physics.setPositionIterations() requires 1 parameter (number)");
    }

	return 0;
}

static int
setContinuous( lua_State *L )
{
	if ( ! lua_isnone( L, 1 ) )
	{
		bool result = ! LuaLibPhysics::IsWorldLocked( L, "physics.setContinuous()" );

		if ( result )
		{
			PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
			physics.GetWorld()->SetContinuousPhysics( lua_toboolean( L, 1 ) );
		}
	}
	else
	{
		luaL_typerror( L, 1, lua_typename( L, LUA_TBOOLEAN ) );
	}

	return 0;
}

static int
setMKS( lua_State *L )
{
	bool result = ! LuaLibPhysics::IsWorldLocked( L, "physics.setMKS()" );

	if ( result )
	{
		const char *key = luaL_checkstring( L, 1 );
		lua_Number value = luaL_checknumber( L, 2 );

		if ( Rtt_StringCompare( "velocityThreshold", key ) == 0 )
		{
			b2Settings::velocityThreshold = value;
		}
		else if ( Rtt_StringCompare( "timeToSleep", key ) == 0 )
		{
			b2Settings::timeToSleep = value;
		}
		else if ( Rtt_StringCompare( "maxSubSteps", key ) == 0 )
		{
			b2Settings::maxSubSteps = (int) luaL_checkinteger( L, 2 );
		}
		else if ( Rtt_StringCompare( "linearSleepTolerance", key ) == 0 )
		{
			b2Settings::linearSleepTolerance = value;
			b2Settings::linearSleepToleranceSq = value * value;
		}
		else if ( Rtt_StringCompare( "angularSleepTolerance", key ) == 0 )
		{
			b2Settings::angularSleepTolerance = value;
			b2Settings::angularSleepToleranceSq = value * value;
		}
		else
		{
			CoronaLuaWarning(L, "physics.setMKS() does not support the %s property", key);
			result = false;
		}
	}

	// return whether succeeded or not
	lua_pushboolean( L, result );
	return 1;
}

static int
getMKS( lua_State *L )
{
	const char *key = luaL_checkstring( L, 1 );
	lua_Number value = 0.f;

	if ( Rtt_StringCompare( "velocityThreshold", key ) == 0 )
	{
		value = b2Settings::velocityThreshold;
	}
	else if ( Rtt_StringCompare( "timeToSleep", key ) == 0 )
	{
		value = b2Settings::timeToSleep;
	}
	else if ( Rtt_StringCompare( "maxSubSteps", key ) == 0 )
	{
		value = b2Settings::maxSubSteps;
	}
	else if ( Rtt_StringCompare( "linearSleepTolerance", key ) == 0 )
	{
		value = b2Settings::linearSleepTolerance;
	}
	else if ( Rtt_StringCompare( "angularSleepTolerance", key ) == 0 )
	{
		value = b2Settings::angularSleepTolerance;
	}
	else
	{
		CoronaLuaWarning(L, "physics.getMKS() does not support the %s property", key);
	}

	lua_pushnumber( L, value );

	return 1;
}

LuaLibPhysics::PhysicsUnitType
PhysicsUnitTypeForString( const char *name )
{
	LuaLibPhysics::PhysicsUnitType result = LuaLibPhysics::kUnknownUnitType;

	if ( 0 == Rtt_StringCompareNoCase( "length", name ) )
	{
		result = LuaLibPhysics::kLengthUnitType;
	}
	else if ( 0 == Rtt_StringCompareNoCase( "velocity", name ) )
	{
		result = LuaLibPhysics::kVelocityUnitType;
	}
	else if ( 0 == Rtt_StringCompareNoCase( "angularVelocity", name ) )
	{
		result = LuaLibPhysics::kAngularVelocityUnitType;
	}

	return result;
}

float
LuaLibPhysics::ToMKS( PhysicsUnitType unitType, const PhysicsWorld& physics, float value )
{
	float result = value;

	switch ( unitType )
	{
		case kLengthUnitType:
		case kVelocityUnitType:
			result = ( value / physics.GetPixelsPerMeter() );
			break;
		case kAngularVelocityUnitType:
			result = value * M_PI / 180.f;
			break;
		default:
			break;
	}

	return result;
}

float
LuaLibPhysics::FromMKS( PhysicsUnitType unitType, const PhysicsWorld& physics, float value )
{
	float result = value;

	switch ( unitType )
	{
		case kLengthUnitType:
		case kVelocityUnitType:
			result = ( value * physics.GetPixelsPerMeter() );
			break;
		case kAngularVelocityUnitType:
			result = value * 180.f / M_PI;
			break;
		default:
			break;
	}

	return result;
}

// physics.toMKS( unitName, value )
// Converts 'value' for the given physical 'unitName' type from Corona units to MKS
static int
toMKS( lua_State *L )
{
	const char *unitName = lua_tostring( L, 1 );
	double result = lua_tonumber( L, 2 );

	LuaLibPhysics::PhysicsUnitType unitType = PhysicsUnitTypeForString( unitName );

	if ( LuaLibPhysics::kUnknownUnitType != unitType )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		result = LuaLibPhysics::ToMKS( unitType, physics, result );
	}
    else
    {
        CoronaLuaError(L, "physics.toMKS() unit type '%s' not recognized", unitName);
    }

	lua_pushnumber( L, result );
	return 1;
}

// physics.fromMKS( unitName, value )
// Converts 'value' for the given physical 'unitName' type from MKS to Corona units
static int
fromMKS( lua_State *L )
{
	const char *unitName = lua_tostring( L, 1 );
	double result = lua_tonumber( L, 2 );

	LuaLibPhysics::PhysicsUnitType unitType = PhysicsUnitTypeForString( unitName );

	if ( LuaLibPhysics::kUnknownUnitType != unitType )
	{
		const PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		result = LuaLibPhysics::FromMKS( unitType, physics, result );
	}
    else
    {
        CoronaLuaError(L, "physics.fromMKS() unit type '%s' not recognized", unitName);
    }

	lua_pushnumber( L, result );
	return 1;
}

// physics.setTimeStep( dt )
// Sets time step of physics simulator to 'dt' (in seconds) for each enterFrame.
// If 'dt' is set to 0, then simulation is time-based with an error in time being
// the desired frame interval. If 'dt' is < 0, then the time step is set to the
// desired frame interval.
static int
setTimeStep( lua_State *L )
{
	if ( LUA_TNUMBER == lua_type( L, 1 ) )
	{
		PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		physics.SetTimeStep( lua_tonumber( L, 1 ) );
	}
    else
    {
        CoronaLuaError(L, "physics.setTimeStep() requires 1 parameter (number)");
    }

	return 0;
}

// physics.setTimeScale( dt )
// Sets time scale of physics simulator. Default is 1
static int
setTimeScale( lua_State *L )
{
	if ( LUA_TNUMBER == lua_type( L, 1 ) )
	{
		PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
		physics.SetTimeScale( lua_tonumber( L, 1 ) );
	}
	else
	{
		CoronaLuaError(L, "physics.setTimeScale() requires 1 parameter (number)");
	}

	return 0;
}

// physics.getTimeScale( )
// Returns time scale of physics simulator.
static int
getTimeScale( lua_State *L )
{
	PhysicsWorld& physics = LuaContext::GetRuntime( L )->GetPhysicsWorld();
	lua_pushnumber(L, physics.GetTimeScale());
	return 1;
}

int
LuaLibPhysics::Open( lua_State *L )
{
#ifdef Rtt_AUTHORING_SIMULATOR
	LuaContext::RegisterModule( L, LuaContext::kPhysicsModuleMask );
#endif

	const luaL_Reg kVTable[] =
	{
		{ "start", start },
		{ "pause", pause },
		{ "stop", stop },
		{ "setGravity", setGravity },
		{ "getGravity", getGravity },
		{ "setReportCollisionsInContentCoordinates", SetReportCollisionsInContentCoordinates },
		{ "getReportCollisionsInContentCoordinates", GetReportCollisionsInContentCoordinates },
		{ "setDebugErrorsEnabled", SetDebugErrorsEnabled },
		{ "getDebugErrorsEnabled", GetDebugErrorsEnabled },
		{ "rayCast", RayCast },
		{ "reflectRay", ReflectRay },
		{ "queryRegion", QueryRegion },
		{ "setAverageCollisionPositions", SetAverageCollisionPositions },
		{ "getAverageCollisionPositions", GetAverageCollisionPositions },
		{ "setScale", setScale },
		{ "newJoint", newJoint },
		{ "newParticleSystem", newParticleSystem },
		{ "addBody", addBody },
		{ "removeBody", removeBody },
		{ "setDrawMode", setDrawMode },
		{ "setVelocityIterations", setVelocityIterations },
		{ "setPositionIterations", setPositionIterations },
		{ "setContinuous", setContinuous },
		{ "setMKS", setMKS },
		{ "getMKS", getMKS },
		{ "toMKS", toMKS },
		{ "fromMKS", fromMKS },
		{ "setTimeStep", setTimeStep },
		{ "setTimeScale", setTimeScale },
		{ "getTimeScale", getTimeScale },

		{ NULL, NULL }
	};

	PhysicsContact::Initialize( L );
	PhysicsJoint::Initialize( L );

	luaL_register( L, "physics", kVTable );

	// Set the "version" property.
	{
		char s[ 128 ];

		snprintf( s,
					( sizeof( s ) - sizeof( s[ 0 ] ) ),
					"Box2D %d.%d.%d with %s",
					b2_version.major,
					b2_version.minor,
					b2_version.revision,
					b2_liquidFunVersionString );

		lua_pushstring( L, s );
		lua_setfield( L, -2, "engineVersion" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_PHYSICS
