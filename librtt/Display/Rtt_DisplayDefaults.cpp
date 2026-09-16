//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_ShaderResource.h"

#include <type_traits>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const Color kBlack = 0xFF000000;
static const Color kWhite = 0xFFFFFFFF;

#ifdef Rtt_AUTHORING_SIMULATOR
    // Simulator should always show errors
    static bool kShaderCompilerVerboseDefault = true;
#else
    static bool kShaderCompilerVerboseDefault = false;
#endif


struct ExtraSpace {
	TimeTransform fTimeTransform;
	bool fWrittenTimeTransform;
};

DisplayDefaults::DisplayDefaults()
:	fClearColor( kBlack ),
	fFillColor( kWhite ),
	fStrokeColor( kWhite ),
	fTextColor( kWhite ),
	fLineColor( kWhite ),
	fAnchorX( 0.5 ),
	fAnchorY( 0.5 ),
	fMagTextureFilter( RenderTypes::kLinearTextureFilter ),
	fMinTextureFilter( RenderTypes::kLinearTextureFilter ),
	fWrapX( RenderTypes::kClampToEdgeWrap ),
	fWrapY( RenderTypes::kClampToEdgeWrap ),
	fEmitterMapping( 0 ),
	fV1Compatibility( false ),
	fPreloadTextures( true ),
	fIsNativeTextFieldFontSizeScaled( true ),
	fIsNativeTextBoxFontSizeScaled( true ),
	fShaderCompilerVerbose( kShaderCompilerVerboseDefault ),
	fIsAnchorClamped( true ),
	fIsImageSheetSampledInsideFrame( false ),
	fIsImageSheetFrameTrimCorrected( false ),
	fIsExternalTextureRetina( true ),
    fSkipsCull( false ),
    fSkipsHitTest( false ),
    fEnableDepthInScene( false ),
    fEnableStencilInScene( false ),
    fAddDepthToResource( false ),
    fAddStencilToResource( false ),
    fSceneDepthClear( 1.0 ),
    fAddedDepthClear( 1.0 ),
    fSceneStencilClear( 0 ),
    fAddedStencilClear( 0 )
{
	Rtt_STATIC_ASSERT( sizeof(fExtraSpace) >= sizeof(ExtraSpace) );
	Rtt_STATIC_ASSERT( std::is_trivially_destructible<ExtraSpace>::value );
	Rtt_STATIC_ASSERT( std::is_trivially_copyable<ExtraSpace>::value );

	ExtraSpace es;

	memcpy( fExtraSpace, &es, sizeof(ExtraSpace) );
}

TimeTransform DisplayDefaults::GetTimeTransform( bool gpuSupportsHighPrecisionShaders ) const
{
	ExtraSpace es;
	
	memcpy( &es, fExtraSpace, sizeof(ExtraSpace) );
	
	if ( es.fWrittenTimeTransform )
	{
		return es.fTimeTransform;
	}
	else
	{
		TimeTransform transform;
		
		transform.fMethod = TimeTransform::kPingPong;
		transform.fArg1 = gpuSupportsHighPrecisionShaders ? 4.55 * 3600 /* 16380, just shy of 2^14 */
															: 63.975; // similarly almost 2^7; 1/8 second precision
		return transform;
	}
}

void DisplayDefaults::SetTimeTransform( const TimeTransform& transform )
{
	ExtraSpace es;
	
	es.fTimeTransform = transform;
	es.fWrittenTimeTransform = true;
	
	memcpy( fExtraSpace, &es, sizeof(ExtraSpace) );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

