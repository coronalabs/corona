//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_ObjectHandleList_h__
#define _Rtt_ObjectHandleList_h__

// ----------------------------------------------------------------------------

#include "Corona/CoronaPublicTypes.h"
#include "Core/Rtt_Macros.h"
#include <vector>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class ObjectHandleNode {
public:
    ObjectHandleNode( CoronaObject * handle, const void * object, int type );

public:
    enum {
        kRenderer,
        kRenderData,
        kShader,
        kShaderData,
        kDisplayObject,
        kGroupObject
    };
    
public:
    static void * ToObject( CoronaObject * handle, int type );
    static const void * ToObject( const CoronaObject * handle, int type );
    
public:
    bool CanGetObject (int type) const
    {
        return type == fType || (kGroupObject == fType && kDisplayObject == type);
    }
    
private:
    CoronaObject * fHandle;
    void * fObject;
    int fType;
    
    friend class ObjectHandleList;
};

class ObjectHandleList {
public:
    ObjectHandleList();
    ~ObjectHandleList();
  
public:
    static const ObjectHandleNode * ToNode(  const CoronaObject * handle );
    
public:
    void AddHandle( CoronaObject * handle, const void * object, int type );
    
private:
    std::vector< ObjectHandleNode > fNodes;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

struct CoronaRenderer : CoronaObject {};
struct CoronaRenderData : CoronaObject {};
struct CoronaShader : CoronaObject {};
struct CoronaShaderData : CoronaObject {};
struct CoronaDisplayObject : CoronaObject {};
struct CoronaGroupObject : CoronaObject {};

template<typename T> Rtt_INLINE T * ObjectHandleLoadPtr( void * ptr ) { return (T *)ptr; }
template<typename T> Rtt_INLINE const T * ObjectHandleLoadPtr( const void * ptr ) { return (const T *)ptr; }

#define OBJECT_HANDLE_STORE(TYPE, NAME, OBJECT) Corona##TYPE NAME = {}; list.AddHandle( (CoronaObject *)&NAME, OBJECT, Rtt::ObjectHandleNode::k##TYPE )
#define OBJECT_HANDLE_LOAD(TYPE, HANDLE) ObjectHandleLoadPtr< Rtt::TYPE >( Rtt::ObjectHandleNode::ToObject( HANDLE, Rtt::ObjectHandleNode::k##TYPE ) )

// ----------------------------------------------------------------------------

#endif // _Rtt_ObjectHandleList_h__
