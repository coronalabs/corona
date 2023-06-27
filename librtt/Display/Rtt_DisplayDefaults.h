//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

		U8 GetEmitterMapping() const { return fEmitterMapping; }
		void SetEmitterMapping( U8 newValue ) { fEmitterMapping = newValue; }

		bool IsImageSheetSampledInsideFrame() const { return fIsImageSheetSampledInsideFrame;}
		void SetImageSheetSampledInsideFrame( bool newValue ) { fIsImageSheetSampledInsideFrame = newValue; }

		bool IsImageSheetFrameTrimCorrected() const { return fIsImageSheetFrameTrimCorrected;}
		void SetImageSheetFrameTrimCorrected( bool newValue ) { fIsImageSheetFrameTrimCorrected = newValue; }

    bool IsExternalTextureRetina() const { return fIsExternalTextureRetina;}
		void SetExternalTextureRetina( bool newValue ) { fIsExternalTextureRetina = newValue; }

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
		U8 fEmitterMapping;
		bool fV1Compatibility;
		bool fPreloadTextures;
		bool fIsNativeTextFieldFontSizeScaled;
		bool fIsNativeTextBoxFontSizeScaled;
		bool fShaderCompilerVerbose;
		bool fIsAnchorClamped;
		bool fIsImageSheetSampledInsideFrame;
		bool fIsImageSheetFrameTrimCorrected;
		bool fIsExternalTextureRetina;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_DisplayDefaults_H__
