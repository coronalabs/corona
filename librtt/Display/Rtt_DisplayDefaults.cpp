//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_DisplayDefaults.h"

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
	fV1Compatibility( false ),
	fPreloadTextures( true ),
	fIsNativeTextFieldFontSizeScaled( true ),
	fIsNativeTextBoxFontSizeScaled( true ),
	fShaderCompilerVerbose( kShaderCompilerVerboseDefault ),
	fIsAnchorClamped( true ),
	fIsImageSheetSampledInsideFrame( false )
{
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

