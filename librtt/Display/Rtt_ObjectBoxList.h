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
//#include "Core/Rtt_Macros.h"
//#include <list>

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
        enum { kNoFreeSlot = U16( ~0 ) };

        ObjectBoxScope();

    public:
        void UpdateCodingFactors();
        void UpdateHash();

        U64 GetHash() const { return fHash; }
        UPtr Encode( const void* object ) const;
        void* Decode( const UPtr& encoded ) const;

        U16 GetNextFreeSlot() const { return fNextFreeSlot; }
        void SetNextFreeSlot( U16 slot ) { fNextFreeSlot = slot; }

    private:
        U64 fIndex;
        U64 fOffset;
        U64 fHash;
        U64 fSeed;
        UPtr fXor1;
        UPtr fXor2;
        U16 fNextFreeSlot;
        U8 fShift1;
        U8 fShift2;
};

class ObjectBoxScopeView {
    public:
        ObjectBoxScopeView();
        ~ObjectBoxScopeView();

    public:
        ObjectBoxScope* GetScope() const { return fScope; }
        U16 GetSlot() const { return fSlot; }

    private:
        ObjectBoxScope* fScope;
        U16 fSlot;
};

class ObjectBox {
    public:
        enum {
            kRenderer,
            kRenderData,
            kShader,
            kShaderData,
            kDisplayObject,
            kGroupObject,
            kCommandBuffer
        };

    public:
        ObjectBox( ObjectBoxScopeView* view, const void* object, int type );
        ObjectBox( ObjectBoxScope* scope, U16 slot, const void* object, int type );

        static void* Extract( const ObjectBox* box, int type, ObjectBoxScope** scope = NULL, U16* scopeSlot = NULL );

    public:
        static const char * StringForType( int type );

    private:
        U64 fStateHash;
        UPtr fObjectHash;
        U16 fSlot;
        U8 fType;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

struct CoronaRenderer : Rtt::ObjectBox {};
struct CoronaRenderData : Rtt::ObjectBox {};
struct CoronaShader : Rtt::ObjectBox {};
struct CoronaShaderData : Rtt::ObjectBox {};
struct CoronaDisplayObject : Rtt::ObjectBox {};
struct CoronaGroupObject : Rtt::ObjectBox {};
struct CoronaCommandBuffer : Rtt::ObjectBox {};

#define OBJECT_BOX_SCOPE() Rtt::ObjectBoxScopeView boxScopeView
#define OBJECT_BOX_SCOPE_EXISTING() Rtt::ObjectBoxScope* scope; U16 scopeSlot
#define OBJECT_BOX_STORE( TYPE, NAME, OBJECT ) Rtt::ObjectBox NAME##_stack( &boxScopeView, OBJECT, Rtt::ObjectBox::k##TYPE ); const auto * NAME = static_cast< Corona##TYPE* >( &NAME##_stack )
#define OBJECT_BOX_STORE_IN_STACK_OBJECT( TYPE, STACK_OBJECT, OBJECT ) new (STACK_OBJECT) Rtt::ObjectBox( scope, scopeSlot, OBJECT, Rtt::ObjectBox::k##TYPE )
#define OBJECT_BOX_LOAD( TYPE, BOX ) static_cast< Rtt::TYPE* >( Rtt::ObjectBox::Extract( BOX, Rtt::ObjectBox::k##TYPE ) )
#define OBJECT_BOX_LOAD_WITH_SCOPE( TYPE, BOX ) static_cast< Rtt::TYPE* >( Rtt::ObjectBox::Extract( BOX, Rtt::ObjectBox::k##TYPE, &scope ) )

// ----------------------------------------------------------------------------

#endif // _Rtt_ObjectBoxList_h__
