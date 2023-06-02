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

ObjectHandleDummy::TypeNode* ObjectHandleDummy::TypeNode::sHead;

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
    fBoxesUsed( 0 ),
    fExternalMask( 0 )
{
}
        
ObjectHandleScope::~ObjectHandleScope()
{
    if ( fBoxesUsed > 0 )
    {
        Rtt_ASSERT( this == sCurrent );

        sCurrent = fPrevious;
    }
}

ObjectHandleScope* ObjectHandleScope::sCurrent;

template< bool is64Bit = sizeof(uintptr_t) == 8 >
class HashValue {
    public:
        uintptr_t GetValue() const { return fValue; }

        void Update( const U64& hash )
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

        void Update( const U64& hash )
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
        hv.Update( wyrand( &sSeed ) );
    }

    fHash = hv.GetValue();
    fPrevious = sCurrent;

    sCurrent = this;
}

ObjectHandleScope::Box* 
ObjectHandleScope::FindBox( unsigned int* ref )
{
    if ( NULL != ref && 0 != *ref )
    {
        U32 index = *ref - 1; // correct for "resolved" state

        if ( index < fBoxesUsed && fExternalMask & ( 1 << index ) )
        {
            return &fBoxes[index];
        }

        else
        {
            Rtt_TRACE_SIM(( "ERROR: Provided invalid external reference: %u", *ref ));

            return NULL;
        }
    }

    if ( kTotal == fBoxesUsed )
    {
        return NULL;
    }

    if ( NULL != ref )
    {
        fExternalMask |= 1 << fBoxesUsed;

        *ref = fBoxesUsed + 1; // add 1 since 0 means unresolved
    }

    return &fBoxes[ fBoxesUsed++ ];
}
  
const ObjectHandleDummy*
ObjectHandleScope::Add( const void* object, const ObjectHandleDummy::TypeNode* type, unsigned int* externalRef )
{
    Rtt_ASSERT( type );
    Rtt_ASSERT( object );

    if ( 0 == fHash )
    {
        Init();
    }

    Box* box = FindBox( externalRef );

    if ( NULL == box )
    {
        return NULL;
    }

    box->fObject = const_cast< void* >( object );
    box->fType = type->Correct( object );

    uintptr_t uintBox = reinterpret_cast< uintptr_t >( box ) ^ fHash;

    return reinterpret_cast< ObjectHandleDummy* >( uintBox );
}

void*
ObjectHandleScope::Extract( const ObjectHandleDummy* handle, const ObjectHandleDummy::TypeNode* node )
{
    Rtt_ASSERT( node && node->IsInitialized() );

    Box* box = NULL;

    for ( ObjectHandleScope* scope = sCurrent; scope && !scope->OwnsHandle( handle, &box ); scope = scope->fPrevious ) {}

    if ( NULL == box )
    {
        Rtt_TRACE_SIM(( "ERROR: Handle for %s object not found in current scope", node->GetName() ));

        return NULL;
    }

    if ( !node->IsCompatible( box->fType ) )
    {
        Rtt_TRACE_SIM(( "ERROR: Expected %s but have %s", node->GetName(), box->fType->GetName() ));

        return NULL;
    }

    return box->fObject;
}
        
bool
ObjectHandleScope::OwnsHandle( const ObjectHandleDummy* handle, Box** box ) const
{
    uintptr_t uintBox = reinterpret_cast< uintptr_t >( handle ) ^ fHash;
    Box* pBox = reinterpret_cast< Box* >( uintBox );

    if ( pBox >= fBoxes && pBox <= &fBoxes[kTotal] )
    {
        *box = pBox;

        return true;
    }

    return false;
}

const ObjectHandleDummy::TypeNode*
ObjectHandleScope::Contains( const void* object ) const
{
    for ( int i = 0; i < kTotal; i++ )
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