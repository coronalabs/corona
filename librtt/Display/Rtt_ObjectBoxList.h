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
#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

template< bool arePointers64Bit = sizeof(void*) == sizeof(U64) > struct PointerSizedInt {
    typedef U32 value;
};

template<> struct PointerSizedInt<true> {
    typedef U64 value;
};

using UPtr = PointerSizedInt<>::value;

class ObjectBoxScope {
    public:
        enum {
            kNoFreeSlot = U16( ~0 ),
            kMaxSlot = kNoFreeSlot - 1 // TODO: limited by how many bits we can split between slot / type in ObjectBox
        };

        ObjectBoxScope();

    public:
        void UpdateCodingFactors();
        void UpdateHash( U16 slot );

        U64 GetHash() const;
        UPtr Encode( const void* object ) const;
        void* Decode( const UPtr& encoded ) const;

        // valid when not in use:
        U16 GetNextFreeSlot() const;

    private:
        U64 fIndex;
        U64 fOffset;
        U64 fHashEx;
        U64 fSeed;
        UPtr fXor1;
        UPtr fXor2;
};

class ObjectBoxScopeView {
    public:
        ObjectBoxScopeView();
        ~ObjectBoxScopeView();

    public:
        U16 GetSlot() const { return fSlot; }

    private:
        U16 fSlot;
};

class ObjectBox {
    public:
        enum {
            kMaxType = 255 // TODO: limited by how many bits we can split between slot / type (compare kMaxSlot)
        };

    public:
        static int Populate( unsigned char buffer[], U16 slot, const void* object, int type );
        static void* Extract( const unsigned char buffer[], int type, U16* scopeSlot = NULL );

    public:
        static int AddType( const char* name, int parent = -1 );
        static const char * StringForType( int type );
};

static int
GetDisplayObjectType();

template< typename T >
bool
GetParentTypeNode( int& parent )
{
    parent = -1;

    return true;
}

template< typename T >
int
GetObjectBoxType( const char* name )
{
    static_assert( sizeof( ObjectBox ) <= sizeof( T::data ), "Unable to interpret type as ObjectBox" );

    static int sType = -1;

    if ( -1 == sType )
    {
       int parent, ok = GetParentTypeNode< T >( parent );

       if ( ok )
       {
           sType = ObjectBox::AddType( name, parent );
       }
    }

    Rtt_ASSERT( Rtt_StringCompare( name, ObjectBox::StringForType( sType ) ) == 0 );

    return sType;
}

#define OBJECT_BOX_LIST_GET_TYPE( TYPE ) Rtt::GetObjectBoxType< Corona##TYPE >( #TYPE )

int
GetDisplayObjectType()
{
    return OBJECT_BOX_LIST_GET_TYPE( DisplayObject );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#define OBJECT_BOX_SCOPE() int allStored = 1; Rtt::ObjectBoxScopeView boxScopeView
#define OBJECT_BOX_SCOPE_EXISTING() int allStored = 1; U16 scopeSlot
#define OBJECT_BOX_STORE( TYPE, NAME, OBJECT ) Corona##TYPE NAME = {}; allStored &= Rtt::ObjectBox::Populate( NAME.data, boxScopeView.GetSlot(), OBJECT, OBJECT_BOX_LIST_GET_TYPE( TYPE ) )
#define OBJECT_BOX_STORE_VIA_POINTER( TYPE, NAME, OBJECT ) allStored &= Rtt::ObjectBox::Populate( NAME->data, scopeSlot, OBJECT, OBJECT_BOX_LIST_GET_TYPE( TYPE ) )
#define OBJECT_BOX_LOAD( TYPE, BOX ) static_cast< Rtt::TYPE* >( Rtt::ObjectBox::Extract( BOX.data, OBJECT_BOX_LIST_GET_TYPE( TYPE ) ) )
#define OBJECT_BOX_LOAD_WITH_SCOPE( TYPE, BOX ) static_cast< Rtt::TYPE* >( Rtt::ObjectBox::Extract( BOX.data, OBJECT_BOX_LIST_GET_TYPE( TYPE ), &scopeSlot ) )

// ----------------------------------------------------------------------------

#endif // _Rtt_ObjectBoxList_h__
