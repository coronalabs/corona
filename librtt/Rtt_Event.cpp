//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_Event.h"

#ifdef Rtt_AUTHORING_SIMULATOR
	// scene is only required for simulator hit testing against status bar or overlay
	#include "Display/Rtt_Scene.h"
#endif

#include "Rtt_PhysicsWorld.h"
#include "Rtt_ParticleSystemObject.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_StageObject.h"
#include "Input/Rtt_PlatformInputAxis.h"
#include "Input/Rtt_PlatformInputDevice.h"
#include "Rtt_Lua.h"
#include "Display/Rtt_BitmapMask.h"
#include "Rtt_HitTestObject.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaResource.h"
#include "Rtt_Runtime.h"
#include "Display/Rtt_SpriteObject.h"

#include "Box2D/Box2D.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char MEvent::kNameKey[] = "name";
const char MEvent::kProviderKey[] = "provider";
const char MEvent::kPhaseKey[] = "phase";
const char MEvent::kTypeKey[] = "type";
const char MEvent::kResponseKey[] = "response";
const char MEvent::kIsErrorKey[] = "isError";
const char MEvent::kErrorCodeKey[] = "errorCode";
const char MEvent::kDataKey[] = "data";

// Deprecated: We should favor kResponseKey in favor of this
// since we can use kIsErrorKey to tell us how to interpret the response
static const char kErrorMessageKey[] = "errorMessage";

// ----------------------------------------------------------------------------

VirtualEvent::~VirtualEvent()
{
}

int
VirtualEvent::PrepareDispatch( lua_State *L ) const
{
	Lua::PushRuntime( L );
	lua_getfield( L, -1, "dispatchEvent" );
	lua_insert( L, -2 ); // swap stack positions for "Runtime" and "dispatchEvent"
	return 1 + Push( L );
}

void
VirtualEvent::Dispatch( lua_State *L, Runtime& ) const
{
	// Invoke Lua code: "Runtime:dispatchEvent( eventKey )"
	int nargs = PrepareDispatch( L );
	LuaContext::DoCall( L, nargs, 0 );
}

int
VirtualEvent::Push( lua_State *L ) const
{
	Lua::NewEvent( L, Name() ); Rtt_ASSERT( lua_istable( L, -1 ) );
	return 1;
}

// ----------------------------------------------------------------------------

ErrorEvent::ErrorEvent()
:	fErrorMsg( NULL ),
	fErrorCode( 0 )
{
}

int
ErrorEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		if ( fErrorMsg )
		{
			lua_pushstring( L, fErrorMsg );
			lua_setfield( L, -2, kErrorMessageKey  );

			lua_pushinteger( L, fErrorCode );
			lua_setfield( L, -2, kErrorCodeKey );
		}
	}
	return 1;
}

void
ErrorEvent::SetError( const char *errorMsg, int errorCode )
{
	fErrorMsg = errorMsg;
	fErrorCode = errorCode;
}

// ----------------------------------------------------------------------------

const FrameEvent&
FrameEvent::Constant()
{
	static const FrameEvent kEvent;
	return kEvent;
}

FrameEvent::FrameEvent()
{
}

const char*
FrameEvent::Name() const
{
	static const char kName[] = "enterFrame";
	return kName;
}

int
FrameEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Runtime *runtime = LuaContext::GetRuntime( L );
		lua_pushnumber( L, runtime->GetFrame() );
		lua_setfield( L, -2, "frame" );
		lua_pushnumber( L, runtime->GetElapsedMS() );
		lua_setfield( L, -2, "time" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

const RenderEvent&
RenderEvent::Constant()
{
	static const RenderEvent kEvent;
	return kEvent;
}

RenderEvent::RenderEvent()
{
}

const char*
RenderEvent::Name() const
{
	static const char kName[] = "lateUpdate";
	return kName;
}

int
RenderEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Runtime *runtime = LuaContext::GetRuntime( L );
		lua_pushnumber( L, runtime->GetFrame() );
		lua_setfield( L, -2, "frame" );
		lua_pushnumber( L, runtime->GetElapsedMS() );
		lua_setfield( L, -2, "time" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char*
SystemEvent::StringForType( Type type )
{
	const char* result = NULL;
	static const char kOnAppExitString[] = "applicationExit";
	static const char kOnAppStartString[] = "applicationStart";
	static const char kOnAppSuspendString[] = "applicationSuspend";
	static const char kOnAppResumeString[] = "applicationResume";
	static const char kOnAppOpenString[] = "applicationOpen";

	switch( type )
	{
		case kOnAppExit:
			result = kOnAppExitString;
			break;
		case kOnAppStart:
			result = kOnAppStartString;
			break;
		case kOnAppSuspend:
			result = kOnAppSuspendString;
			break;
		case kOnAppResume:
			result = kOnAppResumeString;
			break;
		case kOnAppOpen:
			result = kOnAppOpenString;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

SystemEvent::SystemEvent( Type type )
:	fType( type )
{
}

const char*
SystemEvent::Name() const
{
	static const char kName[] = "system";
	return kName;
}

int
SystemEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		const char* value = StringForType( (Type)fType );
		if ( value )
		{
			lua_pushstring( L, value );
			lua_setfield( L, -2, kTypeKey );
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

InternalSystemEvent::InternalSystemEvent( Type type )
:	SystemEvent( type )
{
}
	
const char*
InternalSystemEvent::Name() const
{
	static const char kName[] = "_internalSystem";
	return kName;
}
	
// ----------------------------------------------------------------------------
	
SystemOpenEvent::SystemOpenEvent( const char *url )
:	Super( kOnAppOpen ),
	fUrl( url ),
	fCommandLineDirectoryPath( NULL ),
	fCommandLineArgumentArray( NULL ),
	fCommandLineArgumentCount( 0 )
{
}

void
SystemOpenEvent::SetCommandLineArgs(int argumentCount, const char **arguments)
{
	fCommandLineArgumentCount = argumentCount;
	fCommandLineArgumentArray = arguments;
}

void
SystemOpenEvent::SetCommandLineDir(const char *directoryPath)
{
	fCommandLineDirectoryPath = directoryPath;
}

int
SystemOpenEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		if ( fUrl )
		{
			lua_pushstring( L, fUrl );
			lua_setfield( L, -2, "url" );
		}

		lua_createtable( L, fCommandLineArgumentCount > 0 ? fCommandLineArgumentCount : 0, 0 );
		if ( fCommandLineArgumentArray && ( fCommandLineArgumentCount > 0 ) )
		{
			for ( int index = 0; index < fCommandLineArgumentCount; index++ )
			{
				const char* stringPointer = fCommandLineArgumentArray[ index ];
				lua_pushstring( L, stringPointer ? stringPointer : "" );
				lua_rawseti( L, -2, index + 1 );
			}
		}
		lua_setfield( L, -2, "commandLineArgs" );

		if ( fCommandLineDirectoryPath )
		{
			lua_pushstring( L, fCommandLineDirectoryPath );
			lua_setfield( L, -2, "commandLineDir" );
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

OrientationEvent::OrientationEvent(
	DeviceOrientation::Type type,
	DeviceOrientation::Type previous )
:	fType( type ),
	fPreviousType( previous )
{
}

const char*
OrientationEvent::Name() const
{
	static const char kName[] = "orientation";
	return kName;
}

int
OrientationEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		const char* value = DeviceOrientation::StringForType( (DeviceOrientation::Type)fType );
		if ( value )
		{
			lua_pushstring( L, value );
			lua_setfield( L, -2, kTypeKey );
		}

		// Minus sign, b/c DeviceOrientation::CalculateRotation assumes positive angles
		// are in the CCW direction whereas in our drawing model, positive angles are CW.
		lua_pushinteger( L, - DeviceOrientation::CalculateRotation( (DeviceOrientation::Type)fPreviousType, (DeviceOrientation::Type)fType ) );
		lua_setfield( L, -2, "delta" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

/// Creates a new event indicating that the main app window has been resized.
ResizeEvent::ResizeEvent()
{
}

const char*
ResizeEvent::Name() const
{
	static const char kName[] = "resize";
	return kName;
}

int
ResizeEvent::Push( lua_State *L ) const
{
	return Super::Push( L );
}

// ----------------------------------------------------------------------------

/// Creates a new event indicating that the main was activated or deactivated
WindowStateEvent::WindowStateEvent(bool foreground) : fForeground(foreground)
{
}

const char*
WindowStateEvent::Name() const
{
	static const char kName[] = "windowState";
	return kName;
}

int
WindowStateEvent::Push( lua_State *L ) const
{
	Super::Push( L );
	lua_pushstring( L, fForeground?"foreground":"background");
	lua_setfield( L, -2, kPhaseKey );
	return 1;
}


// ----------------------------------------------------------------------------

/// Creates a new event data object that stores a single accelerometer measurement.
/// @param gravity Pointer to an array of 3 elements storing smoothed acceleration data.
/// @param instant Pointer to an array of 3 elements storing acceleration deltas based on smoother/gravity data.
/// @param raw Pointer to an array of 3 elements storing raw acceleration data.
/// @param isShake Set true if the device was shaken.
/// @param deltaTime Number of seconds since the last measurement.
AccelerometerEvent::AccelerometerEvent(
	double* gravity, double* instant, double* raw, bool isShake, double deltaTime, PlatformInputDevice *device )
:	fGravityAccel( gravity ),
	fInstantAccel( instant ),
	fRawAccel( raw ),
	fIsShake( isShake ),
	fDeltaTime( deltaTime ),
	fDevice( device )
{
}

const char*
AccelerometerEvent::Name() const
{
	static const char kName[] = "accelerometer";
	return kName;
}

static void
PushAccelerationComponents( lua_State *L, double* accel, const char* labels[] )
{
	lua_pushnumber( L, accel[0] );
	lua_setfield( L, -2, labels[0] );
	lua_pushnumber( L, accel[1] );
	lua_setfield( L, -2, labels[1] );
	lua_pushnumber( L, accel[2] );
	lua_setfield( L, -2, labels[2] );
}

int
AccelerometerEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		const char *gravityLabels[] = { "xGravity", "yGravity", "zGravity" };
		PushAccelerationComponents( L, fGravityAccel, gravityLabels );

		const char *instantLabels[] = { "xInstant", "yInstant", "zInstant" };
		PushAccelerationComponents( L, fInstantAccel, instantLabels );

		const char *rawLabels[] = { "xRaw", "yRaw", "zRaw" };
		PushAccelerationComponents( L, fRawAccel, rawLabels );

		lua_pushboolean( L, fIsShake );
		lua_setfield( L, -2, "isShake" );
		
		lua_pushnumber( L, fDeltaTime );
		lua_setfield( L, -2, "deltaTime" );
		
		if (fDevice)
		{
			fDevice->PushTo( L );
			lua_setfield( L, -2, "device" );
		}

	}

	return 1;
}

// ----------------------------------------------------------------------------

/// Creates a new event data object that stores a single gyroscope measurement.
/// @param xRotation Rate of rotation around the x-axis in radians per second.
/// @param yRotation Rate of rotation around the y-axis in radians per second.
/// @param zRotation Rate of rotation around the z-axis in radians per second.
/// @param deltaTime Number of seconds since the last measurement.
GyroscopeEvent::GyroscopeEvent( double xRotation, double yRotation, double zRotation, double deltaTime, PlatformInputDevice *device )
:	fXRotation( xRotation ),
	fYRotation( yRotation ),
	fZRotation( zRotation ),
	fDeltaTime( deltaTime ),
	fDevice(device)
{
}

const char*
GyroscopeEvent::Name() const
{
	static const char kName[] = "gyroscope";
	return kName;
}

int
GyroscopeEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushnumber( L, fXRotation );
		lua_setfield( L, -2, "xRotation" );
		lua_pushnumber( L, fYRotation );
		lua_setfield( L, -2, "yRotation" );
		lua_pushnumber( L, fZRotation );
		lua_setfield( L, -2, "zRotation" );
		lua_pushnumber( L, fDeltaTime );
		lua_setfield( L, -2, "deltaTime" );
		if (fDevice)
		{
			fDevice->PushTo( L );
			lua_setfield( L, -2, "device" );
		}
	}
	
	return 1;
}

// ----------------------------------------------------------------------------

const char LocationEvent::kLatitudeKey[] = "latitude";
const char LocationEvent::kLongitudeKey[] = "longitude";
const char LocationEvent::kAltitudeKey[] = "altitude";
const char LocationEvent::kAccuracyKey[] = "accuracy";
const char LocationEvent::kSpeedKey[] = "speed";
const char LocationEvent::kDirectionKey[] = "direction";
const char LocationEvent::kTimeKey[] = "time";
const char LocationEvent::kIsUpdatingKey[] = "isUpdating";

LocationEvent::LocationEvent( const char *errorMsg, int errorCode )
:	Super(),
	fAccuracy( -1. ),
	fDirection( -1. )
{
	SetError( errorMsg, errorCode );
}

LocationEvent::LocationEvent(
					double latitude, double longitude,
					double altitude, double accuracy,
					double speed, double direction, double time )
:	Super(),
	fLatitude( latitude ),
	fLongitude( longitude ),
	fAltitude( altitude ),
	fAccuracy( accuracy ),
	fSpeed( speed ),
	fDirection( direction ),
	fTime( time )
{
}

const char*
LocationEvent::Name() const
{
	static const char kName[] = "location";
	return kName;
}

int
LocationEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		if ( IsValid() )
		{
			lua_pushnumber( L, fLatitude );
			lua_setfield( L, -2, kLatitudeKey );

			lua_pushnumber( L, fLongitude );
			lua_setfield( L, -2, kLongitudeKey );

			lua_pushnumber( L, fAltitude );
			lua_setfield( L, -2, kAltitudeKey );

			lua_pushnumber( L, fAccuracy );
			lua_setfield( L, -2, kAccuracyKey );

			lua_pushnumber( L, fSpeed );
			lua_setfield( L, -2, kSpeedKey );

			lua_pushnumber( L, fDirection );
			lua_setfield( L, -2, kDirectionKey );

			lua_pushnumber( L, fTime );
			lua_setfield( L, -2, kTimeKey );
		}
	}
	
	return 1;
}

// ----------------------------------------------------------------------------

HeadingEvent::HeadingEvent( double geographic, double magnetic )
:	fGeographic( geographic ),
	fMagnetic( magnetic )
{
}

const char*
HeadingEvent::Name() const
{
	static const char kName[] = "heading";
	return kName;
}

int
HeadingEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushnumber( L, fGeographic );
		lua_setfield( L, -2, "geographic" );
		
		lua_pushnumber( L, fMagnetic );
		lua_setfield( L, -2, "magnetic" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

MemoryWarningEvent::MemoryWarningEvent( )
{
}

const char*
MemoryWarningEvent::Name() const
{
	static const char kName[] = "memoryWarning";
	return kName;
}

int
MemoryWarningEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		// No fields currently in memory warning events	
	}
	return 1;
}

// ----------------------------------------------------------------------------

const char*
NotificationEvent::StringForType( Type type )
{
	static const char kLocalString[] = "local";
	static const char kRemoteString[] = "remote";
	static const char kRemoteRegistrationString[] = "remoteRegistration";

	const char *result = kLocalString;

	switch ( type )
	{
		case kRemote:
			result = kRemoteString;
			break;
		case kRemoteRegistration:
			result = kRemoteRegistrationString;
		default:
			break;
	}

	return result;
}

const char*
NotificationEvent::StringForApplicationState( ApplicationState state )
{
	static const char kBackgroundString[] = "background";
	static const char kActiveString[] = "active";
	static const char kInactiveString[] = "inactive";

	const char *result = kBackgroundString;
	switch ( state )
	{
		case kActive:
			result = kActiveString;
			break;
		case kInactive:
			result = kInactiveString;
			break;
		default:
			break;
	}

	return result;
}

NotificationEvent::NotificationEvent( Type t, ApplicationState state )
:	fType( t ),
	fApplicationState( state )
{
}

const char*
NotificationEvent::Name() const
{
	static const char kName[] = "notification";
	return kName;
}

int
NotificationEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushstring( L, StringForType( (Type)fType ) );
		lua_setfield( L, -2, kTypeKey );

		lua_pushstring( L, StringForApplicationState( (ApplicationState)fApplicationState ) );
		lua_setfield( L, -2, "applicationState" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

// This event is triggered by a delegate method of the native API's reverse geocoder
MapAddressEvent::MapAddressEvent( const char *street, const char *streetDetail, const char *city, const char *cityDetail, const char *region, const char *regionDetail, const char *postalCode, const char *country, const char *countryCode )
:	Super(),
	fStreet( street ),
	fStreetDetail( streetDetail ),
	fCity( city ),
	fCityDetail( cityDetail ),
	fRegion( region ),
	fRegionDetail( regionDetail ),
	fPostalCode( postalCode ),
	fCountry( country ),
	fCountryCode( countryCode ),
	fIsError( false )
{
}

MapAddressEvent::MapAddressEvent( const char *errorMsg, int errorCode )
:	Super(),
	fStreet( NULL ),
	fStreetDetail( NULL ),
	fCity( NULL ),
	fCityDetail( NULL ),
	fRegion( NULL ),
	fRegionDetail( NULL ),
	fPostalCode( NULL ),
	fCountry( NULL ),
	fCountryCode( NULL ),
	fIsError( true )
{
	SetError( errorMsg, errorCode );
}

const char*
MapAddressEvent::Name() const
{
	static const char kName[] = "mapAddress";
	return kName;
}

int
MapAddressEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		if ( ! fIsError )
		{
			lua_pushstring( L, fStreet );
			lua_setfield( L, -2, "street" );
			lua_pushstring( L, fStreetDetail );
			lua_setfield( L, -2, "streetDetail" );
			lua_pushstring( L, fCity );
			lua_setfield( L, -2, "city" );
			lua_pushstring( L, fCityDetail );
			lua_setfield( L, -2, "cityDetail" );
			lua_pushstring( L, fRegion );
			lua_setfield( L, -2, "region" );
			lua_pushstring( L, fRegionDetail );
			lua_setfield( L, -2, "regionDetail" );
			lua_pushstring( L, fPostalCode );
			lua_setfield( L, -2, "postalCode" );
			lua_pushstring( L, fCountry );
			lua_setfield( L, -2, "country" );
			lua_pushstring( L, fCountryCode );
			lua_setfield( L, -2, "countryCode" );
		}
		lua_pushboolean( L, fIsError );
		lua_setfield( L, -2, kIsErrorKey );
	}
	return 1;
}

// ----------------------------------------------------------------------------

// This event is triggered by a delegate method of the native API's geocoder.
MapLocationEvent::MapLocationEvent( double latitude, double longitude )
:	Super(),
	fLatitude( latitude ),
	fLongitude( longitude ),
	fRequestedLocation( NULL ),
	fIsError( false ),
	fType(kUIType)
{
}

MapLocationEvent::MapLocationEvent( double latitude, double longitude, const char *requestedLocation )
:	Super(),
	fLatitude( latitude ),
	fLongitude( longitude ),
	fRequestedLocation( requestedLocation),
	fIsError( false ),
	fType(kRequestType)
{
}

MapLocationEvent::MapLocationEvent( const char *errorMsg, int errorCode, const char *requestedLocation, Type type )
:	Super(),
	fLatitude( 0.0 ),
	fLongitude( 0.0 ),
	fRequestedLocation( requestedLocation ),
	fIsError( true ),
	fType(type)
{
	SetError( errorMsg, errorCode );
}

const char MapLocationEvent::kName[] = "mapLocation";

const char*
MapLocationEvent::Name() const
{
	return Self::kName;
}

int
MapLocationEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		if ( ! fIsError )
		{
			lua_pushnumber( L, fLatitude );
			lua_setfield( L, -2, "latitude" );
			lua_pushnumber( L, fLongitude );
			lua_setfield( L, -2, "longitude" );
		}

		const char *typeValue = StringForType( (Type)fType );  // kRequestType || kUIType
		lua_pushstring( L, typeValue );
		lua_setfield( L, -2, "type" );

		if ( kRequestType == fType && fRequestedLocation)
		{
			// The original string whos location was requested
			lua_pushstring( L, fRequestedLocation );
			lua_setfield( L, -2, "request" );
		}

		lua_pushboolean( L, fIsError );
		lua_setfield( L, -2, kIsErrorKey );
	}
	return 1;
}

const char*
MapLocationEvent::StringForType( Type t )
{
	const char* result = NULL;
	static const char kRequestString[] = "request";
	static const char kUIString[] = "ui";

	switch( t )
	{
		case kRequestType:
			result = kRequestString;
			break;
		case kUIType:
			result = kUIString;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

MapMarkerEvent::MapMarkerEvent( int markerId, double latitude, double longitude )
:	Super(),
	fMarkerId(markerId),
	fLatitude(latitude),
	fLongitude(longitude)
{
}

const char*
MapMarkerEvent::Name() const
{
	return "mapMarker";
}

int
MapMarkerEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushstring( L, Name() );
		lua_setfield( L, -2, kTypeKey );
		lua_pushnumber( L, fMarkerId);
		lua_setfield( L, -2, "markerId" );
		lua_pushnumber( L, fLatitude );
		lua_setfield( L, -2, "latitude" );
		lua_pushnumber( L, fLongitude );
		lua_setfield( L, -2, "longitude" );
	}
	return 1;
}

// ----------------------------------------------------------------------------

#ifdef Rtt_PHYSICS

BaseCollisionEvent::BaseCollisionEvent( DisplayObject& object1, DisplayObject& object2, Real x, Real y, int fixtureIndex1, int fixtureIndex2 )
:	fObject1( object1 ),
	fObject2( object2 ),
	fX( x ),
	fY( y ),
	fFixtureIndex1( fixtureIndex1 ),
	fFixtureIndex2( fixtureIndex2 ),
	fOther( NULL ),
	fContactWrapper( NULL )
{
}

int
BaseCollisionEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		if ( fOther ) 
		{
			// Only define "other" if this is a table listener
			fOther->GetProxy()->PushTable( L );
			lua_setfield( L, -2, "other" );

			// Assume self is fObject1			
			int selfElement = fFixtureIndex1;
			int otherElement = fFixtureIndex2;

			// If fObject1 is other, then self is fObject2, so swap
			if ( & fObject1 == fOther )
			{
				Swap( selfElement, otherElement );
			} 

			lua_pushnumber( L, selfElement );
			lua_setfield( L, -2, "selfElement" );
			
			lua_pushnumber( L, otherElement );
			lua_setfield( L, -2, "otherElement" );			
		}
		
		lua_pushnumber( L, fX );
		lua_setfield( L, -2, "x" );
		
		lua_pushnumber( L, fY );
		lua_setfield( L, -2, "y" );
		
		if ( !fOther )
		{
			// Only define object1 and object2 if this is the runtime listener
			fObject1.GetProxy()->PushTable( L );
			lua_setfield( L, -2, "object1" );
		
			fObject2.GetProxy()->PushTable( L );
			lua_setfield( L, -2, "object2" );
			
			lua_pushnumber( L, fFixtureIndex1 );
			lua_setfield( L, -2, "element1" );
		
			lua_pushnumber( L, fFixtureIndex2 );
			lua_setfield( L, -2, "element2" );

		}

		if ( fContactWrapper )
		{
			fContactWrapper->Push();
			lua_setfield( L, -2, "contact" );
		}
	}

	return 1;
}

// Collision events follow a propagation model similar to touch events.
// They propagate to object1, then object2, then to the global Runtime.
// The choice of object1 and object2 is *arbitrary*, determined by Box2D.
// 
// We want to handle the symmetrical and asymmetrical use cases.  In one case, 
// we want both objects to receive the collision event b/c they may react 
// differently to the same collision; here, the Lua listener should not return 
// anything (which implicitly is false). In the other case, we want to deliver 
// the event to only one of the objects (doesn't matter which one gets it),
// so the Lua listener would return true to stop propagation.
void
BaseCollisionEvent::Dispatch( lua_State *L, Runtime& runtime ) const
{
	Rtt_ASSERT( ! fOther ); // fOther is merely a cache for the Push()

	fOther = & fObject2;
	bool handled = fObject1.DispatchEvent( L, * this );

	if ( ! handled )
	{
		fOther = & fObject1;
		handled = fObject2.DispatchEvent( L, * this );
	}

	fOther = NULL; // Always reset fOther

	if ( ! handled )
	{
		// Send to global Runtime
		Super::Dispatch( L, runtime );
	}
}

CollisionEvent::CollisionEvent( DisplayObject& object1, DisplayObject& object2, Real x, Real y, int fixtureIndex1, int fixtureIndex2, const char *phase )
:	Super( object1, object2, x, y, fixtureIndex1, fixtureIndex2 ),
	fPhase( phase )
{
}

const char*
CollisionEvent::Name() const
{
	static const char kName[] = "collision";
	return kName;
}

int
CollisionEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushstring( L, fPhase );
		lua_setfield( L, -2, kPhaseKey );
	}

	return 1;
}

PreCollisionEvent::PreCollisionEvent( DisplayObject& object1, DisplayObject& object2, Real x, Real y, int fixtureIndex1, int fixtureIndex2 )
:	Super( object1, object2, x, y, fixtureIndex1, fixtureIndex2 )
{
}

const char*
PreCollisionEvent::Name() const
{
	static const char kName[] = "preCollision";
	return kName;
}

PostCollisionEvent::PostCollisionEvent( DisplayObject& object1, DisplayObject& object2, Real x, Real y, int fixtureIndex1, int fixtureIndex2, Real normalImpulse, Real tangentImpulse )
:	Super( object1, object2, x, y, fixtureIndex1, fixtureIndex2 ),
	fNormalImpulse( normalImpulse ),
	fTangentImpulse( tangentImpulse )
{
}

const char*
PostCollisionEvent::Name() const
{
	static const char kName[] = "postCollision";
	return kName;
}

int
PostCollisionEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushnumber( L, fNormalImpulse );
		lua_setfield( L, -2, "force" );
		
		lua_pushnumber( L, fTangentImpulse );
		lua_setfield( L, -2, "friction" );
	}

	return 1;
}

static void _PushCommonParticleCollisionEvent( lua_State *L,
												Runtime &runtime,
												const char *phase,
												DisplayObject *object,
												int fixtureIndex,
												ParticleSystemObject *ps,
												b2Vec2 &positionInMeters,
												b2Vec2 *optionalNormal,
												b2ParticleColor &color )
{
	lua_pushstring( L, phase );
	lua_setfield( L, -2, MEvent::kPhaseKey );

	// The object hit.
	{
		// DisplayObject.
		if( object )
		{
			object->GetProxy()->PushTable( L );
		}
		else
		{
			lua_pushnil( L );
		}
		lua_setfield( L, -2, "object" );

		// Fixture index.
		lua_pushnumber( L, fixtureIndex );
		lua_setfield( L, -2, "element" );

		// ParticleSystemObject.
		if( ps )
		{
			ps->GetProxy()->PushTable( L );
		}
		else
		{
			lua_pushnil( L );
		}
		lua_setfield( L, -2, "particleSystem" );
	}

	// The particle hit.
	{
		float pixels_per_meter = runtime.GetPhysicsWorld().GetPixelsPerMeter();

		b2Vec2 positionInPixels( positionInMeters );
		positionInPixels *= pixels_per_meter;

		// Position.
		{
			lua_pushnumber( L, positionInPixels.x );
			lua_setfield( L, -2, "x" );

			lua_pushnumber( L, positionInPixels.y );
			lua_setfield( L, -2, "y" );
		}

		// Normal.
		if( optionalNormal )
		{
			lua_pushnumber( L, optionalNormal->x );
			lua_setfield( L, -2, "normalX" );

			lua_pushnumber( L, optionalNormal->y );
			lua_setfield( L, -2, "normalY" );
		}

		// Color.
		{
			lua_pushnumber( L, ( (float)color.r / 255.0f ) );
			lua_setfield( L, -2, "r" );

			lua_pushnumber( L, ( (float)color.g / 255.0f ) );
			lua_setfield( L, -2, "g" );

			lua_pushnumber( L, ( (float)color.b / 255.0f ) );
			lua_setfield( L, -2, "b" );

			lua_pushnumber( L, ( (float)color.a / 255.0f ) );
			lua_setfield( L, -2, "a" );
		}
	}
}

BeginParticleCollisionEvent::BeginParticleCollisionEvent( Runtime &runtime,
															b2ParticleSystem *particleSystem,
															b2ParticleBodyContact *particleBodyContact )
: Super()
, fRuntime( runtime )
, fParticleSystem( particleSystem )
, fParticleBodyContact( particleBodyContact )
{
}

const char *BeginParticleCollisionEvent::Name() const
{
	return "particleCollision";
}

int BeginParticleCollisionEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		_PushCommonParticleCollisionEvent( L,
											fRuntime,
											"began",
											static_cast< DisplayObject * >( fParticleBodyContact->body->GetUserData() ),
											(int)(intptr_t)fParticleBodyContact->fixture->GetUserData(),
											static_cast< ParticleSystemObject * >( fParticleSystem->GetUserDataBuffer()[ fParticleBodyContact->index ] ),
											fParticleSystem->GetPositionBuffer()[ fParticleBodyContact->index ],
											&fParticleBodyContact->normal,
											fParticleSystem->GetColorBuffer()[ fParticleBodyContact->index ] );
	}

	return 1;
}

static void _DispatchCommonParticleCollisionEvent( lua_State *L,
													Runtime& runtime,
													const VirtualEvent *ve,
													DisplayObject *display_object,
													ParticleSystemObject *pso )
{
	bool handled;

	handled = display_object->DispatchEvent( L, *ve );
	if( ! handled )
	{
		handled = pso->DispatchEvent( L, *ve );
	}

	if( ! handled )
	{
		// Send to global Runtime.
		ve->VirtualEvent::Dispatch( L, runtime );
	}
}

void BeginParticleCollisionEvent::Dispatch( lua_State *L,
											Runtime& runtime ) const
{
	_DispatchCommonParticleCollisionEvent( L,
											runtime,
											static_cast< const VirtualEvent * >( this ),
											static_cast< DisplayObject * >( fParticleBodyContact->body->GetUserData() ),
											static_cast< ParticleSystemObject * >( fParticleSystem->GetUserDataBuffer()[ fParticleBodyContact->index ] ) );
}

EndParticleCollisionEvent::EndParticleCollisionEvent( Runtime &runtime,
														b2Fixture *fixture,
														b2ParticleSystem *particleSystem,
														int particleIndex )
: Super()
, fRuntime( runtime )
, fFixture( fixture )
, fParticleSystem( particleSystem )
, fParticleIndex( particleIndex )
{
}

const char *EndParticleCollisionEvent::Name() const
{
	return "particleCollision";
}

int EndParticleCollisionEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		_PushCommonParticleCollisionEvent( L,
											fRuntime,
											"ended",
											static_cast< DisplayObject* >( fFixture->GetBody()->GetUserData() ),
											(int)(intptr_t)fFixture->GetUserData(),
											static_cast< ParticleSystemObject * >( fParticleSystem->GetUserDataBuffer()[ fParticleIndex ] ),
											fParticleSystem->GetPositionBuffer()[ fParticleIndex ],
											NULL,
											fParticleSystem->GetColorBuffer()[ fParticleIndex ] );
	}

	return 1;
}

void EndParticleCollisionEvent::Dispatch( lua_State *L,
											Runtime& runtime ) const
{
	_DispatchCommonParticleCollisionEvent( L,
											runtime,
											static_cast< const VirtualEvent * >( this ),
											static_cast< DisplayObject* >( fFixture->GetBody()->GetUserData() ),
											static_cast< ParticleSystemObject * >( fParticleSystem->GetUserDataBuffer()[ fParticleIndex ] ) );
}

#endif // Rtt_PHYSICS

// ----------------------------------------------------------------------------

InputDeviceStatusEvent::InputDeviceStatusEvent(
	PlatformInputDevice *device, bool hasConnectionStateChanged, bool wasReconfigured)
:	fDevice( device ),
	fHasConnectionStateChanged( hasConnectionStateChanged ),
	fWasReconfigured( wasReconfigured )
{
}

const char*
InputDeviceStatusEvent::Name() const
{
	static const char kName[] = "inputDeviceStatus";
	return kName;
}

int
InputDeviceStatusEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		if (fDevice)
		{
			fDevice->PushTo( L );
			lua_setfield( L, -2, "device" );
		}

		lua_pushboolean( L, fHasConnectionStateChanged );
		lua_setfield( L, -2, "connectionStateChanged" );

		lua_pushboolean( L, fWasReconfigured );
		lua_setfield( L, -2, "reconfigured" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char*
KeyEvent::StringForPhase( Phase type )
{
	const char* result = NULL;
	static const char kUnknownString[] = "unknown";
	static const char kDownString[] = "down";
	static const char kUpString[] = "up";

	switch( type )
	{
		case kDown:
			result = kDownString;
			break;
		case kUp:
			result = kUpString;
			break;
		default:
			result = kUnknownString;
			break;
	}

	return result;
}

KeyEvent::KeyEvent(
	PlatformInputDevice *device, Phase phase, const char *keyName, S32 nativeKeyCode,
	bool isShiftDown, bool isAltDown, bool isCtrlDown, bool isCommandDown)
:	fDevice( device ),
	fPhase( phase ),
	fKeyName( keyName ),
	fNativeKeyCode( nativeKeyCode ),
	fIsShiftDown( isShiftDown ),
	fIsAltDown( isAltDown ),
	fIsCtrlDown( isCtrlDown ),
    fIsCommandDown( isCommandDown )
{
}

const char*
KeyEvent::Name() const
{
	static const char kName[] = "key";
	return kName;
}

int
KeyEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		if (fDevice)
		{
			fDevice->PushTo( L );
			lua_setfield( L, -2, "device" );
		}

		if (fDevice)
		{
			lua_pushstring( L, fDevice->GetDescriptor().GetInvariantName() );
			lua_pushstring( L, ": " );
			lua_pushstring( L, fKeyName );
			lua_concat( L, 3 );
		}
		else
		{
			lua_pushstring( L, fKeyName );
		}
		lua_setfield( L, -2, "descriptor" );

		const char* value = StringForPhase( (Phase)fPhase );
		lua_pushstring( L, value );
		lua_setfield( L, -2, kPhaseKey );

		lua_pushstring( L, fKeyName );
		lua_setfield( L, -2, "keyName" );

		lua_pushinteger( L, fNativeKeyCode );
		lua_setfield( L, -2, "nativeKeyCode" );

		lua_pushboolean( L, fIsShiftDown );
		lua_setfield( L, -2, "isShiftDown" );

		lua_pushboolean( L, fIsAltDown );
		lua_setfield( L, -2, "isAltDown" );
        
		lua_pushboolean( L, fIsCtrlDown );
		lua_setfield( L, -2, "isCtrlDown" );
        
		lua_pushboolean( L, fIsCommandDown );
		lua_setfield( L, -2, "isCommandDown" );
	}

	return 1;
}

void
KeyEvent::Dispatch( lua_State *L, Runtime& ) const
{
	// Invoke Lua code: "Runtime:dispatchEvent( eventKey )"
	int nargs = PrepareDispatch( L );
	LuaContext::DoCall( L, nargs, 1 );

	fResult = lua_toboolean( L, -1 ); // fetch result 
	lua_pop( L, 1 ); // pop result off stack
}

// ----------------------------------------------------------------------------

CharacterEvent::CharacterEvent(PlatformInputDevice *device, const char *character)
:	fDevice( device ),
fCharacter( character )
{
}

const char*
CharacterEvent::Name() const
{
	static const char kName[] = "character";
	return kName;
}

int
CharacterEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		if (fDevice)
		{
			fDevice->PushTo( L );
			lua_setfield( L, -2, "device" );
		}
		
		lua_pushstring( L, fCharacter );
		lua_setfield( L, -2, "character" );
	}
	
	return 1;
}

void
CharacterEvent::Dispatch( lua_State *L, Runtime& ) const
{
	// Invoke Lua code: "Runtime:dispatchEvent( eventKey )"
	int nargs = PrepareDispatch( L );
	LuaContext::DoCall( L, nargs, 1 );
	
	fResult = lua_toboolean( L, -1 ); // fetch result
	lua_pop( L, 1 ); // pop result off stack
}

// ----------------------------------------------------------------------------

AxisEvent::AxisEvent(PlatformInputDevice *devicePointer, PlatformInputAxis *axisPointer, Rtt_Real rawValue)
:	fDevicePointer( devicePointer ),
	fAxisPointer( axisPointer ),
	fRawValue( rawValue )
{
}

const char*
AxisEvent::Name() const
{
	static const char kName[] = "axis";
	return kName;
}

int
AxisEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		// Push the device userdata to the Lua event table.
		if (fDevicePointer)
		{
			fDevicePointer->PushTo( L );
			lua_setfield( L, -2, "device" );
		}

		// Push the axis information table to the Lua event table.
		if (fAxisPointer)
		{
			fAxisPointer->PushTo( L );
			lua_setfield( L, -2, "axis" );
		}

		// Push the raw axis value to the Lua event table.
		lua_pushnumber( L, Rtt_RealToFloat( fRawValue ) );
		lua_setfield( L, -2, "rawValue" );

		// Push a normalized axis value to the Lua event table.
		if (fAxisPointer)
		{
			Rtt_Real normalizedValue = fAxisPointer->GetNormalizedValue(fRawValue);
			lua_pushnumber( L, Rtt_RealToFloat( normalizedValue ) );
			lua_setfield( L, -2, "normalizedValue" );
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char ColorSampleEvent::kName[] = "colorSample";

ColorSampleEvent::ColorSampleEvent( float x, float y, RGBA &color )
: fX( x )
, fY( y )
, fColor( color )
{
}

const char*
ColorSampleEvent::Name() const
{
	return Self::kName;
}

int
ColorSampleEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushnumber( L, fX );
		lua_setfield( L, -2, "x" );

		lua_pushnumber( L, fY );
		lua_setfield( L, -2, "y" );

		lua_pushnumber( L, ( (float)fColor.r / 255.0f ) );
		lua_setfield( L, -2, "r" );

		lua_pushnumber( L, ( (float)fColor.g / 255.0f ) );
		lua_setfield( L, -2, "g" );

		lua_pushnumber( L, ( (float)fColor.b / 255.0f ) );
		lua_setfield( L, -2, "b" );

		lua_pushnumber( L, ( (float)fColor.a / 255.0f ) );
		lua_setfield( L, -2, "a" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

HitEvent::HitEvent( Real xScreen, Real yScreen )
:	fXContent( xScreen ),
	fYContent( yScreen ),
	fXScreen( xScreen ),
	fYScreen( yScreen ),
	fTime( -1. ),
	fId( NULL )
{
}

const char*
HitEvent::Name() const
{
	static const char kName[] = "touch";
	return kName;
}

int
HitEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		const char kXKey[] = "x";
		const char kYKey[] = "y";

		lua_pushnumber( L, Rtt_RealToFloat( fXContent ) );
		lua_setfield( L, -2, kXKey );
		lua_pushnumber( L, Rtt_RealToFloat( fYContent ) );
		lua_setfield( L, -2, kYKey );

		if ( fTime < 0. )
		{
			fTime = LuaContext::GetRuntime( L )->GetElapsedMS();
		}

		lua_pushnumber( L, fTime );
		lua_setfield( L, -2, "time" );

//		fHitTarget->GetProxy( L )->PushTable( L );
//		lua_setfield( L, -2, "target" );
	}

	return 1;
}

void
HitEvent::InvalidateTime()
{
	fTime = -1.;
}

enum HitTestProperties
{
	kHitTestHandled = 0x1,
	kHitTestDidHit = 0x2
};

static bool
TestMask( Rtt_Allocator *allocator, DisplayObject& child, const Matrix& srcToDstSpace, Real dstX, Real dstY )
{
	Rtt_ASSERT( allocator );

	bool result = false;

	const BitmapMask *mask = child.GetMask();
	Rtt_ASSERT( NULL != mask );

	// Map dstX, dstY to src (mask's, not child's) coordinates
  	Matrix maskToDstSpace( srcToDstSpace );
  	maskToDstSpace.Concat( mask->GetTransform().GetMatrix( NULL ) );
  	Matrix inverse;
  	Matrix::Invert( maskToDstSpace, inverse );

  	Vertex2 p = { dstX, dstY };
  	inverse.Apply( p );

  	PlatformBitmap *bitmap = mask->GetPaint()->GetBitmap();
	if ( Rtt_VERIFY( bitmap ) )
	{
		Real w = bitmap->Width();
		Real h = bitmap->Height();
		
		Real halfW = Rtt_RealDiv2( w );
		Real halfH = Rtt_RealDiv2( h );

		// Map to coordinates where top-left is origin, instead of center.
		Real x = p.x + halfW;
		Real y = p.y + halfH;

		// Useful for debugging
		#if 0
			Rtt_TRACE( ( "TestMask: touch(%d,%d) half(%d,%d) mask(%d,%d)\n",
				Rtt_RealToInt( p.x ), Rtt_RealToInt( p.y ),
				Rtt_RealToInt( halfW ), Rtt_RealToInt( halfH ), 
				Rtt_RealToInt( x ), Rtt_RealToInt( y ) ) );
		#endif

		result = mask->HitTest( allocator, Rtt_RealToInt( x ), Rtt_RealToInt( y ) );
	}

	return result;
}

void
HitEvent::Test( HitTestObject& hitParent, const Matrix& srcToDstSpace ) const
{
	Rtt_ASSERT( hitParent.Target().AsGroupObject() );

	// We need to create a snapshot of the sub-tree of all possible objects 
	// that *might* get dispatched an event.  The model is that the event is 
	// relevant to all objects in the display hierarchy that were present
	// at the time of the event.  Because listeners can remove old objects and
	// insert new ones, this snapshot allows us to dispatch events correctly.
	Matrix xform( srcToDstSpace );
	GroupObject& object = static_cast< GroupObject& >( hitParent.Target() );
	xform.Concat( object.GetMatrix() ); // Object's transform gets applied first

	const StageObject *stage = object.GetStage(); Rtt_ASSERT( stage );

	const Display& display = stage->GetDisplay();
	Rtt_Allocator *allocator = display.GetRuntime().GetAllocator();

	Real x = fXContent;
	Real y = fYContent;

	for ( S32 i = 0, iMax = object.NumChildren(); i < iMax; i++ )
	{
		DisplayObject& child = object.ChildAt( i );

		// Only add visible/hitTestable objects
		// and in the multitouch case, do not have per object focus id set
		// since we dispatch focused events outside of hit testing.
		if ( child.ShouldHitTest() && ! child.GetFocusId() && ( !child.SkipsHitTest() && child.CanHitTest()) )
		{
			GroupObject* childAsGroup = child.AsGroupObject();
			if ( ! childAsGroup )
			{
//				Rtt_ASSERT( child.IsStageBoundsValid() || ! child.CanCull() );

				// Only test if object is actually on-screen
				// Test bounding box before doing more expensive testing
				if ( ! child.IsOffScreen() && child.StageBounds().HitTest( fXContent, fYContent ) )
				{
					Rtt_ASSERT( child.IsStageBoundsValid() );
					child.Prepare( display );

					// TODO: Should we only do SetForceDraw() if the object is hidden?
					// Ensure Draw() is not a no-op for hidden objects
					// as defined by DisplayObject::IsNotHidden()
					bool oldValue = child.IsForceDraw();
					child.SetForceDraw( true );

					bool didHit = child.HitTest( x, y );

					child.SetForceDraw( oldValue );

					// Only do deeper testing if a mask exists and the "isHitTestMasked" property is true
					if ( didHit && child.IsHitTestMasked() && child.GetMask() )
					{
						Matrix childToDst( xform );
						childToDst.Concat( child.GetMatrix() );
						didHit = TestMask( allocator, child, childToDst, x, y );
					}

					if ( didHit )
					{
						// Only if we hit, do we add child to the snapshot
						HitTestObject* hitChild = Rtt_NEW( object.Allocator(), HitTestObject( child, & hitParent ) );
						hitParent.Prepend( hitChild );
					}
				}
			}
			else
			{
				// By default, we hit test children, but if the group has hit test masking on,
				// then we hit test the group's clipped bounding box before we attempt to
				// hit test the group's children.
				bool hitTestChildren = child.HitTest( x, y );
				if( hitTestChildren && child.IsHitTestMasked() )
				{
					// By default, stage bounds of composite objects are not built.
					child.BuildStageBounds();
					hitTestChildren = child.StageBounds().HitTest( x, y );

					// Only do deeper testing if a mask exists and the "isHitTestMasked" property is true
					if ( hitTestChildren && child.GetMask() )
					{
						Matrix childToDst( xform );
						childToDst.Concat( child.GetMatrix() );

						hitTestChildren = TestMask( allocator, child, childToDst, x, y );
					}
				}

				if ( hitTestChildren )
				{
					HitTestObject* hitGroup = Rtt_NEW( object.Allocator(), HitTestObject( child, & hitParent ) );

					// Recursively call on children
					Test( * hitGroup, xform );
					if ( hitGroup->NumChildren() > 0 )
					{
						// Only groups that contain children that were hit are added to the snapshot
						hitParent.Prepend( hitGroup );
					}
					else
					{
						Rtt_DELETE( hitGroup );
					}
				}
			}
		}
	}
}

bool
HitEvent::DispatchFocused( lua_State *L, Runtime& runtime, StageObject& stage, DisplayObject *focus ) const
{
	Rtt_ASSERT( focus );

	bool handled = false;

	ScreenToContent( runtime.GetDisplay(), fXScreen, fYScreen, fXContent, fYContent );

	// If we have focus, then dispatch only to that object or its ancestors
	Rtt_ASSERT( focus->GetStage() == & stage );

	// Append focus and its ancestors into a list. We'll send the event
	// through this snapshot of the hierarchy.
	PtrArrayDisplayObject hitList( runtime.Allocator() );
	for ( DisplayObject* object = focus;
		  NULL != object;
		  object = object->GetParent() )
	{
		hitList.Append( object );
		object->SetUsedByHitTest( true );
	}

	DisplayObject::ListenerMask mask = (DisplayObject::ListenerMask)GetListenerMask();

	// Now, dispatch event starting at focus
	for ( int i = 0, iMax = hitList.Length();
		  i < iMax;
		  i++ )
	{
		DisplayObject* object = hitList[i];
		if ( ! handled )
		{
			if ( object->HasListener( mask ) )
			{
				handled = object->DispatchEvent( L, * this );
			}
		}
		object->SetUsedByHitTest( false );
	}

	// Prevent the objects in hitList from being deleted by hitList's dtor
	hitList.Remove( 0, hitList.Length(), false );

	return handled;
}

void
HitEvent::ScreenToContent( const Display& display, Real xScreen, Real yScreen, Real& outXContent, Real& outYContent )
{
	// Scale point: map screen coord to content coord
	outXContent = Rtt_RealMul( xScreen, display.GetSx() );
	outXContent -= display.GetXOriginOffset();
	outYContent = Rtt_RealMul( yScreen, display.GetSy() );
	outYContent -= display.GetYOriginOffset();
}

void
HitEvent::Dispatch( lua_State *L, Runtime& runtime ) const
{
	// TODO: If receiver is called more than once, we need to restore fX, fY
	// or alternatively set some flag so that we know that it's been mapped
	Display& display = runtime.GetDisplay();

	ScreenToContent( display, fXScreen, fYScreen, fXContent, fYContent );

	StageObject& stage = * display.GetStage();
	DisplayObject* focus = stage.GetFocus();
	bool handled = false;
	if ( focus )
	{
		handled = DispatchFocused( L, runtime, stage, focus );
	}
	else
	{
#ifdef Rtt_AUTHORING_SIMULATOR
		// This makes it possible to detect hits on the "screen dressing" overlay in skinned Simulator windows
		// and not have them go through to the app
		Matrix identity;
		HitTestObject overlayRoot( LuaContext::GetRuntime( L )->GetDisplay().GetScene().Overlay(), NULL );
		Test( overlayRoot, identity ); // Generates subtree snapshot
		handled = DispatchEvent( L, overlayRoot ); // Dispatches to that subtree

		if (! handled)
#endif // Rtt_AUTHORING_SIMULATOR
		{
			// Default: no focus, so hit test and dispatch to subtree of all hit objects
			Matrix identity;
			stage.UpdateTransform( identity );
			HitTestObject root( stage, NULL );
			Test( root, identity ); // Generates subtree snapshot
			handled = DispatchEvent( L, root ); // Dispatches to that subtree
		}
	}

	if ( ! handled )
	{
		// Send to global Runtime
		Super::Dispatch( L, runtime );
	}

	// Cleanup: Move objects from the snapshot orphanage to the real orphanage
	// so they can be properly removed at the end of the event dispatch scope.
	GroupObject& hitTestOrphanage = * display.HitTestOrphanage();
	GroupObject& orphanage = * display.Orphanage();
	for ( int i = hitTestOrphanage.NumChildren(); --i >= 0; )
	{
		DisplayObject& child = hitTestOrphanage.ChildAt( i );
		orphanage.Insert( -1, & child, false );
	}
	Rtt_ASSERT( 0 == hitTestOrphanage.NumChildren() );
}

U32
HitEvent::GetListenerMask() const
{
	return DisplayObject::kUnknownListener;
}

bool
HitEvent::DispatchEvent( lua_State *L, HitTestObject& parent ) const
{
	// We want to do post-order tree walk. The walk traverses the tree in a
	// depth-first manner. This ensures that the frontmost, "deepest" child is 
	// dispatched the event first. If not handled (i.e. no listener or the 
	// listener returns false) then the event is sent to a sibling that's behind
	// and then to their ancestor.

	// NOTE: All objects in this tree were successfully hit-tested!

	bool handled = false;
	const HitEvent& e = *this;

	DisplayObject::ListenerMask mask = (DisplayObject::ListenerMask)GetListenerMask();

	// Traverse children in order --- they were added via Prepend, so the
	// sibling ordering is reverse of the display hierarchy's. The visibly-
	// frontmost objects are at the beginning of the child list
	for ( HitTestObject* o = parent.Child(); o && ! handled; o = o->Sibling() )
	{
		DisplayObject& target = o->Target();

		// Either target is not a group, or if it is, then o has at least one child.
		// We should never have added a HitTestObject whose target is a GroupObject
		// unless that HitTestObject had child hit objects.
		Rtt_ASSERT( ! target.AsGroupObject() || o->NumChildren() > 0 );

		if ( o->NumChildren() == 0 )
		{
			if ( target.HasListener( mask ) )
			{
				handled = target.DispatchEvent( L, e );
			}
		}
		else
		{
			// Recurse into o
			handled = DispatchEvent( L, *o );
		}
	}

	// Post-order tree walk, so only dispatch to the parent *after* the children.
	if ( ! handled )
	{
		handled = parent.Target().DispatchEvent( L, e );
	}

	return handled;
}

// ----------------------------------------------------------------------------

const Real TouchEvent::kPressureInvalid = Rtt_REAL_NEG_1;
const Real TouchEvent::kPressureThreshold = Rtt_REAL_0;

const char*
TouchEvent::StringForPhase( Phase phase )
{
	const char* result = NULL;
	static const char kBeganString[] = "began";
	static const char kMovedString[] = "moved";
	static const char kStationaryString[] = "stationary";
	static const char kEndedString[] = "ended";
	static const char kCancelledString[] = "cancelled";

	switch( phase )
	{
		case kBegan:
			result = kBeganString;
			break;
		case kMoved:
			result = kMovedString;
			break;
		case kStationary:
			result = kStationaryString;
			break;
		case kEnded:
			result = kEndedString;
			break;
		case kCancelled:
			result = kCancelledString;
			break;
		default:
			break;
	}

	return result;
}


TouchEvent::Phase
TouchEvent::phaseForType( int touchType )
{
	Rtt::TouchEvent::Phase result;
	
	switch( touchType )
	{
	case 0:
		result = Rtt::TouchEvent::kBegan;
		break;
	case 1:
		result = Rtt::TouchEvent::kMoved;
		break;
	case 2:
		result = Rtt::TouchEvent::kStationary;
		break;
	case 3:
		result = Rtt::TouchEvent::kEnded;
		break;
	case 4:
	default:
		result = Rtt::TouchEvent::kCancelled;
		break;
	}
	
	return result;
}

const char TouchEvent::kName[] = "touch";

TouchEvent::TouchEvent()
:	Super( Rtt_REAL_0, Rtt_REAL_0 ),
	fPhase( kBegan ),
	fProperties( 0 ),
	fXStartScreen( Rtt_REAL_0 ),
	fYStartScreen( Rtt_REAL_0 ),
	fXStartContent( Rtt_REAL_0 ),
	fYStartContent( Rtt_REAL_0 ),
	fPressure( kPressureInvalid ),
	fDeltaX( Rtt_REAL_0 ),
	fDeltaY( Rtt_REAL_0 )
{
}

TouchEvent::TouchEvent( Real x, Real y, Real xStartScreen, Real yStartScreen, Phase phase, Real pressure )
:	Super( x, y ),
	fPhase( phase ),
	fProperties( 0 ),
	fXStartScreen( xStartScreen ),
	fYStartScreen( yStartScreen ),
	fXStartContent( xStartScreen ),
	fYStartContent( yStartScreen ),
	fPressure( pressure ),
	fDeltaX( x - xStartScreen ),
	fDeltaY( y - yStartScreen )
{
}

const char*
TouchEvent::Name() const
{
	return kName;
}

int
TouchEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		const char kXStartKey[] = "xStart";
		const char kYStartKey[] = "yStart";
		const char kPressureKey[] = "pressure";
		const char kXDeltaKey[] = "xDelta";
		const char kYDeltaKey[] = "yDelta";

		lua_pushstring( L, StringForPhase( (Phase)fPhase ) );
		lua_setfield( L, -2, kPhaseKey );

		lua_pushnumber( L, Rtt_RealToFloat( fXStartContent ) );
		lua_setfield( L, -2, kXStartKey );
		lua_pushnumber( L, Rtt_RealToFloat( fYStartContent ) );
		lua_setfield( L, -2, kYStartKey );
		lua_pushinteger( L, Rtt_RealToInt( fDeltaX) );
		lua_setfield( L, -2, kXDeltaKey );
		lua_pushinteger( L, Rtt_RealToInt( fDeltaY ));
		lua_setfield( L, -2, kYDeltaKey );
		
		if ( fPressure >= kPressureThreshold )
		{
			lua_pushnumber( L, Rtt_RealToFloat( fPressure ) );
			lua_setfield( L, -2, kPressureKey );
		}

		if ( GetId() )
		{
			lua_pushlightuserdata( L, const_cast< void * >( GetId() ) );
			lua_setfield( L, -2, "id" );
		}
	}

	return 1;
}

void
TouchEvent::Dispatch( lua_State *L, Runtime& runtime ) const
{
	ScreenToContent( runtime.GetDisplay(), fXStartScreen, fYStartScreen, fXStartContent, fYStartContent );

	Super::Dispatch( L, runtime );
}

bool
TouchEvent::DispatchFocused( lua_State *L, Runtime& runtime, StageObject& stage, DisplayObject *focus ) const
{
	ScreenToContent( runtime.GetDisplay(), fXStartScreen, fYStartScreen, fXStartContent, fYStartContent );

	return Super::DispatchFocused( L, runtime, stage, focus );
}

U32
TouchEvent::GetListenerMask() const
{
	return DisplayObject::kTouchListener;
}

void
TouchEvent::SetProperty( U16 mask, bool value )
{
	const U16 p = fProperties;
	fProperties = ( value ? p | mask : p & ~mask );
}

// ----------------------------------------------------------------------------

MultitouchEvent::MultitouchEvent( TouchEvent *touches, int numTouches )
:	Super(),
	fTouches( touches ),
	fNumTouches( numTouches )
{
	Rtt_ASSERT( numTouches > 0 );
}

const char*
MultitouchEvent::Name() const
{
	return TouchEvent::kName;
}

void
MultitouchEvent::Dispatch( lua_State *L, Runtime& runtime ) const
{
	TouchEvent::Phase phase = fTouches[0].GetPhase(); // all fTouches should be in the same phase
	bool shouldCleanup = ( TouchEvent::kEnded == phase || TouchEvent::kCancelled == phase );

	Display& display = runtime.GetDisplay();
	StageObject& stage = * display.GetStage();

	// Iterate through the touches, dispatching individually
	for ( int i = 0, iMax = fNumTouches; i < iMax; i++ )
	{
		const TouchEvent& e = fTouches[i];
		DisplayObject *object = stage.GetFocus( e.GetId() );
		if ( object )
		{
			// Dispatch focused per-object touch events
			e.DispatchFocused( L, runtime, stage, object );

			// Cleanup: remove per-object focus at the end of the touch (e.g. phase is "ended" or "cancelled")
			if ( shouldCleanup )
			{
				stage.SetFocus( object, NULL );
			}
		}
		else
		{
			// Dispatch touch event normally
			e.Dispatch( L, runtime );
		}
	}
}
	
// ----------------------------------------------------------------------------

const char RelativeTouchEvent::kName[] = "relativeTouch";

RelativeTouchEvent::RelativeTouchEvent( Real x, Real y, Phase phase, U64 tapCount )
:	TouchEvent( x, y, Rtt_REAL_0, Rtt_REAL_0, phase, kPressureInvalid ),
	fNumTaps( tapCount )
{
}

const char*
RelativeTouchEvent::Name() const
{
	return RelativeTouchEvent::kName;
}

int
RelativeTouchEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( TouchEvent::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );
		
		const char kTapCountKey[] = "tapCount";
		
		// On relativeTouch ended events, supply the tap count.
		if ( GetPhase() == TouchEvent::kEnded )
		{
			lua_pushinteger( L, fNumTaps );
			lua_setfield( L, -2, kTapCountKey );
		}
	}
	
	return 1;
}

// TouchEvents try to adjust based on content scaling and screen origins. For relative touch events, we
// don't care about the screen, only the relationship between the initial touch and follow-up movements.
// We also do not want hit testing. So, we bypass ScreenToContent from TouchEvent and content/focus tests
// from HitEvent.
void
RelativeTouchEvent::Dispatch( lua_State *L, Runtime& runtime ) const
{
	VirtualEvent::Dispatch( L, runtime );
}

// ----------------------------------------------------------------------------

const char TapEvent::kName[] = "tap";

TapEvent::TapEvent( Real x, Real y, S32 numTaps )
:	Super( x, y ),
	fNumTaps( numTaps )
{
}

const char*
TapEvent::Name() const
{
	return kName;
}

int
TapEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		const char kNumTapsKey[] = "numTaps";

//		Rtt_TRACE( ( "[TapEvent::Push] numTaps(%d)\n", fNumTaps ) );

		lua_pushinteger( L, fNumTaps );
		lua_setfield( L, -2, kNumTapsKey );
	}

	return 1;
}

U32
TapEvent::GetListenerMask() const
{
	return DisplayObject::kTapListener;
}

// ----------------------------------------------------------------------------

DragEvent::DragEvent( Real x, Real y, Real dstX, Real dstY )
:	Super( dstX, dstY ),
	fDeltaX( dstX - x ),
	fDeltaY( dstY - y )
{
}

const char*
DragEvent::Name() const
{
	static const char kName[] = "drag";

	return kName;
}

int
DragEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		const char kXDeltaKey[] = "xDelta";
		const char kYDeltaKey[] = "yDelta";

//		Rtt_TRACE( ( "[TapEvent::Push] numTaps(%d)\n", fNumTaps ) );

		lua_pushinteger( L, Rtt_RealToInt( fDeltaX ) );
		lua_setfield( L, -2, kXDeltaKey );
		lua_pushinteger( L, Rtt_RealToInt( fDeltaY ) );
		lua_setfield( L, -2, kYDeltaKey );
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char MouseEvent::kName[] = "mouse";

const char*
MouseEvent::StringForMouseEventType( MouseEventType eventType )
{
    const char* result = NULL;
    static const char kGenericString[] = "generic";
    static const char kUpString[] = "up";
    static const char kDownString[] = "down";
    static const char kDragString[] = "drag";
    static const char kMoveString[] = "move";
    static const char kScrollString[] = "scroll";
  
    switch( eventType )
    {
        case kGeneric:
            result = kGenericString;
            break;
        case kUp:
            result = kUpString;
            break;
        case kDown:
            result = kDownString;
            break;
        case kDrag:
            result = kDragString;
            break;
        case kMove:
            result = kMoveString;
            break;
        case kScroll:
            result = kScrollString;
            break;
        default:
            break;
    }
    
    return result;
}

    
MouseEvent::MouseEvent(MouseEventType eventType, Real x, Real y, Real scrollX, Real scrollY, int clickCount,
                       bool isPrimaryButtonDown, bool isSecondaryButtonDown, bool isMiddleButtonDown,
                       bool isShiftDown, bool isAltDown, bool isCtrlDown, bool isCommandDown)
:	fEventType(eventType),
    Super( x, y ),
    fScrollX(scrollX),
    fScrollY(scrollY),
    fClickCount(clickCount),
	fIsPrimaryButtonDown( isPrimaryButtonDown ),
	fIsSecondaryButtonDown( isSecondaryButtonDown ),
	fIsMiddleButtonDown( isMiddleButtonDown ),
    fIsShiftDown( isShiftDown ),
    fIsAltDown( isAltDown ),
    fIsCtrlDown( isCtrlDown ),
    fIsCommandDown( isCommandDown )
{
}

const char*
MouseEvent::Name() const
{
	return kName;
}

int
MouseEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		lua_pushnumber( L, fScrollX );
		lua_setfield( L, -2, "scrollX" );
        
		lua_pushnumber( L, fScrollY );
		lua_setfield( L, -2, "scrollY" );
        
		lua_pushinteger( L, fClickCount );
		lua_setfield( L, -2, "clickCount" );
        
		lua_pushboolean( L, fIsPrimaryButtonDown ? 1 : 0 );
		lua_setfield( L, -2, "isPrimaryButtonDown" );
        
		lua_pushboolean( L, fIsSecondaryButtonDown ? 1 : 0 );
		lua_setfield( L, -2, "isSecondaryButtonDown" );
		
		lua_pushboolean( L, fIsMiddleButtonDown ? 1 : 0 );
		lua_setfield( L, -2, "isMiddleButtonDown" );
        
		lua_pushboolean( L, fIsShiftDown );
		lua_setfield( L, -2, "isShiftDown" );
        
		lua_pushboolean( L, fIsAltDown );
		lua_setfield( L, -2, "isAltDown" );
        
		lua_pushboolean( L, fIsCtrlDown );
		lua_setfield( L, -2, "isCtrlDown" );
        
		lua_pushboolean( L, fIsCommandDown );
		lua_setfield( L, -2, "isCommandDown" );
        
		lua_pushstring( L, StringForMouseEventType(fEventType) );
		lua_setfield( L, -2, "type" );
	}

	return 1;
}

U32
MouseEvent::GetListenerMask() const
{
	return DisplayObject::kMouseListener;
}

// ----------------------------------------------------------------------------
/*
void
VirtualLocalEvent::Dispatch( lua_State *L, Runtime& runtime ) const
{
	// Should never call this version for local events
	Rtt_ASSERT_NOT_REACHED();
}
*/
// ----------------------------------------------------------------------------

const char FBConnectBaseEvent::kName[] = "fbconnect";

const char*
FBConnectBaseEvent::StringForType( Type t )
{
	const char* result = NULL;
	static const char kSessionString[] = "session";
	static const char kRequestString[] = "request";
	static const char kDialogString[] = "dialog";

	switch( t )
	{
		case kSession:
			result = kSessionString;
			break;
		case kRequest:
			result = kRequestString;
			break;
		case kDialog:
			result = kDialogString;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

FBConnectBaseEvent::FBConnectBaseEvent( Type t )
:	fResponse( NULL ),
	fIsError( false ),
	fType( t )
{
}

FBConnectBaseEvent::FBConnectBaseEvent( Type t, const char *response, bool isError )
:	fResponse( response ),
	fIsError( isError ),
	fType( t )
{
}

const char*
FBConnectBaseEvent::Name() const
{
	return Self::kName;
}

int
FBConnectBaseEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		const char *value = StringForType( (Type)fType ); Rtt_ASSERT( value );
		lua_pushstring( L, value );
		lua_setfield( L, -2, kTypeKey );

		lua_pushboolean( L, fIsError );
		lua_setfield( L, -2, kIsErrorKey );

		const char *message = fResponse ? fResponse : "";
		lua_pushstring( L, message );
		lua_setfield( L, -2, kResponseKey );
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char*
FBConnectSessionEvent::StringForPhase( Phase phase )
{
	const char* result = NULL;
	static const char kLoginString[] = "login";
	static const char kLoginFailedString[] = "loginFailed";
	static const char kLoginCancelledString[] = "loginCancelled";
	static const char kLogoutString[] = "logout";

	switch( phase )
	{
		case kLogin:
			result = kLoginString;
			break;
		case kLoginFailed:
			result = kLoginFailedString;
			break;
		case kLoginCancelled:
			result = kLoginCancelledString;
			break;
		case kLogout:
			result = kLogoutString;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

FBConnectSessionEvent::FBConnectSessionEvent( const char *token, time_t tokenExpiration )
:	Super( Super::kSession ),
	fPhase( kLogin ),
	fToken( token ),
	fTokenExpiration( tokenExpiration )
{
}

FBConnectSessionEvent::FBConnectSessionEvent( Phase phase )
:	Super( Super::kSession ),
	fPhase( phase ),
	fToken( NULL ),
	fTokenExpiration( 0 )
{
}

FBConnectSessionEvent::FBConnectSessionEvent( Phase phase, const char errorMsg[] )
:	Super( Super::kSession, errorMsg, true ),
	fPhase( phase ),
	fToken( NULL ),
	fTokenExpiration( 0 )
{
}

int
FBConnectSessionEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		const char *value = StringForPhase( (Phase)fPhase ); Rtt_ASSERT( value );
		lua_pushstring( L, value );
		lua_setfield( L, -2, kPhaseKey );

		if ( fToken )
		{
			Rtt_ASSERT( kLogin == fPhase );
			lua_pushstring( L, fToken );
			lua_setfield( L, -2, "token" );

			lua_pushnumber( L, fTokenExpiration );
			lua_setfield( L, -2, "expiration" );
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

FBConnectRequestEvent::FBConnectRequestEvent( const char *response, bool isError )
:	Super( Super::kRequest, response, isError ),
	fDidComplete( false )
{
}

FBConnectRequestEvent::FBConnectRequestEvent( const char *response, bool isError, bool didComplete )
:	Super( Super::kDialog, response, isError ),
	fDidComplete( didComplete )
{
}

int
FBConnectRequestEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		lua_pushboolean( L, fDidComplete );
		lua_setfield( L, -2, "didComplete" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char CompletionEvent::kName[] = "completion";

const char*
CompletionEvent::Name() const
{
	return Self::kName;
}

// ----------------------------------------------------------------------------
	
const char GameNetworkEvent::kName[] = "gameNetwork";

const char*
GameNetworkEvent::Name() const
{
	return Self::kName;
}

// ----------------------------------------------------------------------------

const char UrlRequestEvent::kName[] = "urlRequest";

const char*
UrlRequestEvent::StringForType( Type type )
{
	const char *result = NULL;

	static const char kLinkString[] = "link";
	static const char kFormString[] = "form";
	static const char kHistoryString[] = "history";
	static const char kReloadString[] = "reload";
	static const char kFormResubmittedString[] = "formResubmit";
	static const char kOtherString[] = "other";
	static const char kLoadedString[] = "loaded";

	switch ( type )
	{
		case kLink:
			result = kLinkString;
			break;
		case kForm:
			result = kFormString;
			break;
		case kHistory:
			result = kHistoryString;
			break;
		case kReload:
			result = kReloadString;
			break;
		case kFormResubmitted:
			result = kFormResubmittedString;
			break;
		case kOther:
			result = kOtherString;
			break;
		case kLoaded:
			result = kLoadedString;
			break;
		default:
			break;
	}

	return result;
}

UrlRequestEvent::UrlRequestEvent( const char *url, Type type )
:	fUrl( url ),
	fType( type ),
	fErrorMsg( NULL ),
	fErrorCode( 0 )
{
	Rtt_ASSERT( url );
}

UrlRequestEvent::UrlRequestEvent( const char *url, const char *errorMsg, S32 errorCode )
:	fUrl( url ),
	fType( kUnknown ),
	fErrorMsg( errorMsg ),
	fErrorCode( errorCode )
{
	Rtt_ASSERT( url );
}

const char*
UrlRequestEvent::Name() const
{
	return Self::kName;
}

int
UrlRequestEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushstring( L, fUrl );
		lua_setfield( L, -2, "url"  );

		const char *typeValue = StringForType( fType );
		if ( typeValue )
		{
			lua_pushstring( L, typeValue );
			lua_setfield( L, -2, kTypeKey  );
		}

		if ( fErrorMsg )
		{
			lua_pushstring( L, fErrorMsg );
			lua_setfield( L, -2, kErrorMessageKey );
			lua_pushinteger( L, fErrorCode );
			lua_setfield( L, -2, kErrorCodeKey );
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char UserInputEvent::kName[] = "userInput";

const char*
UserInputEvent::StringForPhase( Phase phase )
{
	const char* result = NULL;
	static const char kBeganString[] = "began";
	static const char kEditingString[] = "editing";
	static const char kSubmittedString[] = "submitted";
	static const char kEndedString[] = "ended";

	switch( phase )
	{
		case kBegan:
			result = kBeganString;
			break;
		case kEditing:
			result = kEditingString;
			break;
		case kSubmitted:
			result = kSubmittedString;
			break;
		case kEnded:
			result = kEndedString;
			break;
		default:
			break;
	}

	return result;
}

UserInputEvent::UserInputEvent( Phase phase )
:	fPhase( phase ),
	fStartPos( -1 ),
	fNumDeleted( -1 ),
	fNewChars( NULL ),
	fOldString( NULL ),
	fNewString( NULL )
{
}

UserInputEvent::UserInputEvent(
	int startpos,
	int numdeleted,
	const char *newchars,
	const char *oldstring,
	const char *newstring )
:	fPhase( kEditing ),
	fStartPos( startpos ),
	fNumDeleted( numdeleted ),
	fNewChars( newchars ),
	fOldString( oldstring ),
	fNewString( newstring )
{
}

const char*
UserInputEvent::Name() const
{
	return Self::kName;
}

int
UserInputEvent::Push( lua_State *L ) const
{
	if ( Super::Push( L ) )
	{
		lua_pushstring( L, StringForPhase( fPhase ) );
		lua_setfield( L, -2, kPhaseKey );
		
		if ( kEditing == fPhase )
		{
			lua_pushinteger( L, fStartPos );
			lua_setfield( L, -2, "startPosition" );

			lua_pushinteger( L, fNumDeleted );
			lua_setfield( L, -2, "numDeleted" );
			
			lua_pushstring( L, fNewChars );
			lua_setfield( L, -2, "newCharacters" );
			
			lua_pushstring( L, fOldString );
			lua_setfield( L, -2, "oldText" );
			
			lua_pushstring( L, fNewString );
			lua_setfield( L, -2, "text" );
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char*
SpriteEvent::StringForPhase( Phase phase )
{
	static const char kBeganString[] = "began";
	static const char kNextString[] = "next";
	static const char kBounceString[] = "bounce";
	static const char kLoopString[] = "loop";
	static const char kEndedString[] = "ended";

	const char *result = NULL;

	switch( phase )
	{
		case kBegan:
			result = kBeganString;
			break;
		case kNext:
			result = kNextString;
			break;
		case kBounce:
			result = kBounceString;
			break;
		case kLoop:
			result = kLoopString;
			break;
		case kEnded:
			result = kEndedString;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

const char
SpriteEvent::kName[] = "sprite";

SpriteEvent::SpriteEvent( const SpriteObject& target, Phase phase )
:	fTarget( target ),
	fPhase( phase )
{
}

const char*
SpriteEvent::Name() const
{
	return kName;
}

int
SpriteEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		lua_pushstring( L, StringForPhase( (Phase)fPhase ) );
		lua_setfield( L, -2, kPhaseKey );

		fTarget.GetProxy()->PushTable( L );
		lua_setfield( L, -2, "target" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char NetworkRequestEvent::kName[] = "networkRequest";

const char*
NetworkRequestEvent::Name() const
{
	return Self::kName;
}

NetworkRequestEvent::NetworkRequestEvent( const char *url, const char *responseString, int statusCode, const char *rawHeader )
:	fUrl( url ),
	fResponseString( responseString ),
	fStatusCode( statusCode ),
	fRawHeader( rawHeader ),
	fIsError( statusCode <= 0 )
{
}

NetworkRequestEvent::NetworkRequestEvent( const char *url, const char *errorMessage )
:	fUrl( url ),
	fResponseString( errorMessage ),
	fStatusCode( -1 ),
	fRawHeader( NULL ),
	fIsError( true )
{
}

int
NetworkRequestEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		lua_pushstring( L, fUrl );
		lua_setfield( L, -2, "url"  );

		lua_pushstring( L, fResponseString ? fResponseString : "" );
		lua_setfield( L, -2, kResponseKey );

		if ( fStatusCode > 0 )
		{
			lua_pushinteger( L, fStatusCode );
			lua_setfield( L, -2, "status"  );
		}

		if ( fRawHeader )
		{
			lua_pushstring( L, fRawHeader );
			lua_setfield( L, -2, "header" );
		}
		
		lua_pushboolean( L, fIsError );
		lua_setfield( L, -2, kIsErrorKey );
	}

	return 1;
}

void
NetworkRequestEvent::DispatchEvent( const LuaResource& resource )
{
	int nargs = resource.PushListenerAndEvent( * this );
	if ( nargs > 0 )
	{
		lua_State *L = resource.L(); Rtt_ASSERT( L );

		// Runtime can be NULL
		Runtime *runtime = LuaContext::GetRuntime( L );
		const MPlatform& platform = LuaContext::GetPlatform( L );

		if ( runtime ) { platform.BeginRuntime( * runtime ); }

		(void) Rtt_VERIFY( 0 == LuaContext::DoCall( L, nargs, 0 ) );

		if ( runtime ) { platform.EndRuntime( * runtime ); }
	}
}

// ----------------------------------------------------------------------------

const char AdsRequestEvent::kName[] = "adsRequest";

AdsRequestEvent::AdsRequestEvent( const char *provider, bool isError )
:	fProvider( provider ),
	fIsError( isError )
{
}

const char*
AdsRequestEvent::Name() const
{
	return Self::kName;
}

int
AdsRequestEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushstring( L, fProvider );
		lua_setfield( L, -2, kProviderKey );

		lua_pushboolean( L, fIsError );
		lua_setfield( L, -2, kIsErrorKey );
	}
	
	return 1;
}

// ----------------------------------------------------------------------------

const char PopupClosedEvent::kName[] = "popupClosed";

PopupClosedEvent::PopupClosedEvent( const char *popupName, bool wasCanceled )
:	fPopupName( popupName ),
	fWasCanceled( wasCanceled )
{
}

const char*
PopupClosedEvent::Name() const
{
	return Self::kName;
}

int
PopupClosedEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		lua_pushstring( L, fPopupName );
		lua_setfield( L, -2, kTypeKey );
		
		lua_pushboolean( L, fWasCanceled ? 1 : 0 );
		lua_setfield( L, -2, "cancelled" );
	}
	
	return 1;
}

// ----------------------------------------------------------------------------

const char VideoEvent::kName[] = "video";

const char*
VideoEvent::StringForPhase( Phase type )
{
	const char* result = NULL;
	static const char kReadyString[] = "ready";
	static const char kEndedString[] = "ended";
	static const char kFailedString[] = "failed";

	switch( type )
	{
		case kReady:
			result = kReadyString;
			break;
		case kEnded:
			result = kEndedString;
			break;
		case kFailed:
			result = kFailedString;
			break;
		default:
			break;
	}

	return result;
}

VideoEvent::VideoEvent( Phase phase )
:	fPhase( phase )
{
}

const char*
VideoEvent::Name() const
{
	return Self::kName;
}

int
VideoEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		const char *phase = StringForPhase( fPhase );
		if ( Rtt_VERIFY( phase ) )
		{
			lua_pushstring( L, phase );
			lua_setfield( L, -2, kPhaseKey );
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

const char FinalizeEvent::kName[] = "finalize";

FinalizeEvent::FinalizeEvent()
{
}

const char*
FinalizeEvent::Name() const
{
	return Self::kName;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

