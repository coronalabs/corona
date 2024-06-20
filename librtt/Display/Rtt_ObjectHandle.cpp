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
#include "Core/Rtt_Time.h"
#include "Display/Rtt_ObjectHandle.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_GroupObject.h"

#include "../../external/wyhash/wyhash.h"

// ----------------------------------------------------------------------------

AUX_OBJECT_HANDLE_DEFINE_TYPE( Any, void );

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ObjectHandleDummy::TypeNode::TypeNode( const char* name )
:   fNext( NULL ),
    fParent( NULL ),
    fName( name ),
    fInitialized( false )
{
}

ObjectHandleDummy::TypeNode*
ObjectHandleDummy::TypeNode::sHead;

void
ObjectHandleDummy::TypeNode::Init( const ObjectHandleDummy::TypeNode* parent )
{
    fNext = sHead;
    fParent = parent;
    fInitialized = true;

    sHead = this;
}

bool 
ObjectHandleDummy::TypeNode::IsCompatible( const ObjectHandleDummy::TypeNode* other ) const
{   
    if ( NULL == other )
    {
        return true;
    }

    for ( const TypeNode* cur = this; cur; cur = cur->fParent )
    {
        if ( cur == other )
        {
            return true;
        }
    }

    return false;
}

const ObjectHandleDummy::TypeNode*
ObjectHandleDummy::TypeNode::Correct( const void* object ) const
{
    if ( GetDisplayObjectType() == this )
    {
        const GroupObject* group = static_cast< const DisplayObject* >( object )->AsGroupObject();

        if ( NULL != group )
        {
            return OBJECT_HANDLE_GET_TYPE( GroupObject );
        }
    }

    return this;
}

ObjectHandleScope::ObjectHandleScope()
:   fPrevious( NULL),
    fHash( 0 ),
    fBoxesUsed( 0 )
{
}
        
ObjectHandleScope::~ObjectHandleScope()
{
    Clear();
}

ObjectHandleScope*
ObjectHandleScope::sCurrent;

template< bool is64Bit = sizeof(uintptr_t) == 8 >
class HashValue {
    public:
        uintptr_t GetValue() const { return fValue; }

        void operator=( const U64& hash )
        {
            fValue = hash;
        }

    private:
        U64 fValue = 0;
};

template<>
class HashValue< false > {
    public:
        ~HashValue()
        {
            GetHash() >>= 32;
        }

        uintptr_t GetValue() const
        {
            return static_cast< uintptr_t >( GetHash() );
        }

        void operator=( const U64& hash )
        {
            GetHash() = hash;
        }

    private:
        U64& GetHash() const
        {
            static U64 sHash;

            return sHash;
        }
};

void
ObjectHandleScope::Init()
{
    static U64 sSeed;

    while ( 0 == sSeed )
    {
        sSeed = Rtt_GetAbsoluteTime();
    }

    HashValue<> hv;

    while ( 0 == hv.GetValue() )
    {
        hv = wyrand( &sSeed );
    }

    fHash = hv.GetValue();
    fPrevious = sCurrent;

    sCurrent = this;
}

void
ObjectHandleScope::Clear()
{
    if ( Initialized() )
    {
        Rtt_ASSERT( this == sCurrent );

        sCurrent = fPrevious;
    }

    fPrevious = NULL;
    fHash = 0;
    fBoxesUsed = 0;
}

ObjectHandleScope::Box* 
ObjectHandleScope::FindFreeBox()
{
    if ( !Initialized() )
    {
        Init();
    }

    if ( kTotal == fBoxesUsed )
    {
        return NULL;
    }

    return &fBoxes[ fBoxesUsed++ ];
}

static uintptr_t
HandleToUint( const ObjectHandleDummy* handle )
{
    return reinterpret_cast< uintptr_t >( handle );
}

ObjectHandleScope::Box*
ObjectHandleScope::FindBoxFromHandle( const uintptr_t& uintHandle ) const
{
    Box* box = reinterpret_cast< Box* >( uintHandle & ~Box::kDynamicBit );

    if ( box >= fBoxes && box < fBoxes + fBoxesUsed )
    {
        return box;
    }

    else
    {
        return NULL;
    }
}

const ObjectHandleDummy*
ObjectHandleScope::ToHandle( const uintptr_t& uint )
{
    return reinterpret_cast< ObjectHandleDummy* >( uint ^ fHash );
}

bool
ObjectHandleScope::Set( const ObjectHandleDummy* handle, const void* object, const TypeNode* type )
{
    Rtt_ASSERT( type && type->IsInitialized() );

    uintptr_t uintHandle = Mix( HandleToUint( handle ) );

    if ( 0 == ( uintHandle & Box::kDynamicBit ) )
    {
        Rtt_TRACE_SIM(( "ERROR: Slot for `%s` not dynamic", type->GetName() ));

        return false;
    }

    Box* box = FindBoxFromHandle( uintHandle );

    if ( NULL == box )
    {
        Rtt_TRACE_SIM(( "ERROR: Handle for dynamic `%s` not found in any scope", type->GetName() ));

        return false;
    }

    box->Set( object, type );

    return true;
}

const ObjectHandleDummy*
ObjectHandleScope::Add( const void* object, const ObjectHandleDummy::TypeNode* type )
{
    Rtt_ASSERT( type );
    Rtt_ASSERT( object );

    Box* box = FindFreeBox();

    if ( NULL == box )
    {
        return NULL;
    }

    box->Set( object, type );

    return ToHandle( box->AsUint() );
}

const ObjectHandleDummy*
ObjectHandleScope::GetFreeSlot()
{
    Box* box = FindFreeBox();

    if ( NULL == box )
    {
        return NULL;
    }

    static_assert( alignof(Box) > 1, "Box assumed to be have alignment >= 2" );

    box->Set( NULL, NULL );

    return ToHandle( box->AsUint() | Box::kDynamicBit );
}

void*
ObjectHandleScope::Extract( const ObjectHandleDummy* handle, const ObjectHandleDummy::TypeNode* node )
{
    Rtt_ASSERT( node && node->IsInitialized() );

    uintptr_t uintHandle = HandleToUint( handle );
    const Box* box = NULL;

    for ( const Self* scope = sCurrent; scope && !box; scope = scope->fPrevious )
    {
        box = scope->FindBoxFromHandle( scope->Mix( uintHandle ) );
    }

    if ( NULL == box )
    {
        Rtt_TRACE_SIM(( "ERROR: Handle for `%s` not found in any scope", node->GetName() ));

        return NULL;
    }

    if ( !node->IsCompatible( box->fType ) )
    {
        Rtt_TRACE_SIM(( "ERROR: Expected `%s` but have `%s`", node->GetName(), box->fType->GetName() ));

        return NULL;
    }

    return box->fObject;
}

const ObjectHandleDummy::TypeNode*
ObjectHandleScope::Contains( const void* object ) const
{
    for ( int i = 0; i < fBoxesUsed; i++ )
    {
        if ( fBoxes[i].fObject == object )
        {
            return fBoxes[i].fType;
        }
    }

    return NULL;
}

const ObjectHandleDummy::TypeNode*
GetDisplayObjectType()
{
    return OBJECT_HANDLE_GET_TYPE( DisplayObject );
}

template<>
const ObjectHandleDummy::TypeNode*
GetParentType< CoronaGroupObject >()
{
    return GetDisplayObjectType();
}

// ----------------------------------------------------------------------------

} // namespace Rtt