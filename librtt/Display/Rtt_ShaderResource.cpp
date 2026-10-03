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

Real
TimeTransform::Apply( Real value ) const
{
	Rtt_ASSERT( func );

	return func( value, arg1, arg2, arg3 );
}

static Real
Modulo( Real x, Real range, Real, Real )
{
    return fmod( x, range ); // TODO?: Rtt_RealFmod
}

static Real
PingPong( Real x, Real range, Real, Real )
{
    Real pos = fmod( x, Rtt_REAL_2 * range ); // TODO?: Rtt_RealFmod

    if (pos > range)
    {
        pos = Rtt_REAL_2 * range - pos;
    }

    return pos;
}

static Real
Sine( Real x, Real amplitude, Real speed, Real shift )
{
    return amplitude * Rtt_RealSin( speed * x + shift );
}

int
TimeTransform::Push( lua_State *L ) const
{
    if ( func )
    {
        lua_newtable( L );

        if ( &Modulo == func || &PingPong == func )
        {
            lua_pushstring( L, &Modulo == func ? "modulo" : "pingpong" );
            lua_setfield( L, -2, "func" );
            lua_pushnumber( L, arg1 );
            lua_setfield( L, -2, "range" );
        }

        else if ( &Sine == func )
        {
            lua_pushliteral( L, "sine" );
            lua_setfield( L, -2, "func" );
            lua_pushnumber( L, arg1 );
            lua_setfield( L, -2, "amplitude" );
            lua_pushnumber( L, (Rtt_REAL_2 * M_PI) / arg2 );
            lua_setfield( L, -2, "period" );
            lua_pushnumber( L, arg3 );
            lua_setfield( L, -2, "phase" );
        }

        else
        {
            Rtt_ASSERT_NOT_REACHED();

            return 0;
        }
    }

    else
    {
        lua_pushnil( L );
    }

    return 1;
}

static void
GetNumberArg( lua_State * L, int arg, Real * value, const char * func, const char * name, const char * what )
{
    lua_getfield( L, arg, name ); // ..., xform, ..., value?
        
    if (!lua_isnil( L, -1 ))
    {
        if (lua_isnumber( L, -1 ))
        {
            *value = (Real)lua_tonumber( L, -1 );
        }

        else
        {
            CoronaLuaWarning( L, "%s ignoring invalid '%s' parameter for %s time transform (expected number but got %s)",
                        what, name, func, lua_typename( L, lua_type( L, -1 ) ) );
        }
    }

    lua_pop( L, 1 ); // ..., xform, ...
}

static void
GetPositiveNumberArg( lua_State * L, int arg, Real * value, const char * func, const char * name, const char * what )
{
    Real old = *value;

    GetNumberArg( L, arg, value, func, name, what );

    if (*value <= Rtt_REAL_0)
    {
        *value = old;

        CoronaLuaWarning( L, "%s ignoring invalid '%s' parameter for %s time transform (must be positive number)",
            what, name, func );
    }
}

void
TimeTransform::SetDefault()
{
    func = &PingPong;
    arg1 = 50; // 50 should be safe for mediump
    arg2 = arg3 = 0;
}

void
TimeTransform::SetFunc( lua_State *L, int arg, const char *what, const char *fname )
{
    switch (*fname)
    {
    case 'm': // modulo
    case 'p': // pingpong
        {
            Real range = Rtt_REAL_1;
                
            GetPositiveNumberArg( L, arg, &range, fname, "range", what );

            bool isModulo = 'm' == *fname;

            func = isModulo ? &Modulo : &PingPong;
            arg1 = range;
        }
        break;

    case 's': // sine
        {
            Real amplitude = Rtt_REAL_1, period = Rtt_REAL_2 * M_PI, phase = Rtt_REAL_0;

            GetNumberArg( L, arg, &amplitude, fname, "amplitude", what );
            GetPositiveNumberArg( L, arg, &period, fname, "period", what );
            GetNumberArg( L, arg, &phase, fname, "phase", what );

            func = &Sine;
            arg1 = amplitude;
            arg2 = (Rtt_REAL_2 * M_PI) / period;
            arg3 = phase;
        }
        break;

    default:
        Rtt_ASSERT_NOT_REACHED();
    }
}

const char*
TimeTransform::FindFunc( lua_State *L, int arg, const char *what )
{
	const char *fname = NULL;

    lua_getfield( L, arg, "func" );    // ..., xform, ..., func

    if (lua_isstring( L, -1 ))
    {
        fname = lua_tostring( L, -1 );

        bool isValid = strcmp( fname, "modulo" ) == 0 ||
                        strcmp( fname, "pingpong" ) == 0 ||
                        strcmp( fname, "sine" ) == 0;
            
		if ( !isValid )
        {
            CoronaLuaWarning( L, "%s ignoring unknown %s time transform", what, fname );

			fname = NULL;
        }
    }

    lua_pop( L, 1 ); // ..., xform, ...

	return fname;
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
	fTimeTransform( NULL ),
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
	fTimeTransform( NULL ),
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

	if ( NULL != fTimeTransform )
	{
		Rtt_DELETE( fTimeTransform );
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

bool ShaderResource::sAddedUsesTime;

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

