//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShaderResource.h"

#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderData.h"
#include "Renderer/Rtt_Program.h"

#include "Display/Rtt_Display.h"
#include "CoronaLua.h"
#include "CoronaGraphics.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

int
ExtraTextureInfo::FindNameInList( const U8* name, const U8* listOfNames, int n )
{
	NamesReader reader( name ), listReader( listOfNames );
	
	reader.PullNext();
	
	return reader.FindCurrentNameInList( listReader, n );
}

U32
ExtraTextureInfo::NamesSize( const U8* listOfNames, int n )
{
	NamesReader reader( listOfNames );
	
	return reader.SizeOfList( n );
}

int
ExtraTextureInfo::EncodeName( U8* buf, const char* name )
{
	return String::EncodeIdentifier( buf, name, kMaxPackedNameCount );
}

// ----------------------------------------------------------------------------

NamesReader::NamesReader( const U8* stream )
:	fCount( 0 ),
	fPulls( 0 ),
	fTally( 0 ),
	fStream( stream )
{
}
			
void
NamesReader::PullNext()
{
	Rtt_ASSERT( fStream );

	fTally += fCount;

	fCount = ( *Current() ) * 3;

	fPulls++;
}

const U8*
NamesReader::Current() const
{
	return fStream + ( fTally + fPulls );
}

int
NamesReader::FindCurrentNameInList( NamesReader& headOfList, int n ) const
{
	Rtt_ASSERT( fStream );

	for ( int i = 0; i < n; i++ )
	{
		headOfList.PullNext();
		
		bool isMatch = ( fCount == headOfList.fCount ) && 0 == memcmp( Current(), headOfList.Current(), fCount );
		if ( isMatch )
		{
			return i;
		}
	}
	
	return -1;
}

bool
NamesReader::MatchesList( const NamesReader& headOfOtherList, int n ) const
{
	Rtt_ASSERT( fStream );

	NamesReader list1 = Clone(), list2 = headOfOtherList.Clone();

	for ( int i = 0; i < n; ++i )
	{
		list1.PullNext();
		list2.PullNext();
	}
	
	int total1 = list1.GetTotalBytes();
	
	return ( total1 == list2.GetTotalBytes() ) && 0 == memcmp( fStream, headOfOtherList.fStream, total1 );
}

U32
NamesReader::SizeOfList( int n ) const
{
	Rtt_ASSERT( fStream );

	NamesReader dup = Clone();
	
	for ( int i = 0; i < n; ++i )
	{
		dup.PullNext();
	}
	
	return dup.GetTotalBytes();
}

void
NamesReader::Decode( char* name ) const
{
	Rtt_ASSERT( fStream );

	String::DecodeIdentifier( name, Current(), String::IdentifierLengthToTriples( fCount ) );
}
	
// ----------------------------------------------------------------------------

int
LengthAccumulator::GetTotalBytes() const
{
	return fTotalTriples * 3;
}

void
LengthAccumulator::AddLength( int length )
{
	fTotalTriples += String::IdentifierLengthToTriples( length );
	
	fCount++;
}

// ----------------------------------------------------------------------------

NamesEncoder::NamesEncoder( U8* stream, const LengthAccumulator& acc )
:	fStream( stream ),
	fPos( 0 ),
	fRef( acc )
{
}

bool
NamesEncoder::Encode( const char* name, int length )
{
	int encoded = ExtraTextureInfo::EncodeName( fStream + fPos + 1, name );
	if ( encoded > 0 )
	{
		U32 triples = String::IdentifierLengthToTriples( length );
		
		Rtt_ASSERT( triples * 3 == (U32)encoded );
		
		fStream[fPos] = triples;
		fPos += encoded + 1;
	
		return true;
	}
	else
	{
		return false;
	}	
}

void
NamesEncoder::CheckTotalCount()
{
	Rtt_ASSERT( fPos == fRef.GetTotalBytes() + fRef.GetCount() );
}
	
// ----------------------------------------------------------------------------

static Real
Modulo( Real x, Real range )
{
    return fmod( x, range );
}

static Real
PingPong( Real x, Real range )
{
    Real pos = fmod( x, Rtt_REAL_2 * range );

    if (pos > range)
    {
        pos = Rtt_REAL_2 * range - pos;
    }

    return pos;
}

static Real
Sine( Real x, Real amplitude, Real speed, Real phase )
{
    return amplitude * Rtt_RealSin( speed * x + phase );
}

static Real
SmootherStep( Real y1, Real y2, Real t )
{
	Real t3 = t * t * t;
	Real factor = t3 * ( 10. - t * ( 15. - t * 6. ) );
	
	return y1 + (y2 - y1) * factor;
}

static Real
SmoothedModulo( Real x, Real range, Real extra )
{
	Real rem = fmod( x, range + extra );
	
	if ( rem < range )
	{
		return rem;
	}
	else
	{
		return SmootherStep( range, 0., ( rem - range ) / extra );
	}
}

static Real
Noise_Lissajous( Real x )
{
	return sin( M_PI * x ) + sin( 2. * x );
}

static Real
Noise_R2( Real x, Real range )
{
	// See from Martin Roberts "Extreme Learning", in particular "Jittering quasirandom point sets":
	// https://web.archive.org/web/20260228140726/https://extremelearning.com.au/a-simple-method-to-construct-isotropic-quasirandom-blue-noise-point-sequences/

	const double K = 1.0 / 1.324717957244746; /* 1 / plastic constant */

	Real ip, fp = modf( x / range, &ip );
	Real raw = ip * K;
	Real y1 = raw - floor( raw );
	Real y2 = ( raw + K ) - floor( raw + K );
	
	return SmootherStep( y1, y2, fp );
}

Real
TimeTransform::Apply( Real value ) const
{
	switch ( fMethod )
	{
	case kNone:
		return value;
	case kModulo:
		return Modulo( value, fArg1 );
	case kPingPong:
		return PingPong( value, fArg1 );
	case kSine:
		return Sine( value, fArg1, fArg2, fArg3 );
	case kSmoothedModulo:
		return SmoothedModulo( value, fArg1, fArg2 );
	case kNoise_Lissajous:
		return Noise_Lissajous( value );
	case kNoise_R2:
		return Noise_R2( value, fArg1 );
	default:
		Rtt_ASSERT_NOT_REACHED();
		return -1;
	}
}

int
TimeTransform::Push( lua_State *L ) const
{
	const char *name = StringForMethod( fMethod );
	
	Rtt_ASSERT( name );

	lua_newtable( L );
	lua_pushstring( L, name );
	lua_setfield( L, -2, "func" );

	switch ( fMethod )
	{
	case kNoise_Lissajous:
		break;
	case kModulo:
	case kPingPong:
	case kNoise_R2:
	case kSmoothedModulo:
		lua_pushnumber( L, fArg1 );
		lua_setfield( L, -2, "range" );
	   
		if ( kSmoothedModulo == fMethod )
		{
			lua_pushnumber( L, fArg2 );
			lua_setfield( L, -2, "smooth" );
		}
		
		break;
	case kSine:
		lua_pushnumber( L, fArg1 );
		lua_setfield( L, -2, "amplitude" );
		lua_pushnumber( L, (Rtt_REAL_2 * M_PI) / fArg2 );
		lua_setfield( L, -2, "period" );
		lua_pushnumber( L, fArg3 );
		lua_setfield( L, -2, "phase" );
		
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();

		return 0;
	}
        
    return 1;
}

static bool
GetNumberArg( lua_State * L, int arg, Real * value, TimeTransform::Method method, const char * name, const char * what )
{
	bool ok = true;

    lua_getfield( L, arg, name ); // ..., xform, ..., value?
        
	if ( lua_isnumber( L, -1 ) )
	{
		*value = (Real)lua_tonumber( L, -1 );
	}

	else if ( !lua_isnil( L, -1 ) )
	{
		CoronaLuaWarning( L, "%s ignoring invalid '%s' parameter for %s time transform (expected number but got %s)",
					what, name, TimeTransform::StringForMethod( method ), lua_typename( L, lua_type( L, -1 ) ) );
	
		ok = false;
	}

    lua_pop( L, 1 ); // ..., xform, ...
    
    return ok;
}

static bool
GetPositiveNumberArg( lua_State * L, int arg, Real * value, TimeTransform::Method method, const char * name, const char * what )
{
    if( GetNumberArg( L, arg, value, method, name, what ) && ( *value > Rtt_REAL_0 ) )
    {
		return true;
	}
	else
    {
        CoronaLuaWarning( L, "%s ignoring invalid '%s' parameter for %s time transform (must be positive number)",
            what, name, TimeTransform::StringForMethod( method ) );
            
		return false;
    }
}

bool
TimeTransform::Matches( const TimeTransform *other ) const
{
	if ( NULL != other )
	{
		return 0 == memcmp( this, other, sizeof(TimeTransform) );
	}
	else
	{	
		return kNone == fMethod;
	}
}

void
TimeTransform::SetMethod( lua_State *L, int arg, const char *what, Method method )
{
	*this = TimeTransform(); // reset to wipe args and account for errors

    switch ( method )
    {
    case kModulo:
    case kPingPong:
    case kNoise_R2:
        {
            Real range = Rtt_REAL_1;

            if( GetPositiveNumberArg( L, arg, &range, method, "range", what ) )
            {
				fMethod = method;
				fArg1 = range;
			}
        }
        break;
    case kSine:
        {
            Real amplitude = Rtt_REAL_1, period = Rtt_REAL_2 * M_PI, phase = Rtt_REAL_0;

            if( GetNumberArg( L, arg, &amplitude, method, "amplitude", what ) &&
				GetPositiveNumberArg( L, arg, &period, method, "period", what ) &&
				GetNumberArg( L, arg, &phase, method, "phase", what ) )
			{
				fMethod = method;
				fArg1 = amplitude;
				fArg2 = (Rtt_REAL_2 * M_PI) / period;
				fArg3 = phase;
			}
        }
        break;
	case kSmoothedModulo:
		{
            Real range = Rtt_REAL_1, smooth = Rtt_REAL_1;

            if( GetPositiveNumberArg( L, arg, &range, method, "range", what ) &&
				GetPositiveNumberArg( L, arg, &smooth, method, "smooth", what ) )
			{
				fMethod = method;
				fArg1 = range;
				fArg2 = smooth;
			}
        }
		break;

	case kNoise_Lissajous:
		fMethod = method;
		break;

    default:
        Rtt_ASSERT_NOT_REACHED();
    }
}

const static struct {
	TimeTransform::Method method;
	const char *name;
} kMethodPairs[] = {
	{ TimeTransform::kNone, "none" },
	{ TimeTransform::kModulo, "modulo" },
	{ TimeTransform::kPingPong, "pingpong" },
	{ TimeTransform::kSine, "sine" },
	{ TimeTransform::kSmoothedModulo, "smoothedModulo" },
	{ TimeTransform::kNoise_Lissajous, "lissajousNoise" },
	{ TimeTransform::kNoise_R2, "r2Noise" }
};

const char*
TimeTransform::StringForMethod( Method method )
{
	for ( auto && pair : kMethodPairs )
	{
		if ( pair.method == method )
		{
			return pair.name;
		}
	}
	
	Rtt_ASSERT_NOT_REACHED();
	
	return NULL;
}

TimeTransform::Method
TimeTransform::MethodForString( const char *name )
{
	for ( auto && pair : kMethodPairs )
	{
		if ( 0 == strcmp( name, pair.name ) )
		{
			return pair.method;
		}
	}
	
	return kNumMethodTypes;
}

TimeTransform::Method
TimeTransform::FindMethod( lua_State *L, int arg, const char *what )
{
	Method method = kNumMethodTypes;
	
	// n.b. "func" is documented, so keep in spite of method nomenclature

    lua_getfield( L, arg, "func" ); // ..., xform, ..., func

    if ( lua_isstring( L, -1 ) )
    {
		method = MethodForString( lua_tostring( L, -1 ) );
    }

    lua_pop( L, 1 ); // ..., xform, ...

	return method;
}

ShaderResource::ShaderResource( Program *program, ShaderTypes::Category category )
:	fCategory( category ),
	fName(),
	fVertexDataMap(),
	fUniformDataMap(),
	fDefaultData( NULL ),
    fEffectCallbacks( NULL ),
    fDetailNames( NULL ),
    fDetailValues( NULL ),
    fDetailsCount( 0U ),
    fExtensionPrelude( NULL ),
    fExtraTextureInfo( NULL ),
    fShellTransform( NULL ),
	fTimeTransform(),
	fExtraTextureCount( 0 ),
    fFirstVersion( 0 ),
	fIsFirstMod25D( false ),
	fAnyVersionBound( false ),
	fSyncPending( false ),
	fUsesUniforms( false ),
	fUsesTime( false ),
	fTextureInfoIsSet( false )
{
	Init(program);
}

ShaderResource::ShaderResource( Program *program, ShaderTypes::Category category, const char *name )
:	fCategory( category ),
	fName( name ),
	fVertexDataMap(),
	fUniformDataMap(),
	fDefaultData( NULL ),
    fEffectCallbacks( NULL ),
    fDetailNames( NULL ),
    fDetailValues( NULL ),
    fDetailsCount( 0U ),
    fExtensionPrelude( NULL ),
    fExtraTextureInfo( NULL ),
    fShellTransform( NULL ),
	fTimeTransform(),
	fExtraTextureCount( 0 ),
    fFirstVersion( 0 ),
	fIsFirstMod25D( false ),
	fAnyVersionBound( false ),
    fSyncPending( false ),
	fUsesUniforms( false ),
	fUsesTime( false ),
	fTextureInfoIsSet( false )
{
	Init(program);
}

void
ShaderResource::Init(Program *defaultProgram)
{
	for (int i = 0; i < kNumProgramMods; i++)
	{
		fPrograms[i] = NULL;
	}
	fPrograms[ShaderResource::kDefault] = defaultProgram;

	fFillTextureInfo[0] = {};
	fFillTextureInfo[1] = {};

	defaultProgram->SetShaderResource( this );
}

ShaderResource::~ShaderResource()
{
	// TODO: We will need to queuerelease of this once we move away from a prototype-based cloning in ShaderFactory
	for (int i = 0; i < kNumProgramMods; i++)
	{
		Rtt_DELETE(fPrograms[i]);
	}
    
    if ( NULL != fDefaultData )
	{
		Rtt_DELETE( fDefaultData );
    }

	Rtt_FREE( const_cast<ExtraTextureInfo*>( fExtraTextureInfo ) );

    SetEffectCallbacks( NULL );
    SetShellTransform( NULL );
	SetExtensionPrelude( NULL );
}

void
ShaderResource::AddEffectDetail( const char * name, const char * value )
{
    fDetailNames.push_back( name );
    fDetailValues.push_back( value );
}

int
ShaderResource::GetEffectDetail( int index, CoronaEffectDetail & detail ) const
{
    if (index >= 0 && index < fDetailNames.size() )
    {
        detail.name = fDetailNames[index].c_str();
        detail.value = fDetailValues[index].c_str();

        return 1;
    }

    return 0;
}

void
ShaderResource::SetProgramMod(ProgramMod mod, Program *program)
{
	
	if ( Rtt_VERIFY(NULL == fPrograms[mod]) )
	{
		fPrograms[mod] = program;

		program->SetShaderResource( this );
	}
}

Program *
ShaderResource::GetProgramMod(ProgramMod mod) const
{
	return fPrograms[mod];
}

void
ShaderResource::SetTextureInfo( const U8* info, U8 count, SamplerTypeDetails fillInfo[2] )
{
	Rtt_FREE( const_cast<ExtraTextureInfo*>( fExtraTextureInfo ) );
	
	Rtt_ASSERT( ( NULL == info ) == ( 0 == count ) );
	
	fExtraTextureInfo = (const ExtraTextureInfo*)info;
	fExtraTextureCount = count;

	fFillTextureInfo[0] = fillInfo[0];
	fFillTextureInfo[1] = fillInfo[1];

	fTextureInfoIsSet = true;
}

const SamplerTypeDetails*
ShaderResource::GetExtraTextureDetails() const
{
	return ( fExtraTextureCount > 0 ) ? (SamplerTypeDetails*)fExtraTextureInfo->fData : NULL;
}

const U8*
ShaderResource::GetExtraTextureNames() const
{
	int detailsSize = fExtraTextureCount * sizeof(SamplerTypeDetails);

	return ( fExtraTextureCount > 0 ) ? fExtraTextureInfo->fData + detailsSize : NULL;
}	

static bool
ReportError( const NamesReader& reader, const char* message )
{
	char rawName[ExtraTextureInfo::kMaxNameLength + 1];

	reader.Decode( rawName );
			
	Rtt_LogException( message, rawName );

	return false;
}

bool
ShaderResource::DetailsAgree( U32 formatBackingValue, const SamplerTypeDetails& details )
{
	if ( !FormatDetails::IsCore( formatBackingValue ) )
	{
		bool targetsAgree = FormatDetails::GetTarget( formatBackingValue ) == details.target;
		bool familiesAgree = FormatDetails::GetFamily( formatBackingValue ) == details.family;
		
		return targetsAgree && familiesAgree && ( FormatDetails::HasArrayFlag( formatBackingValue ) == details.isArray );
	}
	else
	{
		return details.IsDefault();
	}
}

static bool
DetailsAgreeWithFormat( U32 backingValue, const SamplerTypeDetails& details )
{
	return ShaderResource::DetailsAgree( backingValue, details );
}

bool
ShaderResource::AreFormatsConsistent( U32 fillBackingValues[], U32 extraTextureBackingValues[], U32 extraCount, const U8* paintNames, RenderDataState* renderDataState ) const
{
	Rtt_ASSERT( HasTextureInfo() );

	if ( !DetailsAgreeWithFormat( fillBackingValues[0], GetFillInfo( 0 ) ) )
	{
		Rtt_LogException( "`CoronaSampler0` inconsistent with image in paint1" );
		return false;
	}
	
	if ( !DetailsAgreeWithFormat( fillBackingValues[1], GetFillInfo( 1 ) ) )
	{
		Rtt_LogException( "`CoronaSampler1` inconsistent with image in paint2" );
		return false;
	}

	U32 iMax = GetExtraTextureCount();
	const SamplerTypeDetails* shaderDetails = GetExtraTextureDetails();
	const U8* shaderNames = GetExtraTextureNames();

	Rtt_ASSERT( ( NULL != extraTextureBackingValues ) == ( NULL != paintNames ) );

	if ( iMax > extraCount )
	{
		char buf[32] = "no";
		
		if ( extraCount )
		{
			snprintf( buf, sizeof(buf), "only %u", extraCount );
		}

		Rtt_LogException( "WARNING: shader has %i samplers to bind, but %s textures provided in `extraPaints`", iMax, buf );
		return false;
	}

	Rtt_ASSERT( iMax == 0 || ( NULL != extraTextureBackingValues ) );

	U32 occupancyMask = 0;
	int basePaintIndex = 0; // both name lists are sorted, so avoid searching entire list each iteration
	
	NamesReader shaderNamesIter( shaderNames ), paintNamesIter( paintNames );
	for ( U32 i = 0; i < iMax; i++, basePaintIndex++ )
	{
		shaderNamesIter.PullNext();
	
		int index = shaderNamesIter.FindCurrentNameInList( paintNamesIter, extraCount - basePaintIndex );
		
		basePaintIndex += index;
				
		if ( index < 0 )
		{
			return ReportError( shaderNamesIter, "WARNING: unable to match sampler `%s` with a corresponding texture from the paints" );
		}
		else if ( !DetailsAgreeWithFormat( extraTextureBackingValues[basePaintIndex], shaderDetails[i] ) )
		{
			return ReportError( shaderNamesIter, "WARNING: sampler `%s` inconsistent with image provided in `extraPaints`" );
		}

		occupancyMask |= 1U << basePaintIndex;
	}
	
	if ( NULL != renderDataState )
	{
		renderDataState->SetOccupancy( occupancyMask );
	}
	
	return true;
}

bool
ShaderResource::AreTexturesConsistent( const TextureList& list, const U8* paintNames, RenderDataState* renderDataState ) const
{
	Rtt_ASSERT( HasTextureInfo() );

	const Texture* fill0 = list.GetFill0();
	const Texture* fill1 = list.GetFill1();

	U32 fillBackingValues[2] = {
		fill0 ? fill0->GetFormat().GetBackingValue() : 0,
		fill1 ? fill1->GetFormat().GetBackingValue() : 0
	}, extraTextureBackingValues[ RenderDataState::kOccupancyBits ] = {};
	
	U32 extraCount = list.GetCountAfterFills();
	for ( U32 i = 0; i < extraCount; i++ )
	{
		extraTextureBackingValues[i] = list.GetPositionAfterFills()[i]->GetFormat().GetBackingValue();
	}
	
	return AreFormatsConsistent( fillBackingValues, extraCount > 0 ? extraTextureBackingValues : NULL, extraCount, paintNames, renderDataState );
}

void
ShaderResource::PrepareFirstBind( const Program* program, int version )
{
	Rtt_ASSERT( !fAnyVersionBound );
	Rtt_ASSERT( !fSyncPending );
	Rtt_ASSERT( version < Program::Version::kWireframe );
	Rtt_ASSERT( program == fPrograms[kDefault] || program == fPrograms[k25D] );

	fSyncPending = true;
	fFirstVersion = version;
	fIsFirstMod25D = program == fPrograms[k25D];
}

void
ShaderResource::SyncBinding()
{
	fAnyVersionBound = true;
	fSyncPending = false;
}

const Program*
ShaderResource::GetFirstBoundProgram() const
{
	if ( fAnyVersionBound )
	{
		return GetProgramMod( fIsFirstMod25D ? k25D : kDefault );
	}
	else
	{
		return NULL;
	}
}

void
ShaderResource::SetExtensionPrelude( const char* prelude )
{
	if ( NULL != fExtensionPrelude )
	{
		Rtt_FREE( fExtensionPrelude );
	}
	
	fExtensionPrelude = ( NULL != prelude ) ? strdup( prelude ) : NULL;
}

void
ShaderResource::SetEffectCallbacks( CoronaEffectCallbacks * callbacks )
{
    if (NULL != fEffectCallbacks)
    {
        Rtt_DELETE( fEffectCallbacks );
        
        fEffectCallbacks = NULL;
    }
    
    if (NULL != callbacks) // clone to allow removal of original
    {
        fEffectCallbacks = Rtt_NEW( NULL, CoronaEffectCallbacks( *callbacks ) );
    }
}

void
ShaderResource::SetShellTransform( CoronaShellTransform * shellTransform )
{
    if (NULL != fShellTransform)
    {
        Rtt_DELETE( fShellTransform );
        
        fShellTransform = NULL;
    }
    
    if (NULL != shellTransform) // clone to allow removal of original
    {
        fShellTransform = Rtt_NEW( NULL, CoronaShellTransform( *shellTransform ) );
    }
}

int
ShaderResource::GetDataIndex( const char *key ) const
{
	int result = -1;

	if ( Rtt_VERIFY( key ) )
	{
		if ( UsesUniforms() )
		{
			std::string k( key );
			UniformDataMap::const_iterator element = fUniformDataMap.find( k );
			if ( element != fUniformDataMap.end() )
			{
				result = element->second.index;
			}
		}
		else
		{
			std::string k( key );
			VertexDataMap::const_iterator element = fVertexDataMap.find( k );
			if ( element != fVertexDataMap.end() )
			{
				result = element->second;
			}
		}
	}

    if (-1 == result && fEffectCallbacks && fEffectCallbacks->getDataIndex)
    {
        result = fEffectCallbacks->getDataIndex( key );

        if (result >= 0)
        {
            result += ShaderData::kNumData;
        }
    }

	return result;
}

ShaderResource::UniformData
ShaderResource::GetUniformData( const char *key ) const
{
	UniformData result = { -1, Uniform::kScalar };

	if ( Rtt_VERIFY( UsesUniforms() ) )
	{
		std::string k( key );
		UniformDataMap::const_iterator element = fUniformDataMap.find( k );
		if ( element != fUniformDataMap.end() )
		{
			result = element->second;
		}
	}

	return result;
}

void
ShaderResource::SetDefaultData( ShaderData *defaultData )
{
	if ( defaultData != fDefaultData )
	{
		Rtt_DELETE( fDefaultData );
		fDefaultData = defaultData;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

