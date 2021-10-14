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

namespace Rtt
{

// ----------------------------------------------------------------------------

class ObjectBoxList {
public:
    ObjectBoxList();
    ~ObjectBoxList();

public:
    struct Box {
        void * fList;
        void * fObject;
        int fType;
    };

    
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
    static const char * StringForType (int type);
    
public:
    static bool CanGetObject ( const Box * box, int type);
    
public:
    static ObjectBoxList * GetList( const Box * box );
    static bool CheckObject( const Box * box, int type );
    static void * GetObject( const Box * box, int type );
    
public:
    Box * Add( const void * object, int type );
    
private:
    std::list< Box > fBoxes;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

struct CoronaRenderer : Rtt::ObjectBoxList::Box {};
struct CoronaRenderData : Rtt::ObjectBoxList::Box {};
struct CoronaShader : Rtt::ObjectBoxList::Box {};
struct CoronaShaderData : Rtt::ObjectBoxList::Box {};
struct CoronaDisplayObject : Rtt::ObjectBoxList::Box {};
struct CoronaGroupObject : Rtt::ObjectBoxList::Box {};

template<typename T> Rtt_INLINE T * ObjectBoxCast( void * ptr ) { return (T *)ptr; }
template<typename T> Rtt_INLINE const T * ObjectBoxCast( const void * ptr ) { return (const T *)ptr; }

#define OBJECT_BOX_STORE(TYPE, NAME, OBJECT) auto * NAME = ObjectBoxCast< Corona##TYPE >( list.Add( OBJECT, Rtt::ObjectBoxList::k##TYPE ) )
#define OBJECT_BOX_LOAD(TYPE, BOX) ObjectBoxCast< Rtt::TYPE >( Rtt::ObjectBoxList::GetObject( BOX, Rtt::ObjectBoxList::k##TYPE ) )

// ----------------------------------------------------------------------------

#endif // _Rtt_ObjectBoxList_h__
