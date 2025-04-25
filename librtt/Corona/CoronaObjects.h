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
    /**
     No method.
    */
    kAugmentedMethod_None,

    //
    // The following few methods seem to come up most often in test plugins,
    // so put them first, favoring them in sort operations.
    //
    
    /**
     This method is invoked during rendering, if an object is visible and unculled, with `CoronaObjectDrawParams`.
    */
    kAugmentedMethod_Draw,

    /**
     This method is invoked during rendering, to ask whether an object should undergo
     a cull test: if not, it will always be drawn, even if hidden. It is available, with default
     result `false`, through `CoronaObjectBooleanResultParams`.
    */
    kAugmentedMethod_CanCull,

    /**
     This method is invoked during touch events and prepares, to ask whether an object
     should undergo a hit test: if not, it will always be hit, even if hidden. It is available, with
     default result `false`, through `CoronaObjectBooleanResultParams`.
    */
    kAugmentedMethod_CanHitTest, // BooleanResult

    /**
     This method is invoked explicitly on an object via `CoronaObjectSendMessage()`,
     with `CoronaObjectOnMessageParams`.
    */
    kAugmentedMethod_OnMessage,

    /**
     This method is invoked when writing a property to an object, with `CoronaObjectSetValueParams`.
    */
    kAugmentedMethod_SetValue,

    /**
     This method is invoked when reading a property from an object, with `CoronaObjectValueParams`.
    */
    kAugmentedMethod_Value,

    /**
     This method is invoked just before an object is destroyed, with `CoronaObjectOnFinalizeParams`.
    */
    kAugmentedMethod_OnFinalize,

    //
    // The following operations have no obvious use case frequencies.
    //

    /**
     This method is invoked after a new object is added to a group (possibly the stage) but before
     any of its Lua-side presence is established, with `CoronaObjectParentParams`.
    */
    kAugmentedMethod_AddedToParent,

    /**
     This method is invoked when the object stops being visible, with `CoronaObjectBasicParams`.
    */
    kAugmentedMethod_DidMoveOffscreen,

    /**
     This method is invoked following an `UpdateTransform()` and some of its subsequent steps,
     with `CoronaObjectMatrixParams`.
    */
    kAugmentedMethod_DidUpdateTransform,

    /**
     This method is invoked when an object must calculate its bounds, with `CoronaObjectRectResultParams`.
    */
    kAugmentedMethod_GetSelfBounds,

    /**
     This method is invoked when an object must calculate its anchor-related bounds, with `CoronaObjectRectResultParams`.
    */
    kAugmentedMethod_GetSelfBoundsForAnchor,

    /**
     This method is invoked when an object is hit-tested for touch purposes, with `CoronaObjectBooleanResultPointParams`.
    */
    kAugmentedMethod_HitTest,

    /**
     This method is invoked after an object has been constructed and set up, with
     `CoronaObjectOnCreateParams`.
    */
    kAugmentedMethod_OnCreate,

    /**
     This method is invoked when an object is setting up some resources it needs, with `CoronaObjectBasicParams`.
    */
    kAugmentedMethod_Prepare,

    /**
     This method is invoked just before an object is removed, while still belonging to a parent
     (possibly the stage), with `CoronaObjectParentParams`.
    */
    kAugmentedMethod_RemovedFromParent,

    /**
     This method is invoked when the object's rotation is assigned or updated, with `CoronaObjectRotateParams`.
    */
    kAugmentedMethod_Rotate,

    /**
     This method is invoked when the object's scale is assigned or updated, with `CoronaObjectScaleParams`.
    */
    kAugmentedMethod_Scale,

    /**
     This method is invoked when the object's position is assigned or updated, with `CoronaObjectTranslateParams`.
    */
    kAugmentedMethod_Translate,

    /**
     This method is invoked in various circumstances where the object's transformation matrix has
     been updated, with `CoronaObjectBooleanResultMatrix` params.
    */
    kAugmentedMethod_UpdateTransform,

    /**
     This method is invoked when the object becomes visible, with `CoronaObjectBasicParams`.
    */
    kAugmentedMethod_WillMoveOnscreen,

    //
    // The following operations are relevant to groups.
    //

    /**
     This method is invoked after inserting a child into a group, with `CoronaObjectDidInsertParams`.
    */
    kAugmentedMethod_DidInsert,

    /**
     This method is invoked after removing a child from a group, with `CoronaObjectGroupBasicParams`.
    */
    kAugmentedMethod_DidRemove,

    /**
     The number of valid augmentable methods.
    */
    kAugmentedMethod_Count
} CoronaObjectAugmentedMethod;

/**
 Each of the `CoronaObject*Params` structures has this as its first member, to effect C-style inheritance.
*/
typedef struct CoronaObjectParamsHeader {
    /**
     Link to the next method parameter structure in the chain, or `NULL` if this is the last one.
    */
    struct CoronaObjectParamsHeader * next;
    
    /**
     The appropriate member of `CoronaObjectAugmentedMethod` that identifies the payload that follows.
    */
    unsigned short method; // n.b. quite generous: all methods fit easily within a byte
} CoronaObjectParamsHeader;

// ----------------------------------------------------------------------------

#define CORONA_OBJECTS_BOOKENDED_PARAMS(NAME, ...)                \
    typedef void (*CoronaObject##NAME##Bookend) (__VA_ARGS__);    \
                                                                  \
    typedef struct CoronaObject##NAME##Params {     \
        CoronaObjectParamsHeader header;            \
        CoronaObject##NAME##Bookend before, after;  \
        int ignoreOriginal, separateScopes;         \
    } CoronaObject##NAME##Params

/**
 IgnorableMethodParams
 
 This may be used to augment and/or override a built-in method.
 
 With a given method, this can take on the form:
 
 ```
   before( ... )
   original( ... )
   after( ... )
 ```
 
 where all three functions take the same arguments.

 The `before` and `after` functions may be NULL, in which case the respective function is
 not called. Similarly, the stock behavior is skipped if `ignoreOriginal` is non-0.
*/

// ----------------------------------------------------------------------------

/**
 CoronaObjectBasicParams
 
 Inherits from IgnorableMethodParams.

 This is for the most basic method params, whose functions have signature `method( const CoronaDisplayObject * self, void * userData )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Basic, const CoronaDisplayObject * self, void * userData );

/**
 CoronaObjectParentParams
 
 Inherits from IgnorableMethodParams.

 These are method params related to parenting, whose functions have signature `method( const CoronaDisplayObject * self, void * userData, lua_State * L, const CoronaGroupObject * groupObject )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Parent, const CoronaDisplayObject * self, void * userData, lua_State * L, const CoronaGroupObject * groupObject );

/**
 CoronaObjectMatrixParams
 
 Inherits from IgnorableMethodParams.

 These are method params that provide an input/output matrix, whose functions have signature `method( const CoronaDisplayObject * self, void * userData, float matrix[6] )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Matrix, const CoronaDisplayObject * self, void * userData, float matrix[6] );

/**
 CoronaObjectDrawParams
 
 Inherits from IgnorableMethodParams.

 These are method params related to draw opportunities, whose functions have signature `method( const CoronaDisplayObject * self, void * userData, const CoronaRenderer * renderer )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Draw, const CoronaDisplayObject * self, void * userData, const CoronaRenderer * renderer );

/**
 CoronaObjectRectResultParams
 
 Inherits from IgnorableMethodParams.

 These are method params related to rectangular outputs, whose functions have signature `method( const CoronaDisplayObject * self, void * userData, float * xMin, float * yMin, float * xMax, float * yMax )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( RectResult, const CoronaDisplayObject * self, void * userData, float * xMin, float * yMin, float * xMax, float * yMax );

/**
 CoronaObjectRotateParams
 
 Inherits from IgnorableMethodParams.

 These are method params related to rotations, whose functions have signature `method( const CoronaDisplayObject * self, void * userData, float delta )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Rotate, const CoronaDisplayObject * self, void * userData, float delta );

/**
 CoronaObjectScaleParams
 
 Inherits from IgnorableMethodParams.

 These are method params related to scalings, whose functions have signature `method( const CoronaDisplayObject * self, void * userData, float sx, float sy, int isNew )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Scale, const CoronaDisplayObject * self, void * userData, float sx, float sy, int isNew );

/**
 CoronaObjectTranslateParams
 
 Inherits from IgnorableMethodParams.
 
 These are method params related to translations, whose functions have signature `method( const CoronaDisplayObject * self, void * userData, float x, float y )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( Translate, const CoronaDisplayObject * self, void * userData, float x, float y );

/**
 CoronaObjectDidInsertParams
 
 Inherits from IgnorableMethodParams.

 These are method params related to insertions, whose functions have signature `method( CoronaGroupObject * self, void * userData, int childParentChanged )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( DidInsert, const CoronaGroupObject * self, void * userData, int childParentChanged );

/**
 CoronaGroupBasicParams
 
 Inherits from IgnorableMethodParams.

 This is for the most basic group method params, whose functions have signature `method( CoronaGroupObject * self, void * userData )`.
*/
CORONA_OBJECTS_BOOKENDED_PARAMS( GroupBasic, const CoronaGroupObject * self, void * userData );

// ----------------------------------------------------------------------------

#define CORONA_OBJECTS_EARLY_OUTABLE_BOOKENDED_PARAMS(NAME, ...)    \
    typedef void (*CoronaObject##NAME##Bookend) (__VA_ARGS__);      \
                                                                    \
    typedef struct CoronaObject##NAME##Params {               \
        CoronaObjectParamsHeader header;                      \
        CoronaObject##NAME##Bookend before, after;            \
        int ignoreOriginal, earlyOutIfNonZero, separateScopes;\
    } CoronaObject##NAME##Params

/**
 EarlyOutableIgnorableMethodParams

 This may also be used to augment and/or override a built-in method. When `before` logic is
 provided, getting certain results might rule out any meaningful follow-up behavior, so early-outs
 are available.

 With a given method, this can take on the form:
 
 ```
   local result = default
   if before then
     result = before( ..., result )
     if CanEarlyOut( result ) then
      return result
     end
   end
   result = original( ..., result );
   result = after( ..., result );
 ```
 
 where all three functions take the same arguments.

 At the moment, the `CanEarlyOut` predicate is either "result was true" (`earlyOutIfNonZero`)
 or "result was false".

 The `before` and `after` functions may be NULL, in which case the respective function is
 not called. Similarly, the stock behavior is skipped if `ignoreOriginal` is non-0.
*/

// ----------------------------------------------------------------------------

/**
 CoronaObjectBooleanResultParams
 
 Inherits from EarlyOutableIgnorableMethodParams.

 These are method params related to boolean outputs, whose functions have signature `method( const CoronaDisplayObject * self, void * userData, int * result )`.
*/
CORONA_OBJECTS_EARLY_OUTABLE_BOOKENDED_PARAMS( BooleanResult, const CoronaDisplayObject * self, void * userData, int * result );

/**
 CoronaObjectBooleanResultPointParams
 
 Inherits from EarlyOutableIgnorableMethodParams.

 These are method params related to point inputs and boolean outputs, whose functions have
 signature `method( const CoronaDisplayObject * self, void * userData, float x, float y, int * result )`.
*/
CORONA_OBJECTS_EARLY_OUTABLE_BOOKENDED_PARAMS( BooleanResultPoint, const CoronaDisplayObject * self, void * userData, float x, float y, int * result );

/**
 CoronaObjectBooleanResultMatrixParams
 
 Inherits from EarlyOutableIgnorableMethodParams.
 
 These are method params related to matrix inputs and boolean outputs, whose functions have
 signature `method( const CoronaDisplayObject * self, void * userData, const float matrix[6], int * result )`.
*/
CORONA_OBJECTS_EARLY_OUTABLE_BOOKENDED_PARAMS( BooleanResultMatrix, const CoronaDisplayObject * self, void * userData, const float matrix[6], int * result );

// ----------------------------------------------------------------------------

typedef void (*CoronaObjectSetValueBookend) ( const CoronaDisplayObject * self, void * userData, lua_State * L, const char key[], int valueIndex, int * result );

/**
 CoronaObjectSetValueParams
 
 This may be used to augment and/or override the `SetValue` method. When `before` logic
 is provided, setting a value, might rule out any meaningful follow-up behavior, so early-outs
 are available.

 This can take on the form:
 
 ```
   bool result = false
   if before then
     result = before( ..., result )
     if CanEarlyOut( result ) then
      return result
     end
   end
   result = original( ..., result );
   result = after( ..., result );
 ```
 
 where all three functions take the same arguments.

 At the moment, the `CanEarlyOut` predicate is "result is true": a value was assigned.
 It is also possible to suppress early-outs by setting `disallowEarlyOut`.

 The `before` and `after` functions may be NULL, in which case the respective function is
 not called. Similarly, the stock behavior is skipped if `ignoreOriginal` is non-0.
 
 Its functions have signature `method( const CoronaDisplayObject * self, void * userData, lua_State * L, const char key[], int valueIndex, int * result )`.
*/
typedef struct CoronaObjectSetValueParams {
    CoronaObjectParamsHeader header;
    CoronaObjectSetValueBookend before, after;
    int ignoreOriginal, disallowEarlyOut, separateScopes;
} CoronaObjectSetValueParams;

// ----------------------------------------------------------------------------

typedef void (*CoronaObjectValueBookend) ( const CoronaDisplayObject * self, void * userData, lua_State * L, const char key[], int * result );

/**
 CoronaObjectValueParams

 This may be used to augment and/or override the `Value` method. When `before` logic is
 provided, getting a result (or alternatively, not getting one) might rule out any meaningful
 follow-up behavior, so early-outs are available.

 This can take on the form:
 
 ```
   local result = 0
   if before then
     result = before( ..., result )
     if CanEarlyOut( result ) then
      return result
     end
   end
   result = result + original( ..., result );
   result = after( ..., result );
 ```
 
 where all three functions take the same arguments.

 At the moment, the `CanEarlyOut` predicate is either "result is 0" ( `earlyOutIfZero`), or
 "result is non-0". It is also possible to suppress early-outs by setting `disallowEarlyOut`.

 The `before` and `after` functions may be NULL, in which case the respective function is
 not called. Similarly, the stock behavior is skipped if `ignoreOriginal` is non-0.

 Although only the top value on the stack will actually be used as the ultimate value, the
 intermediate results can be accumulated, say to concatenate multiple values at the end.
 
 Its functions have signature `method( const CoronaDisplayObject * self, void * userData, lua_State * L, const char key[], int * result )`.
*/
typedef struct CoronaObjectValueParams {
    CoronaObjectParamsHeader header;
    CoronaObjectValueBookend before, after;
    int ignoreOriginal, disallowEarlyOut, earlyOutIfZero, separateScopes;
} CoronaObjectValueParams;

// ----------------------------------------------------------------------------

/**
 These are method params related to create events, whose `action` has signature
 `method( const CoronaDisplayObject * self, void ** userData )`.
 
 The original value of `*userData` comes from the argument to a `CoronaObjectsPush*` function;
 its value after this method concludes will be used by any subsequent methods.
*/
typedef struct CoronaObjectOnCreateParams {
    CoronaObjectParamsHeader header;
    void (*action)( const CoronaDisplayObject * self, void ** userData );
} CoronaObjectOnCreateParams;

/**
 These are method params related to finalize events, whose `action` has signature
 `method( const CoronaDisplayObject * self, void * userData )`.
*/
typedef struct CoronaObjectOnFinalizeParams {
    CoronaObjectParamsHeader header;
    void (*action)( const CoronaDisplayObject * self, void * userData );
} CoronaObjectOnFinalizeParams;

/**
These method params belong to messages, with `action` having signature `method( const CoronaDisplayObject * self, void * userData, const char * message, const void * data, unsigned int size )`.
*/
typedef struct CoronaObjectOnMessageParams {
    CoronaObjectParamsHeader header;
    void (*action)( const CoronaDisplayObject * self, void * userData, const char * message, const void * data, unsigned int size );
    int preserveScope;
} CoronaObjectOnMessageParams;

/**
 This structure describes the method customizations for a new display object.
*/
typedef struct CoronaObjectParams {
    union {
        /**
         A chain of method parameters. This is suitable for temporary situations, for instance
         if the parameters are on the stack. A dedicated method stream is built for the object
         when pushed.
        */
        CoronaObjectParamsHeader * head;

        /**
         A Lua reference returned by `CoronaObjectsBuildMethodStream()`.
        */
        int ref;
    } u;
    
    /**
     If non-0, the method parameter chain is represented by `ref`; else `head`.
    */
    int useRef;
} CoronaObjectParams;

// ----------------------------------------------------------------------------

/**
 Build an immutable stream that may be shared among multiple objects.
 @param head The first in a chain of parameters, ending when a `next` of `NULL` is found.
            Any elements with `kAugmentedMethod_None` as their `method` are ignored;
            otherwise, any value of `method` must occur at most once.
 @return If successful, a Lua reference to the built stream; else `LUA_REFNIL`.
*/
CORONA_API
int CoronaObjectsBuildMethodStream( lua_State * L, const CoronaObjectParamsHeader * head ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 This behaves like `display.newContainer()` but allows method customization.

 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the container's methods. It is not owned by the
                object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means container was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushContainer( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newEmbossedText()` but allows method customization.

 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the text object's methods. It is not owned by the
                object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means text object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushEmbossedText( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newEmitter()` but allows method customization.

 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the emitter object's methods. It is not owned by
                the object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means emitter object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushEmitter( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newGroup()` but allows method customization.

 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the group's methods. It is not owned by the
                object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means group was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushGroup( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newImage()` but allows method customization.

 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the image object's methods. It is not owned by
                the object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means image object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushImage( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
  This behaves like `display.newImageRect()` but allows method customization.

 The stack contents are used as the arguments.
  @param userData Arbitrary data supplied to the image rect object's methods. It is not owned
                 by the object and it is up to the user to keep it alive while any methods use it.
  @param params Method parameter chain.
  @return number of values pushed onto stack;
          1 - means image rect object  was successfully created and is on stack
          0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushImageRect( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newLine()` but allows method customization.
 
 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the line object's methods. It is not owned by the
                object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means line object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushLine( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newMesh()` but allows method customization.
 
 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the mesh object's methods. It is not owned by
                the object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means mesh object  was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushMesh( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newPolygon()` but allows method customization.
 
 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the polygon object's methods. It is not owned by
                the object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means polygon object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushPolygon( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newRect()` but allows method customization.
 
 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the rect object's methods. It is not owned by
                the object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means rect object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushRect( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newRoundedRect()` but allows method customization.
 
 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the rounded rect's methods. It is not owned by
                the object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means rounded rect object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushRoundedRect( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newSnapshot()` but allows method customization.
 
 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the snapshot object's methods. It is not owned by
                the object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means snapshot object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushSnapshot( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newSprite()` but allows method customization.
 
 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the sprite object's methods. It is not owned by
                the object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means sprite object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushSprite( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

/**
 This behaves like `display.newText()` but allows method customization.
 
 The stack contents are used as the arguments.
 @param userData Arbitrary data supplied to the text object's methods. It is not owned by
                the object and it is up to the user to keep it alive while any methods use it.
 @param params Method parameter chain.
 @return number of values pushed onto stack;
         1 - means text object was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaObjectsPushText( lua_State * L, void * userData, const CoronaObjectParams * params ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 Invalidate an object in the display hierachy.
 @param object Boxed display object.
 @return If non-0, the object was invalidated.
*/
CORONA_API
int CoronaObjectInvalidate( const CoronaDisplayObject * object ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 Get a slot in the current scope, to be used by certain functions to receive objects.
 @return Handle to the slot, or NULL if all are in use.
*/
CORONA_API
const CoronaAny * CoronaObjectGetAvailableSlot( void ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 @param object Boxed display object.
 @param parent Handle to receive the parent. (Obtained from `CoronaObjectGetAvailableSlot`.)
 @return If non-0, the parent was retrieved.
*/
CORONA_API
int CoronaObjectGetParent( const CoronaDisplayObject * object, const CoronaGroupObject* parent ) CORONA_PUBLIC_SUFFIX;

/**
 @param groupObject Boxed group object.
 @param index Index of child belonging to `groupObject`, from 0 to `numChildren`.
 @param child Handle to receive the child. (Obtained from `CoronaObjectGetAvailableSlot`.)
 @return If non-0, the child was retrieved.
*/
CORONA_API
int CoronaGroupObjectGetChild( const CoronaGroupObject * groupObject, int index, const CoronaDisplayObject* child ) CORONA_PUBLIC_SUFFIX;

/**
 @param groupObject Boxed group object.
 @return Count of children (`numChildren`), if `groupObject` was valid (otherwise 0).
*/
CORONA_API
int CoronaGroupObjectGetNumChildren( const CoronaGroupObject * groupObject ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 Send a message immediately to a given display object.
 @param object Boxed display object.
 @param message Null-terminated string describing the message and its payload.
 @param payload Arbitrary payload appropriate to `message`.
 @param size Number of bytes in `payload`.
 @return If non-0, the message was delivered.
*/
CORONA_API
int CoronaObjectSendMessage( const CoronaDisplayObject * object, const char * message, const void * payload, unsigned int size ) CORONA_PUBLIC_SUFFIX;

#endif // _CoronaObjects_H__
