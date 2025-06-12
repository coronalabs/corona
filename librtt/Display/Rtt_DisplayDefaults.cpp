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
	static TimeTransform sTransform;

	fTimeTransform = &sTransform;

	SetTimeTransform( NULL );
}

const TimeTransform* DisplayDefaults::GetTimeTransform() const
{
	return fTimeTransform;
}

void DisplayDefaults::SetTimeTransform( const TimeTransform *transform )
{
	if ( transform )
	{
		*fTimeTransform = *transform;
	}

	else
	{
		fTimeTransform->SetDefault();
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

