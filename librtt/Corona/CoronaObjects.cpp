//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "CoronaObjects.h"
#include "CoronaLua.h"

#include "Rtt_LuaContext.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"

#include "Display/Rtt_ObjectBoxList.h"

#include "Display/Rtt_ContainerObject.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_EmbossedTextObject.h"
#include "Display/Rtt_EmitterObject.h"
#include "Display/Rtt_GroupObject.h"
#include "Display/Rtt_LineObject.h"
#include "Display/Rtt_RectObject.h"
#include "Display/Rtt_RectPath.h"
#include "Display/Rtt_SnapshotObject.h"
#include "Display/Rtt_SpriteObject.h"
#include "Display/Rtt_StageObject.h"
#include "Display/Rtt_TextObject.h"

#include <vector>
#include <stddef.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ObjectBoxList::ObjectBoxList()
{
}
    
ObjectBoxList::~ObjectBoxList()
{
    for (Box & box : fBoxes)
    {
        box.fObject = NULL;
    }
}

const char *
ObjectBoxList::StringForType (int type)
{
    switch (type)
    {
    case kRenderer:
        return "Renderer";
    case kRenderData:
        return "RenderData";
    case kShader:
        return "Shader";
    case kShaderData:
        return "ShaderData";
    case kDisplayObject:
        return "DisplayObject";
    case kGroupObject:
        return "GroupObject";
    case kCommandBuffer:
        return "CommandBuffer";
    default:
        return "Unknown";
    }
}

bool
ObjectBoxList::CanGetObject( const Box * box, int type )
{
    return type == box->fType || (kGroupObject == box->fType && kDisplayObject == type); // see Add() method
}

ObjectBoxList *
ObjectBoxList::GetList( const Box * box )
{
    void * list = const_cast< void * >( box->fList );
    
    return ( ObjectBoxList * )list;
}

bool
ObjectBoxList::CheckObject( const Box * box, int type )
{
    bool ok = false;

    if (NULL == box)
    {
        CORONA_LOG_ERROR( "NULL %s supplied", StringForType( type ) );
    }

    else if (NULL == box->fObject)
    {
        CORONA_LOG_ERROR( "%s object has been invalidated", StringForType( type ) );
    }

    else if (!CanGetObject( box, type ))
    {
        CORONA_LOG_ERROR( "Expected %s but have %s", StringForType( type ), StringForType( box->fType ) );
    }
    
    else
    {
        ok = true;
    }
    
    return ok;
}

void *
ObjectBoxList::GetObject( const Box * box, int type )
{
    if (CheckObject( box, type ))
    {
        return box->fObject;
    }
    
    else
    {
        return NULL;
    }
}

ObjectBoxList::Box *
ObjectBoxList::Add( const void * object, int type )
{
    if (kDisplayObject == type) // some objects need to be more specific...
                                // this is complemented by CanGetObject()
    {
        void * nonConst = const_cast< void * >( object );
        DisplayObject * displayObject = static_cast< DisplayObject * >( nonConst );
        
        if (NULL != displayObject->AsGroupObject()) // ...if so, promote it
        {
            type = kGroupObject;
        }
    }
    
    // TODO?: with certain types (so far, probably just display / group
    // objects), in some extreme cases the list could become quite dense:
    // if that ever becomes a problem, we might consider something like a
    // bloom filter (https://en.wikipedia.org/wiki/Bloom_filter) or a
    // Roaring bitmap (https://github.com/RoaringBitmap/CRoaring) paired
    // with a map.
    
    for (Box & box : fBoxes) // already boxed?
    {
        Rtt_ASSERT( box.fList == this );
        
        if (box.fObject == object && box.fType == type)
        {
            return &box;
        }
    }
    
    // new entry
    fBoxes.push_back( Box{} );
    
    Box & newBox = fBoxes.back();
    
    newBox.fList = this;
    newBox.fObject = const_cast< void * >( object );
    newBox.fType = type;
    
    return &newBox;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#define CORONA_OBJECTS_STREAM_METATABLE_NAME "CoronaObjectsStream"

#define PARAMS(NAME) CoronaObject##NAME##Params
#define AFTER_HEADER_STRUCT(NAME) \
    struct NAME##Struct { unsigned char bytes[ sizeof( PARAMS( NAME ) ) - AFTER_HEADER_OFFSET( NAME ) ]; } NAME

union GenericParams {
    #define AFTER_HEADER_OFFSET(NAME) offsetof( PARAMS( NAME ), action )

    AFTER_HEADER_STRUCT( OnCreate );
    AFTER_HEADER_STRUCT( OnFinalize );
    AFTER_HEADER_STRUCT( OnMessage );

    #undef AFTER_HEADER_OFFSET
    #define AFTER_HEADER_OFFSET(NAME) offsetof( PARAMS( NAME ), ignoreOriginal )

    AFTER_HEADER_STRUCT( Basic );
    AFTER_HEADER_STRUCT( GroupBasic );
    AFTER_HEADER_STRUCT( Parent );
    AFTER_HEADER_STRUCT( DidInsert );
    AFTER_HEADER_STRUCT( Matrix );
    AFTER_HEADER_STRUCT( Draw );
    AFTER_HEADER_STRUCT( RectResult );
    AFTER_HEADER_STRUCT( Rotate );
    AFTER_HEADER_STRUCT( Scale );
    AFTER_HEADER_STRUCT( Translate );

    AFTER_HEADER_STRUCT( BooleanResult );
    AFTER_HEADER_STRUCT( BooleanResultPoint );
    AFTER_HEADER_STRUCT( BooleanResultMatrix );

    AFTER_HEADER_STRUCT( SetValue );
    AFTER_HEADER_STRUCT( Value );
};

template<typename T> T
FindParams( const unsigned char * stream, unsigned short method, size_t offset )
{
    static_assert( kAugmentedMethod_Count < 256, "Stream assumes byte-sized methods" );
    
    T out = {};

    unsigned int count = *stream;
    const unsigned char * methods = stream + 1, * genericParams = methods + count;

    if (method < count) // methods are sorted, so cannot take more than `method` steps
    {
        count = method;
    }

    for (unsigned int i = 0; i < count; ++i, genericParams += sizeof( GenericParams ))
    {
        if (methods[i] == method)
        {
            memcpy( reinterpret_cast< unsigned char * >( &out ) + offset, genericParams, sizeof( T ) - offset );

            break;
        }
    }

    return out;
}

// ----------------------------------------------------------------------------

static bool
ValuePrologue( lua_State * L, const Rtt::MLuaProxyable& o, const char key[], void * userData, const CoronaObjectValueParams & params, int * result )
{
    if (params.before)
    {
        Rtt::ObjectBoxList list;

        // on some platforms, passing &o works fine, others not,
        // owing to different virtual method implementations; so
        // ensure we're looking at the object itself (we do this
        // in the next several functions too)
        const Rtt::DisplayObject& object = static_cast< const Rtt::DisplayObject& >( o );
        
        OBJECT_BOX_STORE( DisplayObject, storedObject, &object );
    
        params.before( storedObject, userData, L, key, result );

        bool canEarlyOut = !params.disallowEarlyOut, expectsNonZero = !params.earlyOutIfZero;

        if (canEarlyOut && expectsNonZero == !!*result)
        {
            return false;
        }
    }

    return true;
}

static int
ValueEpilogue( lua_State * L, const Rtt::MLuaProxyable& o, const char key[], void * userData, const CoronaObjectValueParams & params, int result )
{
    if (params.after)
    {
        Rtt::ObjectBoxList list;
    
        const Rtt::DisplayObject& object = static_cast< const Rtt::DisplayObject& >( o );
        
        OBJECT_BOX_STORE( DisplayObject, storedObject, &object );

        params.after( storedObject, userData, L, key, &result ); // n.b. `result` previous values still on stack
    }

    return result;
}

// ----------------------------------------------------------------------------

static bool
SetValuePrologue( lua_State * L, Rtt::MLuaProxyable& o, const char key[], int valueIndex, void * userData, const CoronaObjectSetValueParams & params, int * result )
{
    if (params.before)
    {
        Rtt::ObjectBoxList list;
        
        Rtt::DisplayObject& object = static_cast< Rtt::DisplayObject& >( o );
        
        OBJECT_BOX_STORE( DisplayObject, storedObject, &object );

        params.before( storedObject, userData, L, key, valueIndex, result );

        bool canEarlyOut = !params.disallowEarlyOut;

        if (canEarlyOut && *result)
        {
            return false;
        }
    }

    return true;
}

static bool
SetValueEpilogue( lua_State * L, Rtt::MLuaProxyable& o, const char key[], int valueIndex, void * userData, const CoronaObjectSetValueParams & params, int result )
{
    if (params.after)
    {
        Rtt::ObjectBoxList list;

        Rtt::DisplayObject& object = static_cast< Rtt::DisplayObject& >( o );
        
        OBJECT_BOX_STORE( DisplayObject, storedObject, &object );

        params.after( storedObject, userData, L, key, valueIndex, &result );
    }

    return result;
}

// ----------------------------------------------------------------------------

template<typename Proxy2, typename BaseProxyVTable>
class Proxy2VTable : public BaseProxyVTable
{
public:
    typedef Proxy2VTable Self;
    typedef BaseProxyVTable Super;

public:
    static const Self& Constant() { static const Self kVTable; return kVTable; }

protected:
    Proxy2VTable() {}

public:
    virtual int ValueForKey( lua_State *L, const Rtt::MLuaProxyable& object, const char key[], bool overrideRestriction = false ) const
    {
        const Proxy2 & resolved = static_cast< const Proxy2 & >( object );
        const auto params = FindParams< CoronaObjectValueParams >( resolved.fStream, kAugmentedMethod_Value, AFTER_HEADER_OFFSET( Value ) );
        void * userData = const_cast< void * >( resolved.fUserData );
        int result = 0;

        if (!ValuePrologue( L, object, key, userData, params, &result ))
        {
            return result;
        }

        else if (!params.ignoreOriginal)
        {
            result += Super::Constant().ValueForKey( L, object, key, overrideRestriction );
        }

        return ValueEpilogue( L, object, key, userData, params, result );
    }

    virtual bool SetValueForKey( lua_State *L, Rtt::MLuaProxyable& object, const char key[], int valueIndex ) const
    {
        const Proxy2 & resolved = static_cast< const Proxy2 & >( object );
        const auto params = FindParams< CoronaObjectSetValueParams >( resolved.fStream, kAugmentedMethod_SetValue, AFTER_HEADER_OFFSET( SetValue ) );
        void * userData = const_cast< void * >( resolved.fUserData );
        int result = 0;

        if (!SetValuePrologue( L, object, key, valueIndex, userData, params, &result ))
        {
            return result;
        }

        else if (!params.ignoreOriginal)
        {
            result = Super::Constant().SetValueForKey( L, object, key, valueIndex );
        }

        return SetValueEpilogue( L, object, key, valueIndex, userData, params, result );
    }

    virtual const Rtt::LuaProxyVTable& Parent() const { return Super::Constant(); }
    virtual const Rtt::LuaProxyVTable& ProxyVTable() const { return Self::Constant(); }
};

#undef AFTER_HEADER_OFFSET

// ----------------------------------------------------------------------------

static bool
PushFactory( lua_State * L, const char * name )
{
    Rtt::Display & display = Rtt::LuaContext::GetRuntime( L )->GetDisplay();

    if (!display.PushObjectFactories()) // stream, ...[, factories]
    {
        return false;
    }

    lua_getfield( L, -1, name ); // stream, ..., factories, factory

    return true;
}

static bool
CallNewFactory( lua_State * L, const char * name, Rtt::DisplayObject::BoxedFunction * funcBox  )
{
    if (PushFactory( L, name ) ) // stream, ...[, factories, factory]
    {
        lua_pushlightuserdata( L, funcBox ); // stream, ..., factories, factory, funcBox
        lua_setupvalue( L, -2, 2 ); // stream, ..., factories, factory; factory.upvalue[2] = funcBox
        lua_insert( L, 2 ); // stream, factory, ..., factories
        lua_pop( L, 1 ); // stream, factory, ...
        lua_call( L, lua_gettop( L ) - 2, 1 ); // stream, object?

        return !lua_isnil( L, 2 );
    }

    return false;
}

// ----------------------------------------------------------------------------

static void
GetSizes( unsigned short method, size_t & fullSize, size_t & paramSize )
{
#define GET_SIZES(NAME)                             \
    fullSize = sizeof( PARAMS( NAME ) );            \
    paramSize = sizeof( GenericParams::NAME.bytes )

#define UNIQUE_METHOD(NAME)     \
    kAugmentedMethod_##NAME:    \
        GET_SIZES(NAME)

    switch (method)
    {
    case kAugmentedMethod_DidMoveOffscreen:
    case kAugmentedMethod_Prepare:
    case kAugmentedMethod_WillMoveOnscreen:
    case kAugmentedMethod_DidRemove:
        GET_SIZES( Basic );

        break;
    case kAugmentedMethod_AddedToParent:
    case kAugmentedMethod_RemovedFromParent:
        GET_SIZES( Parent );
            
        break;
    case kAugmentedMethod_CanCull:
    case kAugmentedMethod_CanHitTest:
        GET_SIZES( BooleanResult );

        break;
    case kAugmentedMethod_GetSelfBounds:
    case kAugmentedMethod_GetSelfBoundsForAnchor:
        GET_SIZES( RectResult );

        break;
    case kAugmentedMethod_DidUpdateTransform:
        GET_SIZES( Matrix );

        break;
    case kAugmentedMethod_HitTest:
        GET_SIZES( BooleanResultPoint );

        break;
    case kAugmentedMethod_UpdateTransform:
        GET_SIZES( BooleanResultMatrix );

        break;
    case UNIQUE_METHOD( DidInsert );
        break;
    case UNIQUE_METHOD( Draw );
        break;
    case UNIQUE_METHOD( OnCreate );
        break;
    case UNIQUE_METHOD( OnFinalize );
        break;
    case UNIQUE_METHOD( OnMessage );
        break;
    case UNIQUE_METHOD( Rotate );
        break;
    case UNIQUE_METHOD( Scale );
        break;
    case UNIQUE_METHOD( SetValue );
        break;
    case UNIQUE_METHOD( Translate );
        break;
    case UNIQUE_METHOD( Value );
        break;
    default:
        Rtt_ASSERT_NOT_REACHED();
    }

#undef GET_SIZES
#undef UNIQUE_METHOD
}

static bool
BuildMethodStream( lua_State * L, const CoronaObjectParamsHeader * head )
{
    if (!head)
    {
        CORONA_LOG_ERROR( "NULL method list" );
        
        return false;
    }

    std::vector< const CoronaObjectParamsHeader * > params;

    for (const CoronaObjectParamsHeader * cur = head; cur; cur = cur->next)
    {
        if (cur->method != kAugmentedMethod_None) // allow these as a convenience, mainly for the head
        {
            params.push_back( cur );
        }
    }

    if (params.empty())
    {
        CORONA_LOG_ERROR( "No method conversions supplied" );
        
        return false;
    }

    // After being built, the stream is immutable, so sort by method to allow some additional optimizations.
    std::sort( params.begin(), params.end(), [](const CoronaObjectParamsHeader * p1, const CoronaObjectParamsHeader * p2) { return p1->method < p2->method; });

    if (params.back()->method >= (unsigned short)( kAugmentedMethod_Count )) // has bogus method(s)?
    {
        CORONA_LOG_ERROR( "Invalid methods supplied" );
        
        return false;
    }

    // Check for duplicates.
    unsigned short prev = (unsigned short)~0U;

    for ( const CoronaObjectParamsHeader * header : params )
    {
        if (header->method == prev)
        {
            CORONA_LOG_ERROR( "Method `%u` was supplied more than once", (unsigned int)prev );
            
            return false;
        }

        prev = header->method;
    }

    unsigned char * stream = (unsigned char *)lua_newuserdata( L, 1U + (1U + sizeof( GenericParams )) * params.size() ); // ..., stream

    luaL_newmetatable( L, CORONA_OBJECTS_STREAM_METATABLE_NAME ); // ..., stream, mt
    lua_setmetatable( L, -2 ); // ..., stream; stream.metatable = mt

    *stream = (unsigned char)( params.size() );

    unsigned char * methods = stream + 1, * genericParams = methods + params.size();

    for (size_t i = 0; i < params.size(); ++i, genericParams += sizeof( GenericParams ))
    {
        methods[i] = (unsigned char)params[i]->method;

        size_t fullSize, paramSize;

        GetSizes( params[i]->method, fullSize, paramSize ); // we no longer need the next pointers and have the methods in front, so only want the post-header size...
        memcpy( genericParams, reinterpret_cast< const unsigned char * >( params[i] ) + (fullSize - paramSize), paramSize ); // ...and content
    }

    return true;
}

static bool
GetStream( lua_State * L, const CoronaObjectsParams * params )
{
    bool hasStream = false;

    if (params)
    {
        if (params->useRef)
        {
            lua_getref( L, params->u.ref ); // ..., stream?

            if (lua_getmetatable( L, -1 )) // ..., stream?[, mt1]
            {
                luaL_getmetatable( L, CORONA_OBJECTS_STREAM_METATABLE_NAME ); // ..., stream?, mt1, mt2

                hasStream = lua_equal( L, -2, -1 );

                lua_pop( L, 2 ); // ..., stream?
            }

            else
            {
                CORONA_LOG_ERROR( "Invalid method stream" );
                
                lua_pop( L, 1 ); // ...
            }
        }

        else
        {
            hasStream = BuildMethodStream( L, params->u.head ); // ...[, stream]
        }
    }

    return hasStream;
}

struct StreamAndUserData {
    unsigned char * stream;
    void * userData;
};

static StreamAndUserData sStreamAndUserData;

template<typename T>
int PushObject( lua_State * L, void * userData, const CoronaObjectParams * params, const char * name, void (*func)() )
{
    if (!GetStream( L, params )) // ...[, stream]
    {
        lua_settop( L, 0 ); // (empty)
        
        return 0;
    }

    lua_insert( L, 1 ); // stream, ...
    
    sStreamAndUserData.stream = (unsigned char *)lua_touserdata( L, 1 );
    sStreamAndUserData.userData = userData;

    Rtt::DisplayObject::BoxedFunction funcBox = { *(void **)&func }; // https://stackoverflow.com/a/16682718
    
    if (CallNewFactory( L, name, &funcBox )) // stream[, object]
    {
        T * object = (T *)Rtt::LuaProxy::GetProxyableObject( L, 2 );

        lua_insert( L, 1 ); // object, stream

        if (!params->useRef) // temporary?
        {
            object->fRef = luaL_ref( L, LUA_REGISTRYINDEX ); // object; registry = { ..., [ref] = stream }
        }
        
        else // guard stream in case reference is dropped; this is redundant after the first object using the stream, but harmless
        {
            lua_pushlightuserdata( L, lua_touserdata( L, -1 ) ); // object, stream, stream_ptr
            lua_insert( L, 2 ); // object, stream_ptr, stream
            lua_rawset( L, LUA_REGISTRYINDEX ); // object; registry = { [stream_ptr] = stream }
        }

        return 1;
    }

    else
    {
        lua_settop( L, 0 ); // (empty)
    }

    return 0;
}

// ----------------------------------------------------------------------------

using FPtr = void (*)();

#define CORONA_OBJECTS_PUSH(OBJECT_KIND) return PushObject< OBJECT_KIND##2 >( L, userData, params, "new" #OBJECT_KIND, ( FPtr )OBJECT_KIND##2::New )

#define FIRST_ARGS storedThis, fUserData
#define CORONA_OBJECTS_METHOD_BOOKEND(WHEN, ...)    \
    if (params.WHEN)                                \
    {                                               \
        params.WHEN( __VA_ARGS__ );                 \
    }

#define CORONA_OBJECTS_METHOD_CORE(METHOD_NAME)  \
    if (!params.ignoreOriginal)                  \
    {                                            \
        Super::METHOD_NAME();                    \
    }

#define CORONA_OBJECTS_METHOD_CORE_WITH_RESULT(METHOD_NAME)  \
    if (!params.ignoreOriginal)                              \
    {                                                        \
        result = Super::METHOD_NAME();                       \
    }

#define CORONA_OBJECTS_METHOD_CORE_WITH_ARGS(METHOD_NAME, ...)   \
    if (!params.ignoreOriginal)                                  \
    {                                                            \
        Super::METHOD_NAME( __VA_ARGS__ );                       \
    }

#define CORONA_OBJECTS_METHOD_CORE_WITH_ARGS_AND_RESULT(METHOD_NAME, ...)    \
    if (!params.ignoreOriginal)                                              \
    {                                                                        \
        result = Super::METHOD_NAME( __VA_ARGS__ );                          \
    }

#define CORONA_OBJECTS_METHOD(METHOD_NAME)               \
    CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS )  \
    CORONA_OBJECTS_METHOD_CORE( METHOD_NAME )            \
    CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS )

#define CORONA_OBJECTS_METHOD_STRIP_ARGUMENT(METHOD_NAME, ARGUMENT)  \
    CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS )              \
    CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( METHOD_NAME, ARGUMENT )    \
    CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS )

#define CORONA_OBJECTS_METHOD_WITH_ARGS(METHOD_NAME, ...)               \
    CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, __VA_ARGS__ )    \
    CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( METHOD_NAME, __VA_ARGS__ )    \
    CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, __VA_ARGS__ )

#define CORONA_OBJECTS_EARLY_OUT_IF_APPROPRIATE()           \
    bool expectNonZeroResult = params.earlyOutIfNonZero;    \
                                                            \
    if (params.before && expectNonZeroResult == !!result)   \
    {                                                       \
        return !!result;                                    \
    }

#define CORONA_OBJECTS_METHOD_BEFORE_WITH_BOOLEAN_RESULT(...)     \
    int result = 0;                                               \
                                                                  \
    CORONA_OBJECTS_METHOD_BOOKEND( before, __VA_ARGS__, &result ) \
    CORONA_OBJECTS_EARLY_OUT_IF_APPROPRIATE()

static void
Copy3 (float * dst, const float * src)
{
    static_assert( sizeof( float ) == sizeof( Rtt::Real ), "Incompatible real type" );

    memcpy(dst, src, 3 * sizeof( float ));
}

#define CORONA_OBJECTS_INIT_MATRIX(SOURCE)  \
    Rtt::Real matrix[6];                    \
                                            \
    Copy3( matrix, SOURCE.Row0() );         \
    Copy3( matrix + 3, SOURCE.Row1() )

#define CORONA_OBJECTS_MATRIX_BOOKEND_METHOD(WHEN)  \
    if (params.WHEN)                                \
    {                                               \
        CORONA_OBJECTS_INIT_MATRIX( srcToDst );     \
                                                    \
        params.WHEN( FIRST_ARGS, matrix );          \
                                                    \
        Copy3(const_cast< float * >(srcToDst.Row0()), matrix);        \
        Copy3(const_cast< float * >(srcToDst.Row1()), matrix + 3);    \
    }

#define CORONA_OBJECTS_GET_PARAMS_SPECIFIC(METHOD, NAME) GenericParams gp = {}; /* workaround on Xcode */  \
const auto params = FindParams< PARAMS( NAME ) >( fStream, kAugmentedMethod_##METHOD, sizeof( PARAMS( NAME ) ) - sizeof( gp.NAME.bytes ) )

#define CORONA_OBJECTS_GET_PARAMS(PARAMS_TYPE) CORONA_OBJECTS_GET_PARAMS_SPECIFIC( PARAMS_TYPE, PARAMS_TYPE )

static void *
OnCreate( const void * object, void * userData, const unsigned char * stream )
{
    const auto params = FindParams< CoronaObjectOnCreateParams >( stream, kAugmentedMethod_OnCreate, sizeof( CoronaObjectOnCreateParams ) - sizeof( GenericParams::OnCreate ) );

    if (params.action)
    {
        Rtt::ObjectBoxList list;
        
        OBJECT_BOX_STORE( DisplayObject, storedObject, object );

        params.action( storedObject, &userData );
    }
    
    return userData;
}

// ----------------------------------------------------------------------------

template<typename Base, typename ProxyVTableType, typename... Args>
struct CoronaObjectsInterface : public Base {
    typedef CoronaObjectsInterface Self;
    typedef Base Super;

    CoronaObjectsInterface( Args ... args )
        : Super( args... ),
        fStream( NULL ),
        fUserData( NULL ),
        fRef( LUA_NOREF )
    {
        fStream = sStreamAndUserData.stream; // TODO(?): some overloads can be invoked while the object is still being pushed, thus these awkward globals
                                             // an alternate hack would be for the offending methods to lazily load from the stack...
        fUserData = sStreamAndUserData.userData;

        sStreamAndUserData.stream = NULL;
        sStreamAndUserData.userData = NULL;

        fUserData = OnCreate( this, fUserData, fStream );
    }

    virtual void AddedToParent( lua_State * L, Rtt::GroupObject * parent )
    {
        Rtt::ObjectBoxList list;
        
        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        OBJECT_BOX_STORE( GroupObject, parentStored, parent );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( AddedToParent, Parent );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, L, parentStored );
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( AddedToParent, L, parent );
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, L, parentStored );
    }

    virtual bool CanCull() const
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( CanCull, BooleanResult );
        CORONA_OBJECTS_METHOD_BEFORE_WITH_BOOLEAN_RESULT( FIRST_ARGS )
        CORONA_OBJECTS_METHOD_CORE_WITH_RESULT( CanCull )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, &result )

        return result;
    }

    virtual bool CanHitTest() const
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( CanHitTest, BooleanResult );
        CORONA_OBJECTS_METHOD_BEFORE_WITH_BOOLEAN_RESULT( FIRST_ARGS )
        CORONA_OBJECTS_METHOD_CORE_WITH_RESULT( CanHitTest )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, &result )

        return result;
    }

    virtual void DidMoveOffscreen()
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( DidMoveOffscreen, Basic );
        CORONA_OBJECTS_METHOD( DidMoveOffscreen )
    }

    virtual void DidUpdateTransform( Rtt::Matrix & srcToDst )
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( DidUpdateTransform, Matrix );
        CORONA_OBJECTS_MATRIX_BOOKEND_METHOD( before )
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( DidUpdateTransform, srcToDst )
        CORONA_OBJECTS_MATRIX_BOOKEND_METHOD( after )
    }

    virtual void Draw( Rtt::Renderer & renderer ) const
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        OBJECT_BOX_STORE( Renderer, rendererStored, &renderer );
        CORONA_OBJECTS_GET_PARAMS( Draw );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, rendererStored );
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( Draw, renderer );
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, rendererStored );
    }

    virtual void FinalizeSelf( lua_State * L )
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( OnFinalize );

        if (params.action)
        {
            params.action( FIRST_ARGS );
        }

        lua_unref( L, fRef );

        Super::FinalizeSelf( L );
    }

    virtual void GetSelfBounds( Rtt::Rect & rect ) const
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( GetSelfBounds, RectResult );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, &rect.xMin, &rect.yMin, &rect.xMax, &rect.yMax )
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( GetSelfBounds, rect )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, &rect.xMin, &rect.yMin, &rect.xMax, &rect.yMax )
    }

    virtual void GetSelfBoundsForAnchor( Rtt::Rect & rect ) const
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( GetSelfBoundsForAnchor, RectResult );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, &rect.xMin, &rect.yMin, &rect.xMax, &rect.yMax )
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( GetSelfBoundsForAnchor, rect )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, &rect.xMin, &rect.yMin, &rect.xMax, &rect.yMax )
    }

    virtual bool HitTest( Rtt::Real contentX, Rtt::Real contentY )
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( HitTest, BooleanResultPoint );
        CORONA_OBJECTS_METHOD_BEFORE_WITH_BOOLEAN_RESULT( FIRST_ARGS, contentX, contentY )
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS_AND_RESULT( HitTest, contentX, contentY )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, contentX, contentY, &result )

        return result;
    }

    virtual void Prepare( const Rtt::Display & display )
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( Prepare, Basic );
        CORONA_OBJECTS_METHOD_STRIP_ARGUMENT( Prepare, display )
    }

    virtual void RemovedFromParent( lua_State * L, Rtt::GroupObject * parent )
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        OBJECT_BOX_STORE( GroupObject, parentStored, parent );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( RemovedFromParent, Parent );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, L, parentStored );
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( RemovedFromParent, L, parent );
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, L, parentStored );
    }

    virtual void Rotate( Rtt::Real deltaTheta )
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( Rotate );
        CORONA_OBJECTS_METHOD_WITH_ARGS( Rotate, deltaTheta )
    }

    virtual void Scale( Rtt::Real sx, Rtt::Real sy, bool isNewValue )
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( Scale );
        CORONA_OBJECTS_METHOD_WITH_ARGS( Scale, sx, sy, isNewValue )
    }

    virtual void SendMessage( const char * message, const void * payload, U32 size ) const
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( OnMessage );

        if (params.action)
        {
            params.action( FIRST_ARGS, message, payload, size );
        }
    }

    virtual void Translate( Rtt::Real deltaX, Rtt::Real deltaY )
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( Translate );
        CORONA_OBJECTS_METHOD_WITH_ARGS( Translate, deltaX, deltaY )
    }

    virtual bool UpdateTransform( const Rtt::Matrix & parentToDstSpace )
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_INIT_MATRIX( parentToDstSpace );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( UpdateTransform, BooleanResultMatrix );
        CORONA_OBJECTS_METHOD_BEFORE_WITH_BOOLEAN_RESULT( FIRST_ARGS, matrix )
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS_AND_RESULT( UpdateTransform, parentToDstSpace )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, matrix, &result )

        return result;
    }

    virtual void WillMoveOnscreen()
    {
        Rtt::ObjectBoxList list;

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( WillMoveOnscreen, Basic );
        CORONA_OBJECTS_METHOD( WillMoveOnscreen )
    }

    virtual const Rtt::LuaProxyVTable& ProxyVTable() const { return ProxyVTableType::Constant(); }

    unsigned char * fStream;
    mutable void * fUserData;
    int fRef;
};

// ----------------------------------------------------------------------------

CORONA_API
int CoronaObjectsBuildMethodStream( lua_State * L, const CoronaObjectParamsHeader * head )
{
    if (BuildMethodStream( L, head )) // ...[, stream]
    {
        return luaL_ref( L, LUA_REGISTRYINDEX ); // ...; registry = { ..., [ref] = stream }
    }

    return LUA_REFNIL;
}

// ----------------------------------------------------------------------------

#define CORONA_OBJECTS_CLASS_INTERFACE(OBJECT_KIND) \
    Rtt_CLASS_NO_COPIES( OBJECT_KIND##2 )            \
                                                    \
public:                                        \
    typedef OBJECT_KIND##2 Self;            \
    typedef OBJECT_KIND##Interface Super

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::ShapeObject,
    Proxy2VTable< class Circle2, Rtt::LuaShapeObjectProxyVTable >,
    
    Rtt::ClosedPath *
> CircleInterface;

class Circle2 : public CircleInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Circle );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::ClosedPath * path )
    {
        return Rtt_NEW( allocator, Self( path ) );
    }

protected:
    Circle2( Rtt::ClosedPath * path )
        : Super( path )
    {
    }
};

CORONA_API
int CoronaObjectsPushCircle( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Circle );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::ContainerObject,
    Proxy2VTable< class Container2, Rtt::LuaGroupObjectProxyVTable >,

    Rtt_Allocator *, Rtt::StageObject *, Rtt::Real, Rtt::Real
> ContainerInterface;

class Container2 : public ContainerInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Container );

public:
    static Super::Super *
    New( Rtt_Allocator* allocator, Rtt::StageObject * stageObject, Rtt::Real w, Rtt::Real h )
    {
        return Rtt_NEW( allocator, Container2( allocator, stageObject, w, h ) );
    }

protected:
    Container2( Rtt_Allocator* allocator, Rtt::StageObject * stageObject, Rtt::Real w, Rtt::Real h )
        : Super( allocator, stageObject, w, h )
    {
    }
};

CORONA_API
int CoronaObjectsPushContainer( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Container );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::EmbossedTextObject,
    Proxy2VTable< class EmbossedText2, Rtt::LuaEmbossedTextObjectProxyVTable >,

    Rtt::Display &, const char[], Rtt::PlatformFont *, Rtt::Real, Rtt::Real, const char[]
> EmbossedTextInterface;

class EmbossedText2 : public EmbossedTextInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( EmbossedText );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::Display& display, const char text[], Rtt::PlatformFont *font, Rtt::Real w, Rtt::Real h, const char alignment[] )
    {
        return Rtt_NEW( allocator, EmbossedText2( display, text, font, w, h, alignment ) );
    }

protected:
    EmbossedText2( Rtt::Display& display, const char text[], Rtt::PlatformFont *font, Rtt::Real w, Rtt::Real h, const char alignment[] )
        : Super( display, text, font, w, h, alignment )
    {
    }
};

CORONA_API
int CoronaObjectsPushEmbossedText( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( EmbossedText );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::EmitterObject,
    Proxy2VTable< class Emitter2, Rtt::LuaEmitterObjectProxyVTable >

    /* no args */
> EmitterInterface;

class Emitter2 : public EmitterInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Emitter );

public:
    static Super::Super *
    New( Rtt_Allocator* allocator )
    {
        return Rtt_NEW( allocator, Emitter2 );
    }

protected:
    Emitter2()
        : Super()
    {
    }
};

CORONA_API
int CoronaObjectsPushEmitter( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Emitter );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::GroupObject,
    Proxy2VTable< class Group2, Rtt::LuaGroupObjectProxyVTable >,
    
    Rtt_Allocator *, Rtt::StageObject *
> GroupInterface;

class Group2 : public GroupInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Group );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::StageObject * stageObject )
    {
        return Rtt_NEW( allocator, Self( allocator, NULL ) );
    }

protected:
    Group2( Rtt_Allocator * allocator, Rtt::StageObject * stageObject )
        : Super( allocator, stageObject )
    {
    }

public:
    virtual void DidInsert( bool childParentChanged )
    {
        Rtt::ObjectBoxList list;
        
        OBJECT_BOX_STORE( GroupObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( DidInsert );
        CORONA_OBJECTS_METHOD_WITH_ARGS( DidInsert, childParentChanged )
    }

    virtual void DidRemove()
    {
        Rtt::ObjectBoxList list;
        
        OBJECT_BOX_STORE( GroupObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( DidRemove, GroupBasic );
        CORONA_OBJECTS_METHOD( DidRemove )
    }
};

CORONA_API
int CoronaObjectsPushGroup( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Group );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::ShapeObject,
    Proxy2VTable< class Image2, Rtt::LuaShapeObjectProxyVTable >,
    
    Rtt::RectPath *
> ImageInterface;

class Image2 : public ImageInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Image );

public:
    static Super::Super *
    New( Rtt_Allocator* allocator, Rtt::Real width, Rtt::Real height )
    {
        Rtt::RectPath * path = Rtt::RectPath::NewRect( allocator, width, height );

        return Rtt_NEW( pAllocator, Self( path ) );
    }

protected:
    Image2( Rtt::RectPath * path )
        : Super( path )
    {
    }
};

CORONA_API
int CoronaObjectsPushImage( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Image );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::ShapeObject,
    Proxy2VTable< class ImageRect2, Rtt::LuaShapeObjectProxyVTable >,
    
    Rtt::RectPath *
> ImageRectInterface;

class ImageRect2 : public ImageRectInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( ImageRect );

public:
    static Super::Super *
    New( Rtt_Allocator* allocator, Rtt::Real width, Rtt::Real height )
    {
        Rtt::RectPath * path = Rtt::RectPath::NewRect( allocator, width, height );

        return Rtt_NEW( pAllocator, Self( path ) );
    }

protected:
    ImageRect2( Rtt::RectPath * path )
        : Super( path )
    {
    }
};

CORONA_API
int CoronaObjectsPushImageRect( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( ImageRect );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::LineObject,
    Proxy2VTable< class Line2, Rtt::LuaShapeObjectProxyVTable >,
    
    Rtt::OpenPath *
> LineInterface;

class Line2 : public LineInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Line );

public:
    static Super::Super *
    New( Rtt_Allocator* allocator, Rtt::OpenPath * path )
    {
        return Rtt_NEW( allocator, Line2( path ) );
    }

protected:
    Line2( Rtt::OpenPath * path )
        : Super( path )
    {
    }
};

CORONA_API
int CoronaObjectsPushLine( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Line );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::ShapeObject,
    Proxy2VTable< class Mesh2, Rtt::LuaShapeObjectProxyVTable >,
    
    Rtt::ClosedPath *
> MeshInterface;

class Mesh2 : public MeshInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Mesh );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::ClosedPath * path )
    {
        return Rtt_NEW( allocator, Self( path ) );
    }

protected:
    Mesh2( Rtt::ClosedPath * path )
        : Super( path )
    {
    }
};

CORONA_API
int CoronaObjectsPushMesh( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Mesh );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::ShapeObject,
    Proxy2VTable< class Polygon2, Rtt::LuaShapeObjectProxyVTable >,
    
    Rtt::ClosedPath *
> PolygonInterface;

class Polygon2 : public PolygonInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Polygon );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::ClosedPath * path )
    {
        return Rtt_NEW( allocator, Self( path ) );
    }

protected:
    Polygon2( Rtt::ClosedPath * path )
        : Super( path )
    {
    }
};

CORONA_API
int CoronaObjectsPushPolygon( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Polygon );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::ShapeObject,
    Proxy2VTable< class Rect2, Rtt::LuaShapeObjectProxyVTable >,
    
    Rtt::ClosedPath *
> RectInterface;

class Rect2 : public RectInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Rect );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::Real width, Rtt::Real height )
    {
        Rtt::RectPath * path = Rtt::RectPath::NewRect( allocator, width, height );

        return Rtt_NEW( pAllocator, Self( path ) );
    }

protected:
    Rect2( Rtt::RectPath * path )
        : Super( path )
    {
    }
};

CORONA_API
int CoronaObjectsPushRect( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Rect );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::ShapeObject,
    Proxy2VTable< class RoundedRect2, Rtt::LuaShapeObjectProxyVTable >,
    
    Rtt::ClosedPath *
> RoundedRectInterface;

class RoundedRect2 : public RoundedRectInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( RoundedRect );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::ClosedPath * path )
    {
        return Rtt_NEW( allocator, Self( path ) );
    }

protected:
    RoundedRect2( Rtt::ClosedPath * path )
        : Super( path )
    {
    }
};

CORONA_API
int CoronaObjectsPushRoundedRect( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( RoundedRect );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::SnapshotObject,
    Proxy2VTable< class Snapshot2, Rtt::LuaShapeObjectProxyVTable >,
    
    Rtt_Allocator *, Rtt::Display &, Rtt::Real, Rtt::Real
> SnapshotInterface;

class Snapshot2 : public SnapshotInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Snapshot );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::Display & display, Rtt::Real width, Rtt::Real height )
    {
        return Rtt_NEW( allocator, Self( allocator, display, width, height ) );
    }

protected:
    Snapshot2( Rtt_Allocator * allocator, Rtt::Display & display, Rtt::Real contentW, Rtt::Real contentH )
        : Super( allocator, display, contentW, contentH )
    {
    }
};

CORONA_API
int CoronaObjectsPushSnapshot( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Snapshot );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::SpriteObject,
    Proxy2VTable< class Sprite2, Rtt::LuaSpriteObjectProxyVTable >,

    Rtt::RectPath *, Rtt_Allocator *, const Rtt::AutoPtr< Rtt::ImageSheet > &, Rtt::SpritePlayer &
> SpriteInterface;

class Sprite2 : public SpriteInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Sprite );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::RectPath * path, const Rtt::AutoPtr< Rtt::ImageSheet > & sheet, Rtt::SpritePlayer & player )
    {
        return Rtt_NEW( allocator, Sprite2( path, allocator, sheet, player ) );
    }

protected:
    Sprite2( Rtt::RectPath * path, Rtt_Allocator * allocator, const Rtt::AutoPtr< Rtt::ImageSheet > & sheet, Rtt::SpritePlayer & player )
        : Super( path, allocator, sheet, player )
    {
    }
};

CORONA_API
int CoronaObjectsPushSprite( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Sprite );
}

// ----------------------------------------------------------------------------

typedef CoronaObjectsInterface<
    Rtt::TextObject,
    Proxy2VTable< class Text2, Rtt::LuaTextObjectProxyVTable >,

    Rtt::Display &, const char[], Rtt::PlatformFont *, Rtt::Real, Rtt::Real, const char[]
> TextInterface;

class Text2 : public TextInterface {
    CORONA_OBJECTS_CLASS_INTERFACE( Text );

public:
    static Super::Super *
    New( Rtt_Allocator * allocator, Rtt::Display& display, const char text[], Rtt::PlatformFont *font, Rtt::Real w, Rtt::Real h, const char alignment[] )
    {
        return Rtt_NEW( allocator, Text2( display, text, font, w, h, alignment ) );
    }

protected:
    Text2( Rtt::Display& display, const char text[], Rtt::PlatformFont *font, Rtt::Real w, Rtt::Real h, const char alignment[] )
        : Super( display, text, font, w, h, alignment )
    {
    }
};

// ----------------------------------------------------------------------------

CORONA_API
int CoronaObjectsPushText( lua_State * L, void * userData, const CoronaObjectParams * params )
{
    CORONA_OBJECTS_PUSH( Text );
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaObjectInvalidate( const CoronaDisplayObject * object )
{
    auto * displayObject = OBJECT_BOX_LOAD( DisplayObject, object );

    if (displayObject)
    {
        displayObject->InvalidateDisplay();

        return 1;
    }

    return 0;
}

// ----------------------------------------------------------------------------

CORONA_API
CoronaGroupObject * CoronaObjectGetParent( const CoronaDisplayObject * object )
{
    auto * displayObject = OBJECT_BOX_LOAD( DisplayObject, object );
    
    if (displayObject)
    {
        Rtt::ObjectBoxList & list = *Rtt::ObjectBoxList::GetList( object );
        
        OBJECT_BOX_STORE( GroupObject, parent, displayObject->GetParent() );
        
        return parent;
    }

    return NULL;
}

// ----------------------------------------------------------------------------

CORONA_API
CoronaDisplayObject * CoronaGroupObjectGetChild( const CoronaGroupObject * groupObject, int index )
{
    auto * go = OBJECT_BOX_LOAD( GroupObject, groupObject );

    if (go && index >= 0 && index < go->NumChildren())
    {
        Rtt::ObjectBoxList & list = *Rtt::ObjectBoxList::GetList( groupObject );
        
        OBJECT_BOX_STORE( DisplayObject, child, &go->ChildAt( index ) );

        return child;
    }

    return NULL;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaGroupObjectGetNumChildren( const CoronaGroupObject * groupObject )
{
    auto * go = OBJECT_BOX_LOAD( GroupObject, groupObject );

    return go ? go->NumChildren() : 0;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaObjectSendMessage( const CoronaDisplayObject * object, const char * message, const void * payload, unsigned int size )
{
    auto * displayObject = OBJECT_BOX_LOAD( DisplayObject, object );

    if (displayObject)
    {
        displayObject->SendMessage( message, payload, size );

        return 1;
    }

    return 0;
}

// ----------------------------------------------------------------------------

#undef PARAMS
#undef AFTER_HEADER_STRUCT
#undef FIRST_ARGS
