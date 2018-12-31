// ----------------------------------------------------------------------------
// 
// TextRendererSettings.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "TextRendererSettings.h"
#include "FontSettings.h"
#include "HorizontalAlignment.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
TextRendererSettings::TextRendererSettings()
:	fFontSettings(ref new CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings()),
	fHorizontalAlignment(CoronaLabs::Corona::WinRT::Interop::Graphics::HorizontalAlignment::Left),
	fBlockWidth(0),
	fBlockHeight(0),
	fClipWidth(0),
	fClipHeight(0),
	fText(nullptr)
{
}

#pragma endregion


#pragma region Public Properties/Methods
CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings^ TextRendererSettings::FontSettings::get()
{
	return fFontSettings;
}

CoronaLabs::Corona::WinRT::Interop::Graphics::HorizontalAlignment^ TextRendererSettings::HorizontalAlignment::get()
{
	return fHorizontalAlignment;
}

void TextRendererSettings::HorizontalAlignment::set(CoronaLabs::Corona::WinRT::Interop::Graphics::HorizontalAlignment^ value)
{
	if (!value)
	{
		throw ref new Platform::NullReferenceException();
	}
	fHorizontalAlignment = value;
}

int TextRendererSettings::BlockWidth::get()
{
	return fBlockWidth;
}

void TextRendererSettings::BlockWidth::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fBlockWidth = value;
}

int TextRendererSettings::BlockHeight::get()
{
	return fBlockHeight;
}

void TextRendererSettings::BlockHeight::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fBlockHeight = value;
}

int TextRendererSettings::ClipWidth::get()
{
	return fClipWidth;
}

void TextRendererSettings::ClipWidth::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fClipWidth = value;
}

int TextRendererSettings::ClipHeight::get()
{
	return fClipHeight;
}

void TextRendererSettings::ClipHeight::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fClipHeight = value;
}

Platform::String^ TextRendererSettings::Text::get()
{
	return fText;
}

void TextRendererSettings::Text::set(Platform::String^ value)
{
	fText = value;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
