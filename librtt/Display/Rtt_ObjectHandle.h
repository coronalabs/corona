//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_ObjectHandle_h__
#define _Rtt_ObjectHandle_h__

// ----------------------------------------------------------------------------

#include "Core/Rtt_Types.h"
#include "Corona/CoronaPublicTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------
    
class ObjectHandleScope;

// N.B. this type is only used as a pointer, never instantiated; this
// provides some basic C type checking. The value is really a handle,
// comprising a hash and two indices rather than an address
class ObjectHandleDummy {
    public:
        class TypeNode {
            public:
                TypeNode( const char* name );

                void Init( const TypeNode* parent );
                bool IsCompatible( const TypeNode* other ) const;
                const TypeNode* Correct( const void* object ) const;

                const char* GetName() const { return fName; }
                bool IsInitialized() const { return fInitialized; }

            private:
                const TypeNode* fNext;
                const TypeNode* fParent;
                const char* fName;
                bool fInitialized;

                static TypeNode* sHead;
        };

    private:
        ObjectHandleDummy() = delete;
        ObjectHandleDummy( const ObjectHandleDummy& ) = delete;

    private:
        int unused;
};

template< typename T >
const ObjectHandleDummy::TypeNode*
GetParentType()
{
    return NULL;
}

template< typename T >
struct TypedDummy : public ObjectHandleDummy {
    static TypeNode*
    GetType()
    {
        if ( !sNode.IsInitialized() )
        {
           sNode.Init( GetParentType< T >() );
        }

        return &sNode;
    }

    static TypeNode sNode;
};

class ObjectHandleScope {
    public:
        using Self = ObjectHandleScope;
        using TypeNode = ObjectHandleDummy::TypeNode;

        enum {
            kTotal = 8
        };

    public:
        ObjectHandleScope(); // n.b. does not become current until first Add()
        ~ObjectHandleScope();
        
    public:
        bool Set( const ObjectHandleDummy* handle, const void* object, const TypeNode* type );

    public:
        const ObjectHandleDummy* Add( const void* object, const TypeNode* type );
        const ObjectHandleDummy* GetFreeSlot();
        static void* Extract( const ObjectHandleDummy* handle, const TypeNode* type );

    public:
        struct Box {
            enum {
                kDynamicBit = 0x1
            };

            const TypeNode* fType;
            void* fObject;

            uintptr_t AsUint() const { return reinterpret_cast< uintptr_t >( this ); }

            void Set( const void* object, const TypeNode* type )
            {
                fObject = const_cast< void* >( object );
                fType = type->Correct( object );
            }
        };

    public:
        static Self* Current() { return sCurrent; }

        bool Initialized() const { return 0 != fHash; }
        bool OwnsHandle( const ObjectHandleDummy* handle, const Box** box ) const;
        const TypeNode* Contains( const void* object ) const;

    public:
        class ClearIf {
            public:
                ClearIf( Self& scope, bool clear )
                :   fScope( scope ),
                    fClear( clear )
                {
                }

                ~ClearIf()
                {
                    if ( fClear )
                    {
                        fScope.Clear();
                    }
                }

            private:
                Self& fScope;
                bool fClear;
        };

    private:
        void Init();
        void Clear() { fBoxesUsed = 0; }

        Box* FindFreeBox();
        Box* FindBoxFromHandle( const uintptr_t& uintHandle ) const;

        const ObjectHandleDummy* ToHandle( const uintptr_t& uint );

        uintptr_t Mix( const uintptr_t& uint ) const { return uint ^ fHash; }

    private:
        static Self* sCurrent;

    private:
        Self* fPrevious;
        Box fBoxes[ kTotal ];
        uintptr_t fHash;
        U32 fBoxesUsed;
};

static const ObjectHandleDummy::TypeNode*
GetDisplayObjectType();

// ----------------------------------------------------------------------------

} // namespace Rtt

#define OBJECT_HANDLE_GET_TYPE( TYPE ) Rtt::TypedDummy< Rtt::TYPE >::GetType()

#define AUX_OBJECT_HANDLE_DEFINE_TYPE( TYPE, DUMMY_TYPE )   struct Corona##TYPE : public Rtt::TypedDummy< DUMMY_TYPE > {}; \
                                                            Rtt::ObjectHandleDummy::TypeNode Corona##TYPE::sNode( #TYPE )

#define OBJECT_HANDLE_DEFINE_TYPE( TYPE ) AUX_OBJECT_HANDLE_DEFINE_TYPE( TYPE, Rtt::TYPE )

// ----------------------------------------------------------------------------

#define OBJECT_HANDLE_SCOPE()   int allStored = 1; \
                                Rtt::ObjectHandleScope handleScope

#define OBJECT_HANDLE_SCOPE_EXISTING()  Rtt::ObjectHandleScope* currentScope = Rtt::ObjectHandleScope::Current(); \
                                        Rtt_ASSERT( currentScope ); \
                                        Rtt::ObjectHandleScope& handleScope = *currentScope

#define OBJECT_HANDLE_STORE( TYPE, NAME, OBJECT )   Rtt_ASSERT( !handleScope.Initialized() || Rtt::ObjectHandleScope::Current() == &handleScope ); \
                                                    const auto* NAME = reinterpret_cast< const Corona##TYPE* >( handleScope.Add( OBJECT, OBJECT_HANDLE_GET_TYPE( TYPE ) ) ); \
                                                    allStored = allStored &= NULL != NAME

#define OBJECT_HANDLE_STORE_EXTERNAL( TYPE, NAME, OBJECT, REF ) handleScope.Set( REF, OBJECT, OBJECT_HANDLE_GET_TYPE( TYPE ) )

#define OBJECT_HANDLE_LOAD( TYPE, HANDLE ) static_cast< Rtt::TYPE* >( Rtt::ObjectHandleScope::Extract( HANDLE, OBJECT_HANDLE_GET_TYPE( TYPE ) ) )

// ----------------------------------------------------------------------------

#endif // _Rtt_ObjectHandle_h__
