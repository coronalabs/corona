//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "Core\Rtt_String.h"
#	include "Rtt_PlatformFont.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	namespace Interop { namespace Graphics {
		ref class FontSettings;
	} }
	ref class CoronaRuntimeEnvironment;
} } }

#pragma endregion


namespace Rtt
{

class WinRTFont : public PlatformFont
{
	public:
		WinRTFont(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment, const char *fontName);
		WinRTFont(const WinRTFont &font);
		virtual ~WinRTFont();

		virtual PlatformFont* CloneUsing(Rtt_Allocator *allocator) const;
		virtual const char* Name() const;
		virtual void SetSize(Rtt_Real newValue);
		virtual Rtt_Real Size() const;
		virtual void* NativeObject() const;
		CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings^ GetSettings();

	private:
		CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ fEnvironment;
		CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings^ fSettings;
		Rtt::String fFontName;
};

}
