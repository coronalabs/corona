//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_ObjectBoxList_h__
#define _Rtt_ObjectBoxList_h__

// ----------------------------------------------------------------------------

#include "Corona/CoronaPublicTypes.h"
#include "Core/Rtt_Macros.h"
#include <list>

// ----------------------------------------------------------------------------

struct CoronaObject {
    void * fList;
    void * fObject;
    int fType;
};

namespace Rtt
{

// ----------------------------------------------------------------------------

class ObjectBoxList {
public:
    ObjectBoxList();
    ~ObjectBoxList();
    
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
    static bool CanGetObject ( const CoronaObject * box, int type);
    
public:
    static ObjectBoxList * GetList( CoronaObject * box );
    static const ObjectBoxList * GetList( const CoronaObject * box );
    static void * GetObject( CoronaObject * box, int type );
    static const void * GetObject( const CoronaObject * box, int type );
    
public:
    CoronaObject * Add( const void * object, int type );
    
private:
    std::list< CoronaObject > fBoxes;
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

template<typename T> Rtt_INLINE T * ObjectBoxCast( void * ptr ) { return (T *)ptr; }
template<typename T> Rtt_INLINE const T * ObjectBoxCast( const void * ptr ) { return (const T *)ptr; }

#define OBJECT_BOX_STORE(TYPE, NAME, OBJECT) auto * NAME = ObjectBoxCast< Corona##TYPE >( list.Add( OBJECT, Rtt::ObjectBoxList::k##TYPE ) )
#define OBJECT_BOX_LOAD(TYPE, BOX) ObjectBoxCast< Rtt::TYPE >( Rtt::ObjectBoxList::GetObject( BOX, Rtt::ObjectBoxList::k##TYPE ) )

// ----------------------------------------------------------------------------

#endif // _Rtt_ObjectBoxList_h__
