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

#ifndef _Rtt_DisplayDefaults_H__
#define _Rtt_DisplayDefaults_H__

#include "Core/Rtt_Types.h"
#include "Display/Rtt_Paint.h"
#include "Renderer/Rtt_RenderTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class DisplayDefaults
{
	public:
		DisplayDefaults();

	public:
		Color GetClearColor() const { return fClearColor; }
		Color GetFillColor() const { return fFillColor; }
		Color GetStrokeColor() const { return fStrokeColor; }
		Color GetTextColor() const { return fTextColor; }
		Color GetLineColor() const { return fLineColor; }
		float GetAnchorX() const { return fAnchorX; }
		float GetAnchorY() const { return fAnchorY; }
		bool IsAnchorClamped() const { return fIsAnchorClamped; }
		
		void SetClearColor( Color newValue ) { fClearColor = newValue; }
		void SetFillColor( Color newValue ) { fFillColor = newValue; }
		void SetStrokeColor( Color newValue ) { fStrokeColor = newValue; }
		void SetTextColor( Color newValue ) { fTextColor = newValue; }
		void SetLineColor( Color newValue ) { fLineColor = newValue; }
		void SetAnchorX( float newValue ) { fAnchorX = newValue; }
		void SetAnchorY( float newValue ) { fAnchorY = newValue; }
		void SetAnchorClamped( bool newValue ) { fIsAnchorClamped = newValue; }
		
	public:
		RenderTypes::TextureFilter GetMagTextureFilter() const { return (RenderTypes::TextureFilter)fMagTextureFilter; }
		void SetMagTextureFilter( RenderTypes::TextureFilter newValue ) { fMagTextureFilter = newValue; }

		RenderTypes::TextureFilter GetMinTextureFilter() const { return (RenderTypes::TextureFilter)fMinTextureFilter; }
		void SetMinTextureFilter( RenderTypes::TextureFilter newValue ) { fMinTextureFilter = newValue; }

		RenderTypes::TextureWrap GetTextureWrapX() const { return (RenderTypes::TextureWrap)fWrapX; }
		void SetTextureWrapX( RenderTypes::TextureWrap newValue ) { fWrapX = newValue; }

		RenderTypes::TextureWrap GetTextureWrapY() const { return (RenderTypes::TextureWrap)fWrapY; }
		void SetTextureWrapY( RenderTypes::TextureWrap newValue ) { fWrapY = newValue; }

		bool IsImageSheetSampledInsideFrame() const { return fIsImageSheetSampledInsideFrame;}
		void SetImageSheetSampledInsideFrame( bool newValue ) { fIsImageSheetSampledInsideFrame = newValue; }
	public:
		bool IsV1Compatibility() const { return fV1Compatibility; }
		void SetV1Compatibility( bool newValue ) { fV1Compatibility = newValue; }

		bool IsByteColorRange() const { return IsV1Compatibility(); }
		
		bool ShouldPreloadTextures() const { return fPreloadTextures; }
		void SetPreloadTextures( bool newValue ) { fPreloadTextures = newValue; }
	
		bool IsNativeTextFieldFontSizeScaled() const { return fIsNativeTextFieldFontSizeScaled; }
		void SetIsNativeTextFieldFontSizeScaled( bool value ) { fIsNativeTextFieldFontSizeScaled = value; }
		bool IsNativeTextBoxFontSizeScaled() const { return fIsNativeTextBoxFontSizeScaled; }
		void SetIsNativeTextBoxFontSizeScaled( bool value ) { fIsNativeTextBoxFontSizeScaled = value; }

		bool IsShaderCompilerVerbose() const { return fShaderCompilerVerbose; }
		void SetShaderCompilerVerbose( bool newValue ) { fShaderCompilerVerbose = newValue; }

	private:
		Color fClearColor;
		Color fFillColor;
		Color fStrokeColor;
		Color fTextColor;
		Color fLineColor;
		float fAnchorX;
		float fAnchorY;
		U8 fMagTextureFilter;
		U8 fMinTextureFilter;
		U8 fWrapX;
		U8 fWrapY;
		bool fV1Compatibility;
		bool fPreloadTextures;
		bool fIsNativeTextFieldFontSizeScaled;
		bool fIsNativeTextBoxFontSizeScaled;
		bool fShaderCompilerVerbose;
		bool fIsAnchorClamped;
		bool fIsImageSheetSampledInsideFrame;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_DisplayDefaults_H__
