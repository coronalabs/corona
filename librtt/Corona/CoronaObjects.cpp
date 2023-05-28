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

// This is free and unencumbered software released into the public domain under The Unlicense (http://unlicense.org/)
// main repo: https://github.com/wangyi-fudan/wyhash
// author: ?? Wang Yi <godspeed_china@yeah.net>
// contributors: Reini Urban, Dietrich Epp, Joshua Haberman, Tommy Ettinger, Daniel Lemire, Otmar Ertl, cocowalla, leo-yuriev, Diego Barrios Romero, paulie-g, dumblob, Yann Collet, ivte-ms, hyb, James Z.M. Gao, easyaspi314 (Devin), TheOneric

/* quick example:
   string s="fjsakfdsjkf";
   uint64_t hash=wyhash(s.c_str(), s.size(), 0, _wyp);
*/

#ifndef wyhash_final_version_4
#define wyhash_final_version_4

#ifndef WYHASH_CONDOM
//protections that produce different results:
//1: normal valid behavior
//2: extra protection against entropy loss (probability=2^-63), aka. "blind multiplication"
#define WYHASH_CONDOM 1
#endif

#ifndef WYHASH_32BIT_MUM
//0: normal version, slow on 32 bit systems
//1: faster on 32 bit systems but produces different results, incompatible with wy2u0k function
#define WYHASH_32BIT_MUM 0  
#endif

//includes
#include <stdint.h>
#include <string.h>
#if defined(_MSC_VER) && defined(_M_X64)
  #include <intrin.h>
  #pragma intrinsic(_umul128)
#endif

//likely and unlikely macros
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
  #define _likely_(x)  __builtin_expect(x,1)
  #define _unlikely_(x)  __builtin_expect(x,0)
#else
  #define _likely_(x) (x)
  #define _unlikely_(x) (x)
#endif

//128bit multiply function
static inline uint64_t _wyrot(uint64_t x) { return (x>>32)|(x<<32); }
static inline void _wymum(uint64_t *A, uint64_t *B){
#if(WYHASH_32BIT_MUM)
  uint64_t hh=(*A>>32)*(*B>>32), hl=(*A>>32)*(uint32_t)*B, lh=(uint32_t)*A*(*B>>32), ll=(uint64_t)(uint32_t)*A*(uint32_t)*B;
  #if(WYHASH_CONDOM>1)
  *A^=_wyrot(hl)^hh; *B^=_wyrot(lh)^ll;
  #else
  *A=_wyrot(hl)^hh; *B=_wyrot(lh)^ll;
  #endif
#elif defined(__SIZEOF_INT128__)
  __uint128_t r=*A; r*=*B; 
  #if(WYHASH_CONDOM>1)
  *A^=(uint64_t)r; *B^=(uint64_t)(r>>64);
  #else
  *A=(uint64_t)r; *B=(uint64_t)(r>>64);
  #endif
#elif defined(_MSC_VER) && defined(_M_X64)
  #if(WYHASH_CONDOM>1)
  uint64_t  a,  b;
  a=_umul128(*A,*B,&b);
  *A^=a;  *B^=b;
  #else
  *A=_umul128(*A,*B,B);
  #endif
#else
  uint64_t ha=*A>>32, hb=*B>>32, la=(uint32_t)*A, lb=(uint32_t)*B, hi, lo;
  uint64_t rh=ha*hb, rm0=ha*lb, rm1=hb*la, rl=la*lb, t=rl+(rm0<<32), c=t<rl;
  lo=t+(rm1<<32); c+=lo<t; hi=rh+(rm0>>32)+(rm1>>32)+c;
  #if(WYHASH_CONDOM>1)
  *A^=lo;  *B^=hi;
  #else
  *A=lo;  *B=hi;
  #endif
#endif
}

//multiply and xor mix function, aka MUM
static inline uint64_t _wymix(uint64_t A, uint64_t B){ _wymum(&A,&B); return A^B; }

//endian macros
#ifndef WYHASH_LITTLE_ENDIAN
  #if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    #define WYHASH_LITTLE_ENDIAN 1
  #elif defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    #define WYHASH_LITTLE_ENDIAN 0
  #else
    #warning could not determine endianness! Falling back to little endian.
    #define WYHASH_LITTLE_ENDIAN 1
  #endif
#endif

//read functions
#if (WYHASH_LITTLE_ENDIAN)
static inline uint64_t _wyr8(const uint8_t *p) { uint64_t v; memcpy(&v, p, 8); return v;}
static inline uint64_t _wyr4(const uint8_t *p) { uint32_t v; memcpy(&v, p, 4); return v;}
#elif defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
static inline uint64_t _wyr8(const uint8_t *p) { uint64_t v; memcpy(&v, p, 8); return __builtin_bswap64(v);}
static inline uint64_t _wyr4(const uint8_t *p) { uint32_t v; memcpy(&v, p, 4); return __builtin_bswap32(v);}
#elif defined(_MSC_VER)
static inline uint64_t _wyr8(const uint8_t *p) { uint64_t v; memcpy(&v, p, 8); return _byteswap_uint64(v);}
static inline uint64_t _wyr4(const uint8_t *p) { uint32_t v; memcpy(&v, p, 4); return _byteswap_ulong(v);}
#else
static inline uint64_t _wyr8(const uint8_t *p) {
  uint64_t v; memcpy(&v, p, 8);
  return (((v >> 56) & 0xff)| ((v >> 40) & 0xff00)| ((v >> 24) & 0xff0000)| ((v >>  8) & 0xff000000)| ((v <<  8) & 0xff00000000)| ((v << 24) & 0xff0000000000)| ((v << 40) & 0xff000000000000)| ((v << 56) & 0xff00000000000000));
}
static inline uint64_t _wyr4(const uint8_t *p) {
  uint32_t v; memcpy(&v, p, 4);
  return (((v >> 24) & 0xff)| ((v >>  8) & 0xff00)| ((v <<  8) & 0xff0000)| ((v << 24) & 0xff000000));
}
#endif
static inline uint64_t _wyr3(const uint8_t *p, size_t k) { return (((uint64_t)p[0])<<16)|(((uint64_t)p[k>>1])<<8)|p[k-1];}
//wyhash main function
static inline uint64_t wyhash(const void *key, size_t len, uint64_t seed, const uint64_t *secret){
  const uint8_t *p=(const uint8_t *)key; seed^=_wymix(seed^secret[0],secret[1]);	uint64_t	a,	b;
  if(_likely_(len<=16)){
    if(_likely_(len>=4)){ a=(_wyr4(p)<<32)|_wyr4(p+((len>>3)<<2)); b=(_wyr4(p+len-4)<<32)|_wyr4(p+len-4-((len>>3)<<2)); }
    else if(_likely_(len>0)){ a=_wyr3(p,len); b=0;}
    else a=b=0;
  }
  else{
    size_t i=len; 
    if(_unlikely_(i>48)){
      uint64_t see1=seed, see2=seed;
      do{
        seed=_wymix(_wyr8(p)^secret[1],_wyr8(p+8)^seed);
        see1=_wymix(_wyr8(p+16)^secret[2],_wyr8(p+24)^see1);
        see2=_wymix(_wyr8(p+32)^secret[3],_wyr8(p+40)^see2);
        p+=48; i-=48;
      }while(_likely_(i>48));
      seed^=see1^see2;
    }
    while(_unlikely_(i>16)){  seed=_wymix(_wyr8(p)^secret[1],_wyr8(p+8)^seed);  i-=16; p+=16;  }
    a=_wyr8(p+i-16);  b=_wyr8(p+i-8);
  }
  a^=secret[1]; b^=seed;  _wymum(&a,&b);
  return  _wymix(a^secret[0]^len,b^secret[1]);
}

//the default secret parameters
static const uint64_t _wyp[4] = {0xa0761d6478bd642full, 0xe7037ed1a0b428dbull, 0x8ebc6af09c88c6e3ull, 0x589965cc75374cc3ull};

//a useful 64bit-64bit mix function to produce deterministic pseudo random numbers that can pass BigCrush and PractRand
static inline uint64_t wyhash64(uint64_t A, uint64_t B){ A^=0xa0761d6478bd642full; B^=0xe7037ed1a0b428dbull; _wymum(&A,&B); return _wymix(A^0xa0761d6478bd642full,B^0xe7037ed1a0b428dbull);}

//The wyrand PRNG that pass BigCrush and PractRand
static inline uint64_t wyrand(uint64_t *seed){ *seed+=0xa0761d6478bd642full; return _wymix(*seed,*seed^0xe7037ed1a0b428dbull);}

//convert any 64 bit pseudo random numbers to uniform distribution [0,1). It can be combined with wyrand, wyhash64 or wyhash.
static inline double wy2u01(uint64_t r){ const double _wynorm=1.0/(1ull<<52); return (r>>12)*_wynorm;}

//convert any 64 bit pseudo random numbers to APPROXIMATE Gaussian distribution. It can be combined with wyrand, wyhash64 or wyhash.
static inline double wy2gau(uint64_t r){ const double _wynorm=1.0/(1ull<<20); return ((r&0x1fffff)+((r>>21)&0x1fffff)+((r>>42)&0x1fffff))*_wynorm-3.0;}

#ifdef	WYTRNG
#include <sys/time.h>
//The wytrand true random number generator, passed BigCrush.
static inline uint64_t wytrand(uint64_t *seed){
	struct	timeval	t;	gettimeofday(&t,0);
	uint64_t	teed=(((uint64_t)t.tv_sec)<<32)|t.tv_usec;
	teed=_wymix(teed^_wyp[0],*seed^_wyp[1]);
	*seed=_wymix(teed^_wyp[0],_wyp[2]);
	return _wymix(*seed,*seed^_wyp[3]);
}
#endif

#if(!WYHASH_32BIT_MUM)
//fast range integer random number generation on [0,k) credit to Daniel Lemire. May not work when WYHASH_32BIT_MUM=1. It can be combined with wyrand, wyhash64 or wyhash.
static inline uint64_t wy2u0k(uint64_t r, uint64_t k){ _wymum(&r,&k); return k; }
#endif

//make your own secret
static inline void make_secret(uint64_t seed, uint64_t *secret){
  uint8_t c[] = {15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89, 90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120, 135, 139, 141, 142, 147, 149, 150, 153, 154, 156, 163, 165, 166, 169, 170, 172, 177, 178, 180, 184, 195, 197, 198, 201, 202, 204, 209, 210, 212, 216, 225, 226, 228, 232, 240 };
  for(size_t i=0;i<4;i++){
    uint8_t ok;
    do{
      ok=1; secret[i]=0;
      for(size_t j=0;j<64;j+=8) secret[i]|=((uint64_t)c[wyrand(&seed)%sizeof(c)])<<j;
      if(secret[i]%2==0){ ok=0; continue; }
      for(size_t j=0;j<i;j++) {
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
        if(__builtin_popcountll(secret[j]^secret[i])!=32){ ok=0; break; }
#elif defined(_MSC_VER) && defined(_M_X64)
        if(_mm_popcnt_u64(secret[j]^secret[i])!=32){ ok=0; break; }
#else
        //manual popcount
        uint64_t x = secret[j]^secret[i];
        x -= (x >> 1) & 0x5555555555555555;
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
        x = (x * 0x0101010101010101) >> 56;
        if(x!=32){ ok=0; break; }
#endif
      }
    }while(!ok);
  }
}

#endif

/* The Unlicense
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ObjectBoxScope::ObjectBoxScope()
:   fIndex( 0 ),
    fXor1( 0 ),
    fXor2( 0 )
{
    fOffset = Rtt_GetAbsoluteTime();
    fSeed = wyhash64( fOffset, (U64)this );

    UpdateHash( kNoFreeSlot );
}

const U8 kNumBits = sizeof(UPtr) * 8;
const U8 kHalfNumBits = kNumBits / 2;

template<typename U> void
GetXorValues( U& xor1, U& xor2, U64* seed )
{
    U64 result = wyrand( seed );

    xor1 = (U32)result;
    xor2 = result >> 32;
}

template<> void
GetXorValues<U64>( U64& xor1, U64& xor2, U64* seed )
{
    xor1 = wyrand( seed );
    xor2 = wyrand( seed );
}

const U64 kOmitHashMask = (U64)ObjectBoxScope::kNoFreeSlot;
const U64 kKeepHashMask = ~kOmitHashMask;

void
ObjectBoxScope::UpdateCodingFactors()
{
    GetXorValues( fXor1, fXor2, &fSeed );

    U16 shift1 = fXor1 % kHalfNumBits;
    U16 shift2 = fXor2 % kHalfNumBits;

    fHashEx = ( fHashEx & kKeepHashMask ) | ( shift1 << 8 ) | shift2;
}

void
ObjectBoxScope::UpdateHash( U16 slot )
{
    fHashEx = ( wyhash64( fIndex++, fOffset ) & kKeepHashMask ) | slot;
}

U64
ObjectBoxScope::GetHash() const
{
    return fHashEx & kKeepHashMask;
}

static UPtr
RotateBitsLeft( const UPtr& v, U8 shift )
{
    return (v << shift) | ( v >> ( kNumBits - shift ) );
}

static UPtr
RotateBitsRight( const UPtr& v, U8 shift )
{
    return (v >> shift) | ( v << ( kNumBits - shift ) );
}

UPtr
ObjectBoxScope::Encode( const void* object ) const
{
    U16 shifts = (U16)( fHashEx & kOmitHashMask );
    UPtr p = (UPtr)object;

    p ^= fXor1;
    p = RotateBitsLeft( p, shifts >> 8 );
    p ^= fXor2;
    
    return RotateBitsRight( p, shifts & 0xFF );
}
        
void*
ObjectBoxScope::Decode( const UPtr& encoded ) const
{
    U16 shifts = (U16)( fHashEx & kOmitHashMask );
    UPtr p = RotateBitsLeft( encoded, shifts & 0xFF );

    p ^= fXor2;
    p = RotateBitsRight( p, shifts >> 8 );
    p ^= fXor1;
    
    return (void*)p;
}

U16
ObjectBoxScope::GetNextFreeSlot() const
{
    return (U16)( fHashEx & kOmitHashMask );
}

static std::vector<ObjectBoxScope> sScopes; // only accessed through RAII views, so all "released" by end of frame
static U16 sFirstFreeSlot = ObjectBoxScope::kNoFreeSlot; // n.b. count is assumed to be quite small

ObjectBoxScopeView::ObjectBoxScopeView()
{
    if ( ObjectBoxScope::kNoFreeSlot != sFirstFreeSlot)
    {
        fSlot = sFirstFreeSlot;

        sFirstFreeSlot = sScopes[fSlot].GetNextFreeSlot();
    }
    
    else
    {
        fSlot = sScopes.size();

        sScopes.emplace_back();
    }

    sScopes[fSlot].UpdateCodingFactors();
}
        
ObjectBoxScopeView::~ObjectBoxScopeView()
{
    ObjectBoxScope& scope = sScopes[fSlot];

    scope.UpdateHash( sFirstFreeSlot ); // invalidate objects

    sFirstFreeSlot = fSlot;
}

struct TypeNode {
    const char* fName;
    int fParent;
};

static std::vector< TypeNode > sTypes;

static int
CorrectType( const void* object, int type )
{
    Rtt_ASSERT( type < sTypes.size() );

    if ( GetDisplayObjectType() == type )
    {
        const GroupObject* group = static_cast< const DisplayObject* >( object )->AsGroupObject();

        if ( NULL != group )
        {
            return OBJECT_BOX_LIST_GET_TYPE( GroupObject );
        }
    }

    return type;
}

const U16 kSlotMask = 0xFF;
const int kSlotShift = 8;
const int kTypeMask = ( 1 << kSlotShift ) - 1;

static U16
PackSlotAndType( U16 slot, int type )
{
    return ( ( slot & kSlotMask ) << kSlotShift ) | ( type & kTypeMask );
}

int
ObjectBox::Populate( unsigned char data[], U16 slot, const void* object, int type )
{
    ObjectBox* box = reinterpret_cast< ObjectBox* >( data );

    type = CorrectType( object, type );

    int stored = type >= 0;
    U64 hashEx = ObjectBoxScope::kNoFreeSlot;
    UPtr objectHash = 0;

    if ( stored )
    {
        const ObjectBoxScope& scope = sScopes[slot];

        hashEx = scope.GetHash() | PackSlotAndType( slot, type );
        objectHash = scope.Encode( object );
    }

    memcpy( data, &hashEx, 8 );
    memcpy( data + 8, &objectHash, sizeof(UPtr) );

    return stored;
}

static int
NodeValue( const TypeNode* node )
{
    return (int)( node - sTypes.data() );
}

static bool IsCompatible( const TypeNode* node, int type )
{
    while ( NodeValue( node ) != type )
    {
        if ( node->fParent >= 0 )
        {
            node = &sTypes[node->fParent];
        }

        else
        {
            return false;
        }
    }

    return true;
}

static U16
ExtractSlot( U64 hashEx )
{
    return (U16)( ( hashEx & kOmitHashMask ) >> kSlotShift );
}

static int
ExtractType( U64 hashEx )
{
    return (int)( hashEx & kTypeMask );
}

void*
ObjectBox::Extract( const unsigned char buffer[], int type, U16* scopeSlot )
{
    if ( type < sTypes.size() )
    {
        Rtt_TRACE_SIM(( "WARNING: Unknown type %i supplied", type ));;

        return NULL;
    }

    U64 hashEx;

    memcpy( &hashEx, buffer, 8 );

    if ( NULL == buffer )
    {
        Rtt_TRACE_SIM(( "WARNING: NULL %s supplied", sTypes[type].fName ));

        return NULL;
    }
    
    TypeNode& node = sTypes[type];
    int boxedType = ExtractType( hashEx );

    if ( !IsCompatible( &node, boxedType ) )
    {
        Rtt_TRACE_SIM(( "WARNING: Expected %s but have %s", sTypes[type].fName, StringForType( boxedType ) ));

        return NULL;
    }

    U16 slot = ExtractSlot( hashEx );

    Rtt_ASSERT( slot < sScopes.size() );

    const ObjectBoxScope& scope = sScopes[slot];

    if ( ( hashEx & kKeepHashMask ) != scope.GetHash() )
    {
        Rtt_TRACE_SIM(( "WARNING: %s object has been invalidated", sTypes[type].fName ));

        return NULL;
    }

    if ( NULL != scopeSlot )
    {
        *scopeSlot = slot;
    }

    UPtr objectHash;

    memcpy( &objectHash, buffer + 8, sizeof(UPtr) );

    return scope.Decode( objectHash );
}

int
ObjectBox::AddType( const char* name, int parent )
{
    if ( ObjectBox::kMaxType == sTypes.size() )
    {
        Rtt_TRACE_SIM(( "ERROR: Unable to box value of type %s (capacity exceeded)", name ));

        return NULL;
    }

    TypeNode node;

    node.fName = name; // n.b. assumed to be constant literal
    node.fParent = parent;

    sTypes.push_back( node );

    return sTypes.size() - 1;
}

const char *
ObjectBox::StringForType (int type)
{
    if ( type >= 0 && type < sTypes.size() )
    {
        return sTypes[type].fName;
    }

    else
    {
        return "Unknown";
    }
}

template<>
bool
GetParentTypeNode< CoronaGroupObject >( int& parent )
{
    parent = GetDisplayObjectType();

    return parent >= 0;
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
        OBJECT_BOX_SCOPE();

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
        OBJECT_BOX_SCOPE();
    
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
        OBJECT_BOX_SCOPE();
        
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
        OBJECT_BOX_SCOPE();

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
CallNewFactory( lua_State * L, const char * name, void * funcBox )
{
    if (PushFactory( L, name ) ) // stream, ...[, factories, factory]
    {
        Rtt::Display & display = Rtt::LuaContext::GetRuntime( L )->GetDisplay();

        display.SetFactoryFunc( funcBox );

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

    if (CallNewFactory( L, name, *(void **)&func )) // stream[, object]
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
        OBJECT_BOX_SCOPE();

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
        OBJECT_BOX_SCOPE();
        
        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        OBJECT_BOX_STORE( GroupObject, parentStored, parent );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( AddedToParent, Parent );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, L, parentStored );
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( AddedToParent, L, parent );
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, L, parentStored );
    }

    virtual bool CanCull() const
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( CanCull, BooleanResult );
        CORONA_OBJECTS_METHOD_BEFORE_WITH_BOOLEAN_RESULT( FIRST_ARGS )
        CORONA_OBJECTS_METHOD_CORE_WITH_RESULT( CanCull )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, &result )

        return result;
    }

    virtual bool CanHitTest() const
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( CanHitTest, BooleanResult );
        CORONA_OBJECTS_METHOD_BEFORE_WITH_BOOLEAN_RESULT( FIRST_ARGS )
        CORONA_OBJECTS_METHOD_CORE_WITH_RESULT( CanHitTest )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, &result )

        return result;
    }

    virtual void DidMoveOffscreen()
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( DidMoveOffscreen, Basic );
        CORONA_OBJECTS_METHOD( DidMoveOffscreen )
    }

    virtual void DidUpdateTransform( Rtt::Matrix & srcToDst )
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( DidUpdateTransform, Matrix );
        CORONA_OBJECTS_MATRIX_BOOKEND_METHOD( before )
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( DidUpdateTransform, srcToDst )
        CORONA_OBJECTS_MATRIX_BOOKEND_METHOD( after )
    }

    virtual void Draw( Rtt::Renderer & renderer ) const
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        OBJECT_BOX_STORE( Renderer, rendererStored, &renderer );
        CORONA_OBJECTS_GET_PARAMS( Draw );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, rendererStored );
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( Draw, renderer );
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, rendererStored );
    }

    virtual void FinalizeSelf( lua_State * L )
    {
        OBJECT_BOX_SCOPE();

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
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( GetSelfBounds, RectResult );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, &rect.xMin, &rect.yMin, &rect.xMax, &rect.yMax )
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( GetSelfBounds, rect )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, &rect.xMin, &rect.yMin, &rect.xMax, &rect.yMax )
    }

    virtual void GetSelfBoundsForAnchor( Rtt::Rect & rect ) const
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( GetSelfBoundsForAnchor, RectResult );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, &rect.xMin, &rect.yMin, &rect.xMax, &rect.yMax )
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( GetSelfBoundsForAnchor, rect )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, &rect.xMin, &rect.yMin, &rect.xMax, &rect.yMax )
    }

    virtual bool HitTest( Rtt::Real contentX, Rtt::Real contentY )
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( HitTest, BooleanResultPoint );
        CORONA_OBJECTS_METHOD_BEFORE_WITH_BOOLEAN_RESULT( FIRST_ARGS, contentX, contentY )
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS_AND_RESULT( HitTest, contentX, contentY )
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, contentX, contentY, &result )

        return result;
    }

    virtual void Prepare( const Rtt::Display & display )
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( Prepare, Basic );
        CORONA_OBJECTS_METHOD_STRIP_ARGUMENT( Prepare, display )
    }

    virtual void RemovedFromParent( lua_State * L, Rtt::GroupObject * parent )
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        OBJECT_BOX_STORE( GroupObject, parentStored, parent );
        CORONA_OBJECTS_GET_PARAMS_SPECIFIC( RemovedFromParent, Parent );
        CORONA_OBJECTS_METHOD_BOOKEND( before, FIRST_ARGS, L, parentStored );
        CORONA_OBJECTS_METHOD_CORE_WITH_ARGS( RemovedFromParent, L, parent );
        CORONA_OBJECTS_METHOD_BOOKEND( after, FIRST_ARGS, L, parentStored );
    }

    virtual void Rotate( Rtt::Real deltaTheta )
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( Rotate );
        CORONA_OBJECTS_METHOD_WITH_ARGS( Rotate, deltaTheta )
    }

    virtual void Scale( Rtt::Real sx, Rtt::Real sy, bool isNewValue )
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( Scale );
        CORONA_OBJECTS_METHOD_WITH_ARGS( Scale, sx, sy, isNewValue )
    }

    virtual void SendMessage( const char * message, const void * payload, U32 size ) const
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( OnMessage );

        if (params.action)
        {
            params.action( FIRST_ARGS, message, payload, size );
        }
    }

    virtual void Translate( Rtt::Real deltaX, Rtt::Real deltaY )
    {
        OBJECT_BOX_SCOPE();

        OBJECT_BOX_STORE( DisplayObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( Translate );
        CORONA_OBJECTS_METHOD_WITH_ARGS( Translate, deltaX, deltaY )
    }

    virtual bool UpdateTransform( const Rtt::Matrix & parentToDstSpace )
    {
        OBJECT_BOX_SCOPE();

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
        OBJECT_BOX_SCOPE();

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
        OBJECT_BOX_SCOPE();
        
        OBJECT_BOX_STORE( GroupObject, storedThis, this );
        CORONA_OBJECTS_GET_PARAMS( DidInsert );
        CORONA_OBJECTS_METHOD_WITH_ARGS( DidInsert, childParentChanged )
    }

    virtual void DidRemove()
    {
        OBJECT_BOX_SCOPE();
        
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
int CoronaObjectInvalidate( CoronaDisplayObject object )
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
int CoronaObjectGetParent( CoronaDisplayObject object, CoronaGroupObject * parent )
{
    OBJECT_BOX_SCOPE_EXISTING();

    auto * displayObject = OBJECT_BOX_LOAD_WITH_SCOPE( DisplayObject, object );
    
    if (displayObject && parent)
    {
        OBJECT_BOX_STORE_VIA_POINTER( GroupObject, parent, displayObject->GetParent() );
        
        return allStored;
    }

    return 0;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaGroupObjectGetChild( CoronaGroupObject groupObject, int index, CoronaDisplayObject * child )
{
    OBJECT_BOX_SCOPE_EXISTING();

    auto * go = OBJECT_BOX_LOAD_WITH_SCOPE( GroupObject, groupObject );

    if (go && child && index >= 0 && index < go->NumChildren())
    {
        OBJECT_BOX_STORE_VIA_POINTER( DisplayObject, child, &go->ChildAt( index ) );

        return allStored;
    }

    return 0;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaGroupObjectGetNumChildren( CoronaGroupObject groupObject )
{
    auto * go = OBJECT_BOX_LOAD( GroupObject, groupObject );

    return go ? go->NumChildren() : 0;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaObjectSendMessage( CoronaDisplayObject object, const char * message, const void * payload, unsigned int size )
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
