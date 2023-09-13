//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinTextBoxObject.h"
#include "Core\Rtt_Build.h"
#include "Corona\CoronaLua.h"
#include "Display\Rtt_Display.h"
#include "Display\Rtt_LuaLibDisplay.h"
#include "Interop\Graphics\CoronaFontServices.h"
#include "Interop\Graphics\Font.h"
#include "Interop\Graphics\FontSizeConverter.h"
#include "Interop\Graphics\HorizontalAlignment.h"
#include "Interop\UI\RenderSurfaceControl.h"
#include "Interop\UI\TextBox.h"
#include "Interop\UI\Window.h"
#include "Interop\MDeviceSimulatorServices.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_Event.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"
#include "Rtt_WinFont.h"
#include "Rtt_WinPlatform.h"
#include <cmath>
#include <GdiPlus.h>


namespace Rtt
{

#pragma region Constructors/Destructors
WinTextBoxObject::WinTextBoxObject(Interop::RuntimeEnvironment& environment, const Rect& bounds, bool isSingleLine)
:	Super(environment, bounds),
	fTextBoxPointer(nullptr),
	fIsSingleLine(isSingleLine),
	fIsFontSizeScaled(true),
	fCachedSimulatorZoomScale(1.0),
	fIsPlaceholderTextNil(true),
	fResizedEventHandler(this, &WinTextBoxObject::OnResized),
	fGainedFocusEventHandler(this, &WinTextBoxObject::OnGainedFocus),
	fLostFocusEventHandler(this, &WinTextBoxObject::OnLostFocus),
	fTextChangedEventHandler(this, &WinTextBoxObject::OnTextChanged),
	fPressedEnterKeyEventHandler(this, &WinTextBoxObject::OnPressedEnterKey)
{
}

WinTextBoxObject::~WinTextBoxObject()
{
	if (fTextBoxPointer)
	{
		delete fTextBoxPointer;
		fTextBoxPointer = nullptr;
	}
}

#pragma endregion


#pragma region Public Methods
Interop::UI::Control* WinTextBoxObject::GetControl() const
{
	return fTextBoxPointer;
}

bool WinTextBoxObject::Initialize()
{
	// Do not continue if this object was already initialized.
	if (fTextBoxPointer)
	{
		return true;
	}

	// Fetch the bounds of this display object converted from Corona coordinates to native screen coordinates.
	Rect screenBounds;
	GetScreenBounds(screenBounds);

	// Create and configure the native Win32 text box this object will manage.
	Interop::UI::TextBox::CreationSettings settings{};
	settings.ParentWindowHandle = GetRuntimeEnvironment().GetRenderSurface()->GetWindowHandle();
	settings.IsSingleLine = fIsSingleLine;
	settings.Bounds.left = (LONG)Rtt_RealToInt(screenBounds.xMin);
	settings.Bounds.top = (LONG)Rtt_RealToInt(screenBounds.yMin);
	settings.Bounds.right = (LONG)Rtt_RealToInt(screenBounds.xMax);
	settings.Bounds.bottom = (LONG)Rtt_RealToInt(screenBounds.yMax);
	fTextBoxPointer = new Interop::UI::TextBox(settings);
	if (!fIsSingleLine)
	{
		fTextBoxPointer->SetReadOnly(true);
	}

	// Add event handlers to the native text box.
	fTextBoxPointer->GetResizedEventHandlers().Add(&fResizedEventHandler);
	fTextBoxPointer->GetGainedFocusEventHandlers().Add(&fGainedFocusEventHandler);
	fTextBoxPointer->GetLostFocusEventHandlers().Add(&fLostFocusEventHandler);
	fTextBoxPointer->GetTextChangedEventHandlers().Add(&fTextChangedEventHandler);
	fTextBoxPointer->GetPressedEnterKeyEventHandlers().Add(&fPressedEnterKeyEventHandler);

	// Apply the simulator's zoom scale to the text box's font size, if applicable.
	ApplySimulatorZoomScale();

	// Let the base class finish initialization of this object.
	return WinDisplayObject::Initialize();
}

const LuaProxyVTable& WinTextBoxObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetTextFieldObjectProxyVTable();
}

int WinTextBoxObject::ValueForKey(lua_State *L, const char key[]) const
{
	// Validate.
	if (!fTextBoxPointer || Rtt_StringIsEmpty(key))
	{
		if (key && !strcmp(PlatformDisplayObject::kUserInputEvent, key))
		{
			// This property will get queried before Initialize() gets called. Let it no-op.
		}
		else
		{
			// Trigger an assert for everything else.
			Rtt_ASSERT(0);
		}
		return 0;
	}

	int result = 1;
	if (strcmp("text", key) == 0)
	{
		// Fetch the UTF-16 encoded text from the control.
		std::wstring utf16Text;
		fTextBoxPointer->CopyTextTo(utf16Text);

		// Convert to UTF-8 and push to Lua. Make sure we never return nil.
		WinString stringTranscoder(utf16Text.c_str());
		auto utf8Text = stringTranscoder.GetUTF8();
		lua_pushstring(L, utf8Text ? utf8Text : "");
	}
	else if (strcmp("size", key) == 0)
	{
		float fontSize;
		auto fontPointer = fTextBoxPointer->GetFont().GetGdiPlusFont();
		if (fontPointer)
		{
			Interop::Graphics::FontSizeConverter fontSizeConverter;
			fontSizeConverter.SetSize((float)fontPointer->GetSize(), fontPointer->GetUnit());
			if (fIsFontSizeScaled)
			{
				fontSizeConverter.ConvertTo(Gdiplus::UnitPixel);
				fontSize = fontSizeConverter.GetSize();
				fontSize *= Rtt_RealToFloat(GetRuntimeEnvironment().GetRuntime()->GetDisplay().GetSxUpright());
			}
			else
			{
				fontSizeConverter.ConvertTo(Gdiplus::UnitPoint);
				fontSize = fontSizeConverter.GetSize();
			}
		}
		else
		{
			fontSize = Rtt_RealToFloat(GetRuntimeEnvironment().GetPlatform()->GetStandardFontSize());
		}
		if (GetRuntimeEnvironment().GetDeviceSimulatorServices())
		{
			fontSize /= (float)fCachedSimulatorZoomScale;
		}
		lua_pushnumber(L, fontSize);
	}
	else if (strcmp("font", key) == 0)
	{
		auto fontPointer = Rtt_NEW(&GetRuntimeEnvironment().GetAllocator(), WinFont(GetRuntimeEnvironment()));
		HDC deviceContextHandle = ::GetDC(fTextBoxPointer->GetWindowHandle());
		if (deviceContextHandle)
		{
			Interop::Graphics::FontSettings fontSettings;
			fontSettings.CopyFrom(deviceContextHandle, fTextBoxPointer->GetFont());
			fontSettings.CopyTo(*fontPointer);
			::ReleaseDC(fTextBoxPointer->GetWindowHandle(), deviceContextHandle);
		}
		if (fIsFontSizeScaled)
		{
			auto fontSize = fontPointer->Size();
			fontSize = Rtt_RealMul(fontSize, GetRuntimeEnvironment().GetRuntime()->GetDisplay().GetSxUpright());
			fontPointer->SetSize(fontSize);
		}
		else
		{
			Interop::Graphics::FontSizeConverter fontSizeConverter;
			fontSizeConverter.SetSize(Rtt_RealToFloat(fontPointer->Size()), Gdiplus::UnitPixel);
			fontSizeConverter.ConvertTo(Gdiplus::UnitPoint);
			fontPointer->SetSize(Rtt_FloatToReal(fontSizeConverter.GetSize()));
		}
		if (GetRuntimeEnvironment().GetDeviceSimulatorServices())
		{
			float fontSize = Rtt_RealToFloat(fontPointer->Size());
			fontSize /= (float)fCachedSimulatorZoomScale;
			fontPointer->SetSize(Rtt_FloatToReal(fontSize));
		}
		result = LuaLibNative::PushFont(L, fontPointer);
	}
	else if (strcmp("isFontSizeScaled", key) == 0)
	{
		lua_pushboolean(L, fIsFontSizeScaled ? 1 : 0);
	}
	else if (strcmp("resizeFontToFitHeight", key) == 0)
	{
		if (fIsSingleLine)
		{
			lua_pushcfunction(L, WinTextBoxObject::OnResizeFontToFitHeight);
		}
	}
	else if (strcmp("resizeHeightToFitFont", key) == 0)
	{
		if (fIsSingleLine)
		{
			lua_pushcfunction(L, WinTextBoxObject::OnResizeHeightToFitFont);
		}
	}
	else if (strcmp("setReturnKey", key) == 0)
	{
		lua_pushcfunction(L, WinTextBoxObject::OnSetReturnKey);
	}
	else if (strcmp("setTextColor", key) == 0)
	{
		lua_pushcfunction(L, WinTextBoxObject::OnSetTextColor);
	}
	else if (strcmp("setSelection", key) == 0)
	{
		lua_pushcfunction(L, WinTextBoxObject::OnSetSelection);
	}
	else if (strcmp("getSelection", key) == 0)
	{
		lua_pushcfunction(L, WinTextBoxObject::OnGetSelection);
	}
	else if (strcmp("align", key) == 0)
	{
		auto alignmentPointer = fTextBoxPointer->GetAlignment();
		if (!alignmentPointer)
		{
			alignmentPointer = &Interop::Graphics::HorizontalAlignment::kLeft;
		}
		lua_pushstring(L, alignmentPointer->GetCoronaStringId());
	}
	else if (strcmp("isSecure", key) == 0)
	{
		if (fIsSingleLine)
		{
			lua_pushboolean(L, fTextBoxPointer->IsSecure() ? 1 : 0);
		}
		else
		{
			result = 0;
		}
	}
	else if (strcmp("inputType", key) == 0)
	{
		if (fIsSingleLine)
		{
			if (fTextBoxPointer->IsNumericOnly())
			{
				lua_pushstring(L, "number");
			}
			else
			{
				lua_pushstring(L, "default");
			}
		}
		else
		{
			result = 0;
		}
	}
	else if (strcmp("isEditable", key) == 0)
	{
		if (!fIsSingleLine)
		{
			bool isReadOnly = fTextBoxPointer->IsReadOnly();
			lua_pushboolean(L, !isReadOnly ? 1 : 0);
		}
		else
		{
			result = 0;
		}
	}
	else if (strcmp("margin", key) == 0)
	{
		auto controlHeight = fTextBoxPointer->GetHeight();
		auto clientHeight = fTextBoxPointer->GetClientHeight();
		auto margin = (double)(controlHeight - clientHeight) / 2.0;
		margin++;
		margin *= (double)Rtt_RealToFloat(GetRuntimeEnvironment().GetRuntime()->GetDisplay().GetSxUpright());
		lua_pushnumber(L, margin);
	}
	else if (strcmp("placeholder", key) == 0)
	{
		// Fetch the UTF-16 encoded placeholder text from the control.
		std::wstring utf16Text;
		fTextBoxPointer->CopyPlaceholderTextTo(utf16Text);

		// Convert to UTF-8 and push to Lua.
		WinString stringTranscoder(utf16Text.c_str());
		auto utf8Text = stringTranscoder.GetUTF8();
		if (fIsPlaceholderTextNil && Rtt_StringIsEmpty(utf8Text))
		{
			lua_pushnil(L);
		}
		else
		{
			lua_pushstring(L, utf8Text);
		}
	}
	else if (!strcmp("autocorrectionType", key) || !strcmp("spellCheckingType", key) || !strcmp("clearButtonMode", key))
	{
		if (fIsSingleLine)
		{
			CoronaLuaWarning(L, "Native TextFields on Windows do not support the \"%s\" property.", key);
		}
		else
		{
			CoronaLuaWarning(L, "Native TextBoxes do not support the \"%s\" property.", key);
		}
		lua_pushnil(L);
	}
	else
	{
		result = Super::ValueForKey(L, key);
	}

	return result;
}

bool WinTextBoxObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
{
	// Validate.
	if (!fTextBoxPointer || Rtt_StringIsEmpty(key))
	{
		Rtt_ASSERT(0);
		return 0;
	}

	bool result = true;
	if (strcmp("text", key) == 0)
	{
		if (lua_type(L, valueIndex) == LUA_TSTRING)
		{
			WinString stringTranscoder;
			stringTranscoder.SetUTF8(lua_tostring(L, valueIndex));
			fTextBoxPointer->SetText(stringTranscoder.GetUTF16());
		}
		else
		{
			CoronaLuaError(
					L, "Invalid value type was assigned to the native %s.%s property.",
					fIsSingleLine ? "TextField" : "TextBox", key);
		}
	}
	else if (strcmp("size", key) == 0)
	{
		const auto luaType = lua_type(L, valueIndex);
		if ((luaType == LUA_TNUMBER) || (luaType == LUA_TNIL))
		{
			float fontSize = 0;
			if (luaType == LUA_TNUMBER)
			{
				fontSize = (float)lua_tonumber(L, valueIndex);
			}
			if (fontSize >= 1.0f)
			{
				if (fIsFontSizeScaled)
				{
					fontSize /= GetRuntimeEnvironment().GetRuntime()->GetDisplay().GetSxUpright();
				}
				else
				{
					Interop::Graphics::FontSizeConverter fontSizeConverter(fontSize, Gdiplus::UnitPoint);
					fontSizeConverter.ConvertTo(Gdiplus::UnitPixel);
					fontSize = fontSizeConverter.GetSize();
				}
			}
			else
			{
				fontSize = Rtt_RealToFloat(GetRuntimeEnvironment().GetPlatform()->GetStandardFontSize());
			}
			if (GetRuntimeEnvironment().GetDeviceSimulatorServices())
			{
				fontSize *= (float)fCachedSimulatorZoomScale;
			}
			HDC deviceContextHandle = ::GetDC(fTextBoxPointer->GetWindowHandle());
			if (deviceContextHandle)
			{
				Interop::Graphics::FontSettings fontSettings;
				fontSettings.CopyFrom(deviceContextHandle, fTextBoxPointer->GetFont());
				fontSettings.SetPixelSize(fontSize);
				auto font = Interop::Graphics::Font(
						deviceContextHandle, GetRuntimeEnvironment().GetFontServices(), fontSettings);
				fTextBoxPointer->SetFont(font);
				::ReleaseDC(fTextBoxPointer->GetWindowHandle(), deviceContextHandle);
			}
		}
		else
		{
			CoronaLuaError(
					L, "Invalid value type was assigned to the native %s.%s property.",
					fIsSingleLine ? "TextField" : "TextBox", key);
		}
	}
	else if (strcmp("font", key) == 0)
	{
		auto fontPointer = (WinFont*)LuaLibNative::ToFont(L, valueIndex);
		if (fontPointer)
		{
			Interop::Graphics::FontSettings fontSettings;
			fontSettings.CopyFrom(*fontPointer, Gdiplus::UnitPixel);
			if (fontSettings.GetPixelSize() >= Rtt_REAL_1)
			{
				if (fIsFontSizeScaled)
				{
					// Convert Corona scaled content size to pixels.
					auto fontSize = fontSettings.GetPixelSize();
					fontSize /= GetRuntimeEnvironment().GetRuntime()->GetDisplay().GetSxUpright();
					fontSettings.SetPixelSize(fontSize);
				}
				else
				{
					fontSettings.CopyFrom(*fontPointer, Gdiplus::UnitPoint);
				}
			}
			else
			{
				auto defaultFontSize = GetRuntimeEnvironment().GetPlatform()->GetStandardFontSize();
				fontSettings.SetPixelSize(Rtt_RealToFloat(defaultFontSize));
			}
			if (GetRuntimeEnvironment().GetDeviceSimulatorServices())
			{
				auto fontSize = fontSettings.GetPixelSize();
				fontSize *= (float)fCachedSimulatorZoomScale;
				fontSettings.SetPixelSize(fontSize);
			}
			HDC deviceContextHandle = ::GetDC(fTextBoxPointer->GetWindowHandle());
			auto font = Interop::Graphics::Font(
					deviceContextHandle, GetRuntimeEnvironment().GetFontServices(), fontSettings);
			fTextBoxPointer->SetFont(font);
			::ReleaseDC(fTextBoxPointer->GetWindowHandle(), deviceContextHandle);
		}
	}
	else if (strcmp("isFontSizeScaled", key) == 0)
	{
		if (lua_type(L, valueIndex) == LUA_TBOOLEAN)
		{
			fIsFontSizeScaled = lua_toboolean(L, valueIndex) ? true : false;
		}
		else
		{
			CoronaLuaError(L, "Invalid value type was assigned to the native TextField.%s property.", key);
		}
	}
	else if (strcmp("isSecure", key) == 0)
	{
		// This propery is only supported by single line text boxes.
		if (fIsSingleLine)
		{
			if (lua_type(L, valueIndex) == LUA_TBOOLEAN)
			{
				bool isSecure = lua_toboolean(L, valueIndex) ? true : false;
				fTextBoxPointer->SetSecure(isSecure);
			}
			else
			{
				CoronaLuaError(L, "Invalid value type was assigned to the native TextField.%s property.", key);
			}
		}
	}
	else if (strcmp("align", key) == 0)
	{
		if (lua_type(L, valueIndex) == LUA_TSTRING)
		{
			const char* alignmentStringId = lua_tostring(L, valueIndex);
			auto alignmentPointer = Interop::Graphics::HorizontalAlignment::FromCoronaStringId(alignmentStringId);
			if (alignmentPointer)
			{
				fTextBoxPointer->SetAlignment(*alignmentPointer);
			}
			else
			{
				CoronaLuaError(
						L, "Native %s.%s was assigned unknown key \"%s\".",
						fIsSingleLine ? "TextField" : "TextBox", key,
						alignmentStringId ? alignmentStringId : "");
			}
		}
		else
		{
			CoronaLuaError(
					L, "Invalid value type was assigned to the native %s.%s property.",
					fIsSingleLine ? "TextField" : "TextBox", key);
		}
	}
	else if (strcmp("inputType", key) == 0)
	{
		// This propery is only supported by single line text boxes.
		if (fIsSingleLine)
		{
			if (lua_type(L, valueIndex) == LUA_TSTRING)
			{
				auto inputTypeStringId = lua_tostring(L, valueIndex);
				if (Rtt_StringCompare("default", inputTypeStringId) == 0)
				{
					fTextBoxPointer->SetNumericOnly(false);
				}
				else if (Rtt_StringCompare("number", inputTypeStringId) == 0)
				{
					fTextBoxPointer->SetNumericOnly(true);
				}
				else if (Rtt_StringCompare("decimal", inputTypeStringId) == 0)
				{
					fTextBoxPointer->SetDecimalNumericOnly(true);
				}
				else if (Rtt_StringCompare("no-emoji", inputTypeStringId) == 0)
				{
					fTextBoxPointer->SetNoEmoji(true);
				}
				else if (inputTypeStringId)
				{
					CoronaLuaWarning(
							L, "Native TextField.%s key \"%s\" is not supported on Windows.", key, inputTypeStringId);
				}
			}
			else
			{
				CoronaLuaError(L, "Invalid value type was assigned to the native TextField.%s property.", key);
			}
		}
	}
	else if (strcmp("isEditable", key) == 0)
	{
		// This propery is only supported by multiline text boxes.
		if (!fIsSingleLine)
		{
			if (lua_type(L, valueIndex) == LUA_TBOOLEAN)
			{
				bool isEditable = lua_toboolean(L, valueIndex) ? true : false;
				fTextBoxPointer->SetReadOnly(!isEditable);
			}
			else
			{
				CoronaLuaError(L, "Invalid value type was assigned to the native TextBox.%s property.", key);
			}
		}
	}
	else if (strcmp("placeholder", key) == 0)
	{
		if (lua_type(L, valueIndex) == LUA_TSTRING)
		{
			WinString stringTranscoder;
			stringTranscoder.SetUTF8(lua_tostring(L, valueIndex));
			fTextBoxPointer->SetPlaceholderText(stringTranscoder.GetUTF16());
			fIsPlaceholderTextNil = false;
		}
		else if (lua_isnil(L, valueIndex))
		{
			fTextBoxPointer->SetPlaceholderText(nullptr);
			fIsPlaceholderTextNil = true;
		}
		else
		{
			CoronaLuaError(
					L, "Invalid value type was assigned to the native %s.%s property.",
					fIsSingleLine ? "TextField" : "TextBox", key);
		}
	}
	else if (!strcmp("autocorrectionType", key) || !strcmp("spellCheckingType", key) || !strcmp("clearButtonMode", key))
	{
		if (fIsSingleLine)
		{
			CoronaLuaWarning(L, "Native TextFields on Windows do not support the \"%s\" property.", key);
		}
		else
		{
			CoronaLuaWarning(L, "Native TextBoxes do not support the \"%s\" property.", key);
		}
	}
	else
	{
		result = Super::SetValueForKey(L, key, valueIndex);
	}

	return result;
}

#pragma endregion


#pragma region Private Static Functions
void WinTextBoxObject::OnResized(Interop::UI::Control& sender, const Interop::EventArgs& arguments)
{
	ApplySimulatorZoomScale();
}

void WinTextBoxObject::OnGainedFocus(Interop::UI::Control& sender, const Interop::EventArgs& arguments)
{
	// Ignore this event if this is a read-only text box.
	if (fTextBoxPointer->IsReadOnly())
	{
		return;
	}

	// Dispatch a Lua "userInput" event with phase "began".
	Rtt::UserInputEvent event(Rtt::UserInputEvent::kBegan);
	DispatchEventWithTarget(event);
}

void WinTextBoxObject::OnLostFocus(Interop::UI::Control& sender, const Interop::EventArgs& arguments)
{
	// Ignore this event if this is a read-only text box.
	if (fTextBoxPointer->IsReadOnly())
	{
		return;
	}

	// Dispatch a Lua "userInput" event with phase "ended".
	Rtt::UserInputEvent event(Rtt::UserInputEvent::kEnded);
	DispatchEventWithTarget(event);
}

void WinTextBoxObject::OnTextChanged(
	Interop::UI::TextBox& sender, const Interop::UI::UITextChangedEventArgs& arguments)
{
	// Create a string providing the characters added, if applicable.
	std::string addedString("");
	if (arguments.GetAddedCharacterCount() > 0)
	{
		auto newTextLength = (int)strlen(arguments.GetNewTextAsUtf8());
		if (arguments.GetPreviousStartSelectionIndex() < newTextLength)
		{
			addedString = arguments.GetNewTextAsUtf8() + arguments.GetPreviousStartSelectionIndex();
			if (arguments.GetAddedCharacterCount() < (int)addedString.length())
			{
				addedString.erase(arguments.GetAddedCharacterCount());
			}
		}
	}

	// Dispatch a Lua "userInput" event with phase "editing".
	Rtt::UserInputEvent event(
			arguments.GetPreviousStartSelectionIndex(), arguments.GetDeletedCharacterCount(),
			addedString.c_str(), arguments.GetPreviousTextAsUtf8(), arguments.GetNewTextAsUtf8());
	DispatchEventWithTarget(event);
}

void WinTextBoxObject::OnPressedEnterKey(Interop::UI::TextBox& sender, Interop::HandledEventArgs& arguments)
{
	// Ignore this event if this is a multiline text box.
	if (!fIsSingleLine)
	{
		return;
	}

	// Dispatch a Lua "userInput" event with phase "submitted".
	Rtt::UserInputEvent event(Rtt::UserInputEvent::kSubmitted);
	DispatchEventWithTarget(event);

	// Flag the enter key as handled.
	arguments.SetHandled();
}

void WinTextBoxObject::ApplySimulatorZoomScale()
{
	// Do not continue if not running under the simulator.
	auto deviceSimulatorServicesPointer = GetRuntimeEnvironment().GetDeviceSimulatorServices();
	if (!deviceSimulatorServicesPointer)
	{
		return;
	}

	// Fetch the current zoom scale.
	double currentZoomScale = deviceSimulatorServicesPointer->GetZoomScale();
	if (currentZoomScale <= 0)
	{
		currentZoomScale = 1.0;
	}

	// Do not continue if the zoom level hasn't changed.
	if (std::abs(currentZoomScale - fCachedSimulatorZoomScale) < DBL_EPSILON)
	{
		return;
	}

	// Update the text box's font size.
	HDC deviceContextHandle = ::GetDC(fTextBoxPointer->GetWindowHandle());
	if (deviceContextHandle)
	{
		// Fetch the text box's current font settings.
		Interop::Graphics::FontSettings fontSettings;
		fontSettings.CopyFrom(deviceContextHandle, fTextBoxPointer->GetFont());

		// Apply the zoom scale to the font size.
		double scaledFontSize = (double)fontSettings.GetPixelSize();
		scaledFontSize /= fCachedSimulatorZoomScale;
		scaledFontSize *= currentZoomScale;
		fontSettings.SetPixelSize((float)scaledFontSize);

		// Create a new scaled font and assign it to the text box.
		auto font = Interop::Graphics::Font(
				deviceContextHandle, GetRuntimeEnvironment().GetFontServices(), fontSettings);
		fTextBoxPointer->SetFont(font);
		::ReleaseDC(fTextBoxPointer->GetWindowHandle(), deviceContextHandle);
	}

	// Update the cached zoom scale. This must be done last.
	fCachedSimulatorZoomScale = currentZoomScale;
}

#pragma endregion


#pragma region Private Static Functions
int WinTextBoxObject::OnSetTextColor(lua_State *L)
{
	auto displayObjectPointer = (WinTextBoxObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
	{
		ColorUnion colorConverter;
		colorConverter.pixel = LuaLibDisplay::toColor(L, 2, displayObjectPointer->IsByteColorRange());
		COLORREF nativeColor = RGB(colorConverter.rgba.r, colorConverter.rgba.g, colorConverter.rgba.b);
		if (displayObjectPointer->fTextBoxPointer)
		{
			displayObjectPointer->fTextBoxPointer->SetTextColor(nativeColor);
		}
	}
	return 0;
}

int WinTextBoxObject::OnSetReturnKey(lua_State *L)
{
	return 0;
}

int WinTextBoxObject::OnSetSelection(lua_State *L)
{
	auto displayObjectPointer = (WinTextBoxObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
	{
		if ((lua_type(L, 2) == LUA_TNUMBER) && (lua_type(L, 3) == LUA_TNUMBER))
		{
			int startIndex = (int)lua_tointeger(L, 2);
			int endIndex = (int)lua_tointeger(L, 3);
			if (startIndex < 0)
			{
				startIndex = 0;
			}
			if (endIndex < 0)
			{
				endIndex = 0;
			}
			displayObjectPointer->fTextBoxPointer->SetSelection(startIndex, endIndex);
		}
	}
	return 0;
}

int WinTextBoxObject::OnGetSelection(lua_State *L)
{
	auto displayObjectPointer = (WinTextBoxObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
	{
		int startIndex = 0;
		int endIndex = 0;
		displayObjectPointer->fTextBoxPointer->GetSelection(&startIndex, &endIndex);
		
		lua_pushnumber(L, startIndex);
		lua_pushnumber(L, endIndex);
		return 2;
	}
	return 0;
}

int WinTextBoxObject::OnResizeFontToFitHeight(lua_State *L)
{
	auto displayObjectPointer = (WinTextBoxObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
	{
		auto textBoxPointer = displayObjectPointer->fTextBoxPointer;
		HDC deviceContextHandle = ::GetDC(textBoxPointer->GetWindowHandle());
		if (deviceContextHandle)
		{
			auto font = textBoxPointer->GetFont();
			auto result = font.CreateLogFontUsing(deviceContextHandle);
			if (result.HasSucceeded())
			{
				auto logFont = result.GetValue();
				logFont.lfHeight = textBoxPointer->GetClientHeight() - 2;
				if (logFont.lfHeight < 0)
				{
					logFont.lfHeight = 0;
				}
				Interop::Graphics::FontSettings fontSettings;
				fontSettings.CopyFrom(deviceContextHandle, logFont);
				font = Interop::Graphics::Font(
						deviceContextHandle,
						displayObjectPointer->GetRuntimeEnvironment().GetFontServices(),
						fontSettings);
				textBoxPointer->SetFont(font);
			}
			::ReleaseDC(displayObjectPointer->fTextBoxPointer->GetWindowHandle(), deviceContextHandle);
		}
	}
	return 0;
}

int WinTextBoxObject::OnResizeHeightToFitFont(lua_State *L)
{
	auto displayObjectPointer = (WinTextBoxObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
	{
		auto textBoxPointer = displayObjectPointer->fTextBoxPointer;
		HDC deviceContextHandle = ::GetDC(textBoxPointer->GetWindowHandle());
		if (deviceContextHandle)
		{
			int dpi = ::GetDeviceCaps(deviceContextHandle, LOGPIXELSY);
			auto fontHandle = textBoxPointer->GetFont().GetFontHandle();
			if (fontHandle && (dpi > 0))
			{
				TEXTMETRICW textMetrics{};
				auto previousFontHandle = ::SelectObject(deviceContextHandle, fontHandle);
				::GetTextMetricsW(deviceContextHandle, &textMetrics);
				::SelectObject(deviceContextHandle, previousFontHandle);
				auto oldPixelHeight = Rtt_IntToReal(textBoxPointer->GetHeight());
				auto newPixelHeight = Rtt_FloatToReal((float)textMetrics.tmHeight * ((float)dpi / 96.0f));
				newPixelHeight += oldPixelHeight - Rtt_IntToReal(textBoxPointer->GetClientHeight());
				newPixelHeight += Rtt_REAL_2;
				Rtt::Rect coronaBounds;
				displayObjectPointer->GetSelfBounds(coronaBounds);
				auto newContentHeight = Rtt_RealMul(coronaBounds.Height(), Rtt_RealDiv(newPixelHeight, oldPixelHeight));
				displayObjectPointer->SetSelfBounds(Rtt_REAL_NEG_1, newContentHeight);
			}
			::ReleaseDC(displayObjectPointer->fTextBoxPointer->GetWindowHandle(), deviceContextHandle);
		}
	}
	return 0;
}

#pragma endregion


} // namespace Rtt
