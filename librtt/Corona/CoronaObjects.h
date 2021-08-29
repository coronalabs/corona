//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------

#ifndef _CoronaObjects_H__
#define _CoronaObjects_H__

#include "CoronaMacros.h"
#include "CoronaPublicTypes.h"


#ifdef __cplusplus
extern "C" {
#endif
    typedef struct lua_State lua_State;
#ifdef __cplusplus
}
#endif


// C API
// ----------------------------------------------------------------------------

/**
 Enumeration of Corona object methods that may be augmented. @see CoronaObjectParamsHeader
 
 Although the values are summarized, most of this assumes some familiarity with the Solar2D source, in
 particular from `librtt/Display`:

 `Rtt_MDrawable.h`, `Rtt_DisplayObject.h`, `Rtt_GroupObject.h`
*/

// TODO: backend, e.g. texture / FBO stuff (Rtt_Snapshot.h in list above)

typedef enum {
    kAugmentedMethod_None,

    //
    // The following few methods seem to come up most often in test plugins,
    // so put them first, favoring them in sort operations.
    //
    
    /**
      (TODO)
    */
    kAugmentedMethod_Draw, // uses "own" type, i.e. `CoronaObjectDrawParams`

    /**
      (TODO)
    */
    kAugmentedMethod_CanCull, // BooleanResult, i.e. uses `CoronaObjectBooleanResultParams` type

    /**
      (TODO)
    */
    kAugmentedMethod_CanHitTest, // BooleanResult

    /**
      (TODO)
    */
    kAugmentedMethod_OnMessage,

    /**
      (TODO)
    */
    kAugmentedMethod_SetValue,

    /**
      (TODO)
    */
    kAugmentedMethod_Value,

    /**
      (TODO)
    */
    kAugmentedMethod_OnFinalize, // Lifetime

    //
    // The following operations have no obvious use case frequencies.
    //

    /**
      (TODO)
    */
    kAugmentedMethod_AddedToParent,

    /**
      (TODO)
    */
    kAugmentedMethod_DidMoveOffscreen, // Basic

    /**
      (TODO)
    */
    kAugmentedMethod_DidUpdateTransform, // Matrix

    /**
      (TODO)
    */
    kAugmentedMethod_GetSelfBounds, // RectResult

    /**
      (TODO)
    */
    kAugmentedMethod_GetSelfBoundsForAnchor, // RectResult

    /**
      (TODO)
    */
    kAugmentedMethod_HitTest, // BooleanResultPoint

    /**
      (TODO)
    */
    kAugmentedMethod_OnCreate, // Lifetime

    /**
      (TODO)
    */
    kAugmentedMethod_Prepare, // Basic

    /**
      (TODO)
    */
    kAugmentedMethod_RemovedFromParent,

    /**
      (TODO)
    */
    kAugmentedMethod_Rotate,

    /**
      (TODO)
    */
    kAugmentedMethod_Scale,

    /**
      (TODO)
    */
    kAugmentedMethod_Translate,

    /**
      (TODO)
    */
    kAugmentedMethod_UpdateTransform, // BooleanResultMatrix

    /**
      (TODO)
    */
    kAugmentedMethod_WillMoveOnscreen, // Basic

    //
    // The following operations are relevant to groups.
    //

    /**
      (TODO)
    */
    kAugmentedMethod_DidInsert,

    /**
      (TODO)
    */
    kAugmentedMethod_DidRemove, // Basic

    /**
      The number of valid augmentable methods.
    */
    kAugmentedMethod_Count
} CoronaObjectAugmentedMethod;

/**
  (TODO)
*/
typedef struct CoronaObjectParamsHeader {
    /**
      Link to the next header in the chain, or `NULL` if this is the last one.
     */
    struct CoronaObjectParamsHeader * next;
    
    /**
      The appropriate member of `CoronaObjectAugmentedMethod` that identifies the payload that follows.
     */
    unsigned short method; // n.b. quite generous: all methods fit easily within a byte)
} CoronaObjectParamsHeader;

// ----------------------------------------------------------------------------

#define CORONA_OBJECTS_BOOKENDED_PARAMS(NAME, ...)                \
    typedef void (*CoronaObject##NAME##Bookend) (__VA_ARGS__);    \
                                                                  \
    typedef struct CoronaObject##NAME##Params {     \
        CoronaObjectParamsHeader header;            \
        unsigned short ignoreOriginal;              \
        CoronaObject##NAME##Bookend before, after;  \
    } CoronaObject##NAME##Params

/**
  (TODO)
*/

// ----------------------------------------------------------------------------

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Basic, const CoronaDisplayObject * object, void * userData ); // CoronaObjectBasicBookend, CoronaObjectBasicParams...

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( AddedToParent, const CoronaDisplayObject * object, void * userData, lua_State * L, CoronaGroupObject * groupObject ); // ...and so on

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Matrix, const CoronaDisplayObject * object, void * userData, float matrix[6] );

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Draw, const CoronaDisplayObject * object, void * userData, CoronaRenderer * renderer );

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( RectResult, const CoronaDisplayObject * object, void * userData, float * xMin, float * yMin, float * xMax, float * yMax );

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( RemovedFromParent, const CoronaDisplayObject * object, void * userData, lua_State * L, CoronaGroupObject * groupObject );

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Rotate, const CoronaDisplayObject * object, void * userData, float delta );

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Scale, const CoronaDisplayObject * object, void * userData, float sx, float sy, int isNew );

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Translate, const CoronaDisplayObject * object, void * userData, float x, float y );

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( DidInsert, CoronaGroupObject * groupObject, void * userData, int childParentChanged );

/**
  (TODO)
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( GroupBasic, const CoronaGroupObject * groupObject, void * userData );

// ----------------------------------------------------------------------------

#define CORONA_OBJECTS_EARLY_OUTABLE_BOOKENDED_PARAMS(NAME, ...)    \
    typedef void (*CoronaObject##NAME##Bookend) (__VA_ARGS__);      \
                                                                    \
    typedef struct CoronaObject##NAME##Params {             \
        CoronaObjectParamsHeader header;                    \
        unsigned char ignoreOriginal, earlyOutIfNonZero;    \
        CoronaObject##NAME##Bookend before, after;          \
    } CoronaObject##NAME##Params

/**
  (TODO)
*/

// ----------------------------------------------------------------------------

/**
  (TODO)
*/
CORONA_OBJECTS_EARLY_OUTABLE_BOOKENDED_PARAMS( BooleanResult, const CoronaDisplayObject * object, void * userData, int * result );

/**
  (TODO)
*/
CORONA_OBJECTS_EARLY_OUTABLE_BOOKENDED_PARAMS( BooleanResultPoint, const CoronaDisplayObject * object, void * userData, float x, float y, int * result );

/**
  (TODO)
*/
CORONA_OBJECTS_EARLY_OUTABLE_BOOKENDED_PARAMS( BooleanResultMatrix, const CoronaDisplayObject * object, void * userData, const float matrix[6], int * result );

// ----------------------------------------------------------------------------

typedef void (*CoronaObjectSetValueBookend) ( const CoronaDisplayObject * object, void * userData, lua_State * L, const char key[], int valueIndex, int * result );

/**
  (TODO)
*/
typedef struct CoronaObjectSetValueParams {
    CoronaObjectParamsHeader header;
    unsigned char ignoreOriginal, disallowEarlyOut;
    CoronaObjectSetValueBookend before, after;
} CoronaObjectSetValueParams;

// ----------------------------------------------------------------------------

typedef void (*CoronaObjectValueBookend) ( const CoronaDisplayObject * object, void * userData, lua_State * L, const char key[], int * result );

/**
  (TODO)
*/
typedef struct CoronaObjectValueParams {
    CoronaObjectParamsHeader header;
    unsigned char ignoreOriginal, disallowEarlyOut : 1, earlyOutIfZero : 1;
    CoronaObjectValueBookend before, after;
} CoronaObjectValueParams;

// ----------------------------------------------------------------------------

/**
  (TODO)
*/
typedef struct CoronaObjectLifetimeParams {
    CoronaObjectParamsHeader header;
    void (*action)( CoronaDisplayObject * object, void * userData );
} CoronaObjectLifetimeParams;

/**
  (TODO)
*/
typedef struct CoronaObjectOnMessageParams {
    CoronaObjectParamsHeader header;
    void (*action)( const CoronaDisplayObject * object, void * userData, const char * message, const void * data, unsigned int size );
} CoronaObjectOnMessageParams;

/**
  (TODO)
*/
typedef struct CoronaObjectParams {
    union {
        CoronaObjectParamsHeader * head;
        int ref;
    } u;
    int useRef;
} CoronaObjectsParams;

// ----------------------------------------------------------------------------

/**
  (TODO) build one offline
*/
CORONA_API
int CoronaObjectsBuildMethodStream( lua_State * L, const CoronaObjectParamsHeader * head ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
  (TODO) L - normally, userData - input, params - methods
*/
CORONA_API
int CoronaObjectsPushContainer( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushEmbossedText( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushEmitter( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushGroup( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushImage( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushImageRect( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushLine( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushMesh( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushPolygon( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushRect( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushRoundedRect( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushSnapshot( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushSprite( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsPushText( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

// NYI

// CORONA_API
// int CoronaObjectsShouldDraw( const CoronaDisplayObject * object, int * shouldDraw ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaObjectsSetHasDummyStageBounds( CoronaDisplayObject * object, int hasBounds ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
  (TODO) scope - probably will never be still in scope...
*/
CORONA_API
CoronaGroupObject * CoronaObjectGetParent( const CoronaDisplayObject * object ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO) scope
*/
CORONA_API
CoronaObject * CoronaGroupObjectGetChild( const CoronaGroupObject * groupObject, int index ) CORONA_PUBLIC_SUFFIX;

/**
  (TODO)
*/
CORONA_API
int CoronaGroupObjectGetNumChildren( const CoronaGroupObject * groupObject ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
  (TODO)
*/
CORONA_API
int CoronaObjectSendMessage( const CoronaDisplayObject * object, const char * message, const void * payload, unsigned int size ) CORONA_PUBLIC_SUFFIX;

#endif // _CoronaObjects_H__
