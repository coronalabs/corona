//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Key.h"
#include "Core\Rtt_Assert.h"
#include "Rtt_KeyName.h"
#include <unordered_map>
#include "WinString.h"

namespace Interop { namespace Input {

#pragma region StlStringPointerHashProvider Struct
struct StlStringPointerHashProvider : public std::hash<const char*>
{
	size_t operator()(const char* stringPointer) const
	{
		size_t hashValue = 0;
		if (stringPointer)
		{
			// Hash all of the characters in the string, excluding the null termination character.
			// Note: Null and empty strings will have the same hash value.
			for (; *stringPointer != '\0'; stringPointer++)
			{
				// Rotate the hash's bits left by one character to ensure all bytes in the hash are used.
#ifdef _WIN64
				hashValue = RotateLeft64(hashValue, 8);
#else
				hashValue = RotateLeft32(hashValue, 8);
#endif

				// XOR the next character in the string to the hash's first byte.
				hashValue ^= (size_t)(*stringPointer);
			}
		}
		return hashValue;
	}
};

#pragma endregion


#pragma region StlStringPointerEqualityComparer Struct
struct StlStringPointerEqualityComparer : public std::equal_to<const char*>
{
	bool operator()(const char* string1, const char* string2) const
	{
		// If the string pointers match or they are both null, then they are equal.
		if (string1 == string2)
		{
			return true;
		}

		// If one of the string pointers are null and the other is not, then they are not equal.
		// Note: Both pointers can't be null at this point due to the above check.
		if (!string1 || !string2)
		{
			return false;
		}

		// Check if all characters match.
		return (strcmp(string1, string2) == 0);
	}
};

#pragma endregion


#pragma region Static Member Variables
/// <summary>Key/Value pair used by the "sNativeCodeToKeyCollection" static variable.</summary>
typedef std::pair<int, const Key*> NativeCodeToKeyPair;

/// <summary>Key/Value pair used by the "sCoronaNameToKeyCollection" static variable.</summary>
typedef std::pair<const char*, const Key*> CoronaNameToKeyPair;

/// <summary>
///  <para>Hash table storing pointers to all predefined Key constants such as kSpace, kUp, kDown, etc.</para>
///  <para>Uses the platform's "native key code" as the hash table key for quick lookups.</para>
/// </summary>
/// <remarks>This static variable must be declared before the Key constant declarations.</remarks>
static std::unordered_map<int, const Key*> sNativeCodeToKeyCollection;

/// <summary>
///  <para>Hash table storing pointers to all predefined Key constants such as kSpace, kUp, kDown, etc.</para>
///  <para>
///   Uses the unique Corona key name (such as "space", "up", "down", etc.) as the hash table key for quick lookups.
///  </para>
/// </summary>
/// <remarks>This static variable must be declared before the Key constant declarations.</remarks>
static std::unordered_map<const char*, const Key*, StlStringPointerHashProvider, StlStringPointerEqualityComparer> sCoronaNameToKeyCollection;

/// <summary>Defines a Corona "Key" constant declared in the "Key.h" header file.</summary>
/// <param name="constantName">
///  <para>The name of the Key class' declared constant such as kUp, kDown, etc.</para>
///  <para>This constant name must also exactly match a constant name in the Rtt::KeyName class.</para>
/// </param>
/// <param name="nativeKeyCode">The platform's unique integer code such as VK_UP, VK_DOWN, etc.</param>
#define CORONA_KEY_CONSTANT_DEFINITION(constantName, nativeKeyCode) \
	const Key Key::##constantName(Rtt::KeyName::##constantName, nativeKeyCode);

CORONA_KEY_CONSTANT_DEFINITION(kA, (int)'A');
CORONA_KEY_CONSTANT_DEFINITION(kB, (int)'B');
CORONA_KEY_CONSTANT_DEFINITION(kC, (int)'C');
CORONA_KEY_CONSTANT_DEFINITION(kD, (int)'D');
CORONA_KEY_CONSTANT_DEFINITION(kE, (int)'E');
CORONA_KEY_CONSTANT_DEFINITION(kF, (int)'F');
CORONA_KEY_CONSTANT_DEFINITION(kG, (int)'G');
CORONA_KEY_CONSTANT_DEFINITION(kH, (int)'H');
CORONA_KEY_CONSTANT_DEFINITION(kI, (int)'I');
CORONA_KEY_CONSTANT_DEFINITION(kJ, (int)'J');
CORONA_KEY_CONSTANT_DEFINITION(kK, (int)'K');
CORONA_KEY_CONSTANT_DEFINITION(kL, (int)'L');
CORONA_KEY_CONSTANT_DEFINITION(kM, (int)'M');
CORONA_KEY_CONSTANT_DEFINITION(kN, (int)'N');
CORONA_KEY_CONSTANT_DEFINITION(kO, (int)'O');
CORONA_KEY_CONSTANT_DEFINITION(kP, (int)'P');
CORONA_KEY_CONSTANT_DEFINITION(kQ, (int)'Q');
CORONA_KEY_CONSTANT_DEFINITION(kR, (int)'R');
CORONA_KEY_CONSTANT_DEFINITION(kS, (int)'S');
CORONA_KEY_CONSTANT_DEFINITION(kT, (int)'T');
CORONA_KEY_CONSTANT_DEFINITION(kU, (int)'U');
CORONA_KEY_CONSTANT_DEFINITION(kV, (int)'V');
CORONA_KEY_CONSTANT_DEFINITION(kW, (int)'W');
CORONA_KEY_CONSTANT_DEFINITION(kX, (int)'X');
CORONA_KEY_CONSTANT_DEFINITION(kY, (int)'Y');
CORONA_KEY_CONSTANT_DEFINITION(kZ, (int)'Z');

CORONA_KEY_CONSTANT_DEFINITION(k0, (int)'0');
CORONA_KEY_CONSTANT_DEFINITION(k1, (int)'1');
CORONA_KEY_CONSTANT_DEFINITION(k2, (int)'2');
CORONA_KEY_CONSTANT_DEFINITION(k3, (int)'3');
CORONA_KEY_CONSTANT_DEFINITION(k4, (int)'4');
CORONA_KEY_CONSTANT_DEFINITION(k5, (int)'5');
CORONA_KEY_CONSTANT_DEFINITION(k6, (int)'6');
CORONA_KEY_CONSTANT_DEFINITION(k7, (int)'7');
CORONA_KEY_CONSTANT_DEFINITION(k8, (int)'8');
CORONA_KEY_CONSTANT_DEFINITION(k9, (int)'9');

CORONA_KEY_CONSTANT_DEFINITION(kNumPad0, VK_NUMPAD0);
CORONA_KEY_CONSTANT_DEFINITION(kNumPad1, VK_NUMPAD1);
CORONA_KEY_CONSTANT_DEFINITION(kNumPad2, VK_NUMPAD2);
CORONA_KEY_CONSTANT_DEFINITION(kNumPad3, VK_NUMPAD3);
CORONA_KEY_CONSTANT_DEFINITION(kNumPad4, VK_NUMPAD4);
CORONA_KEY_CONSTANT_DEFINITION(kNumPad5, VK_NUMPAD5);
CORONA_KEY_CONSTANT_DEFINITION(kNumPad6, VK_NUMPAD6);
CORONA_KEY_CONSTANT_DEFINITION(kNumPad7, VK_NUMPAD7);
CORONA_KEY_CONSTANT_DEFINITION(kNumPad8, VK_NUMPAD8);
CORONA_KEY_CONSTANT_DEFINITION(kNumPad9, VK_NUMPAD9);

CORONA_KEY_CONSTANT_DEFINITION(kNumPadAdd, VK_ADD);
CORONA_KEY_CONSTANT_DEFINITION(kNumPadSubtract, VK_SUBTRACT);
CORONA_KEY_CONSTANT_DEFINITION(kNumPadMultiply, VK_MULTIPLY);
CORONA_KEY_CONSTANT_DEFINITION(kNumPadDivide, VK_DIVIDE);
CORONA_KEY_CONSTANT_DEFINITION(kNumPadPeriod, VK_DECIMAL);
CORONA_KEY_CONSTANT_DEFINITION(kNumPadComma, VK_SEPARATOR);

CORONA_KEY_CONSTANT_DEFINITION(kUp, VK_UP);
CORONA_KEY_CONSTANT_DEFINITION(kDown, VK_DOWN);
CORONA_KEY_CONSTANT_DEFINITION(kLeft, VK_LEFT);
CORONA_KEY_CONSTANT_DEFINITION(kRight, VK_RIGHT);

CORONA_KEY_CONSTANT_DEFINITION(kLeftAlt, VK_LMENU);
CORONA_KEY_CONSTANT_DEFINITION(kRightAlt, VK_RMENU);
CORONA_KEY_CONSTANT_DEFINITION(kLeftControl, VK_LCONTROL);
CORONA_KEY_CONSTANT_DEFINITION(kRightControl, VK_RCONTROL);
CORONA_KEY_CONSTANT_DEFINITION(kLeftShift, VK_LSHIFT);
CORONA_KEY_CONSTANT_DEFINITION(kRightShift, VK_RSHIFT);
CORONA_KEY_CONSTANT_DEFINITION(kLeftCommand, VK_LWIN);
CORONA_KEY_CONSTANT_DEFINITION(kRightCommand, VK_RWIN);

CORONA_KEY_CONSTANT_DEFINITION(kF1, VK_F1);
CORONA_KEY_CONSTANT_DEFINITION(kF2, VK_F2);
CORONA_KEY_CONSTANT_DEFINITION(kF3, VK_F3);
CORONA_KEY_CONSTANT_DEFINITION(kF4, VK_F4);
CORONA_KEY_CONSTANT_DEFINITION(kF5, VK_F5);
CORONA_KEY_CONSTANT_DEFINITION(kF6, VK_F6);
CORONA_KEY_CONSTANT_DEFINITION(kF7, VK_F7);
CORONA_KEY_CONSTANT_DEFINITION(kF8, VK_F8);
CORONA_KEY_CONSTANT_DEFINITION(kF9, VK_F9);
CORONA_KEY_CONSTANT_DEFINITION(kF10, VK_F10);
CORONA_KEY_CONSTANT_DEFINITION(kF11, VK_F11);
CORONA_KEY_CONSTANT_DEFINITION(kF12, VK_F12);
CORONA_KEY_CONSTANT_DEFINITION(kF13, VK_F13);
CORONA_KEY_CONSTANT_DEFINITION(kF14, VK_F14);
CORONA_KEY_CONSTANT_DEFINITION(kF15, VK_F15);
CORONA_KEY_CONSTANT_DEFINITION(kF16, VK_F16);
CORONA_KEY_CONSTANT_DEFINITION(kF17, VK_F17);
CORONA_KEY_CONSTANT_DEFINITION(kF18, VK_F18);
CORONA_KEY_CONSTANT_DEFINITION(kF19, VK_F19);
CORONA_KEY_CONSTANT_DEFINITION(kF20, VK_F20);
CORONA_KEY_CONSTANT_DEFINITION(kF21, VK_F21);
CORONA_KEY_CONSTANT_DEFINITION(kF22, VK_F22);
CORONA_KEY_CONSTANT_DEFINITION(kF23, VK_F23);
CORONA_KEY_CONSTANT_DEFINITION(kF24, VK_F24);

CORONA_KEY_CONSTANT_DEFINITION(kBack, VK_BROWSER_BACK);
CORONA_KEY_CONSTANT_DEFINITION(kForward, VK_BROWSER_FORWARD);

CORONA_KEY_CONSTANT_DEFINITION(kMediaPlay, VK_PLAY);
CORONA_KEY_CONSTANT_DEFINITION(kMediaPause, VK_PAUSE);
CORONA_KEY_CONSTANT_DEFINITION(kMediaPlayPause, VK_MEDIA_PLAY_PAUSE);
CORONA_KEY_CONSTANT_DEFINITION(kMediaStop, VK_MEDIA_STOP);
CORONA_KEY_CONSTANT_DEFINITION(kMediaNext, VK_MEDIA_NEXT_TRACK);
CORONA_KEY_CONSTANT_DEFINITION(kMediaPrevious, VK_MEDIA_PREV_TRACK);

CORONA_KEY_CONSTANT_DEFINITION(kVolumeUp, VK_VOLUME_UP);
CORONA_KEY_CONSTANT_DEFINITION(kVolumeDown, VK_VOLUME_DOWN);
CORONA_KEY_CONSTANT_DEFINITION(kVolumeMute, VK_VOLUME_MUTE);

CORONA_KEY_CONSTANT_DEFINITION(kBookmark, VK_BROWSER_FAVORITES);
CORONA_KEY_CONSTANT_DEFINITION(kMail, VK_LAUNCH_MAIL);
CORONA_KEY_CONSTANT_DEFINITION(kMusic, VK_LAUNCH_MEDIA_SELECT);
CORONA_KEY_CONSTANT_DEFINITION(kSearch, VK_BROWSER_SEARCH);

CORONA_KEY_CONSTANT_DEFINITION(kApostrophe, VK_OEM_7);
CORONA_KEY_CONSTANT_DEFINITION(kBackSlash, VK_OEM_5);
CORONA_KEY_CONSTANT_DEFINITION(kForwardSlash, VK_OEM_2);
CORONA_KEY_CONSTANT_DEFINITION(kBreak, VK_CANCEL);
CORONA_KEY_CONSTANT_DEFINITION(kCapsLock, VK_CAPITAL);
CORONA_KEY_CONSTANT_DEFINITION(kClear, VK_CLEAR);
CORONA_KEY_CONSTANT_DEFINITION(kComma, VK_OEM_COMMA);
CORONA_KEY_CONSTANT_DEFINITION(kDeleteBack, VK_BACK);
CORONA_KEY_CONSTANT_DEFINITION(kDeleteForward, VK_DELETE);
CORONA_KEY_CONSTANT_DEFINITION(kEnter, VK_RETURN);
CORONA_KEY_CONSTANT_DEFINITION(kEscape, VK_ESCAPE);
CORONA_KEY_CONSTANT_DEFINITION(kBackTick, VK_OEM_3);
CORONA_KEY_CONSTANT_DEFINITION(kInsert, VK_INSERT);
CORONA_KEY_CONSTANT_DEFINITION(kLeftBracket, VK_OEM_4);
CORONA_KEY_CONSTANT_DEFINITION(kRightBracket, VK_OEM_6);
CORONA_KEY_CONSTANT_DEFINITION(kNumLock, VK_NUMLOCK);
CORONA_KEY_CONSTANT_DEFINITION(kPlus, VK_OEM_PLUS);
CORONA_KEY_CONSTANT_DEFINITION(kMinus, VK_OEM_MINUS);
CORONA_KEY_CONSTANT_DEFINITION(kHome, VK_HOME);
CORONA_KEY_CONSTANT_DEFINITION(kEnd, VK_END);
CORONA_KEY_CONSTANT_DEFINITION(kPageUp, VK_PRIOR);
CORONA_KEY_CONSTANT_DEFINITION(kPageDown, VK_NEXT);
CORONA_KEY_CONSTANT_DEFINITION(kPeriod, VK_OEM_PERIOD);
CORONA_KEY_CONSTANT_DEFINITION(kScrollLock, VK_SCROLL);
CORONA_KEY_CONSTANT_DEFINITION(kSemicolon, VK_OEM_1);
CORONA_KEY_CONSTANT_DEFINITION(kSpace, VK_SPACE);
CORONA_KEY_CONSTANT_DEFINITION(kPrintScreen, VK_SNAPSHOT);
CORONA_KEY_CONSTANT_DEFINITION(kTab, VK_TAB);

CORONA_KEY_CONSTANT_DEFINITION(kButton1, VK_CORONA_BUTTON1);
CORONA_KEY_CONSTANT_DEFINITION(kButton2, VK_CORONA_BUTTON2);
CORONA_KEY_CONSTANT_DEFINITION(kButton3, VK_CORONA_BUTTON3);
CORONA_KEY_CONSTANT_DEFINITION(kButton4, VK_CORONA_BUTTON4);
CORONA_KEY_CONSTANT_DEFINITION(kButton5, VK_CORONA_BUTTON5);
CORONA_KEY_CONSTANT_DEFINITION(kButton6, VK_CORONA_BUTTON6);
CORONA_KEY_CONSTANT_DEFINITION(kButton7, VK_CORONA_BUTTON7);
CORONA_KEY_CONSTANT_DEFINITION(kButton8, VK_CORONA_BUTTON8);
CORONA_KEY_CONSTANT_DEFINITION(kButton9, VK_CORONA_BUTTON9);
CORONA_KEY_CONSTANT_DEFINITION(kButton10, VK_CORONA_BUTTON10);
CORONA_KEY_CONSTANT_DEFINITION(kButton11, VK_CORONA_BUTTON11);
CORONA_KEY_CONSTANT_DEFINITION(kButton12, VK_CORONA_BUTTON12);
CORONA_KEY_CONSTANT_DEFINITION(kButton13, VK_CORONA_BUTTON13);
CORONA_KEY_CONSTANT_DEFINITION(kButton14, VK_CORONA_BUTTON14);
CORONA_KEY_CONSTANT_DEFINITION(kButton15, VK_CORONA_BUTTON15);
CORONA_KEY_CONSTANT_DEFINITION(kButton16, VK_CORONA_BUTTON16);
CORONA_KEY_CONSTANT_DEFINITION(kButtonA, VK_PAD_A);
CORONA_KEY_CONSTANT_DEFINITION(kButtonB, VK_PAD_B);
CORONA_KEY_CONSTANT_DEFINITION(kButtonX, VK_PAD_X);
CORONA_KEY_CONSTANT_DEFINITION(kButtonY, VK_PAD_Y);
CORONA_KEY_CONSTANT_DEFINITION(kButtonSelect, VK_PAD_BACK);
CORONA_KEY_CONSTANT_DEFINITION(kButtonStart, VK_PAD_START);
CORONA_KEY_CONSTANT_DEFINITION(kLeftShoulderButton1, VK_PAD_LSHOULDER);
CORONA_KEY_CONSTANT_DEFINITION(kRightShoulderButton1, VK_PAD_RSHOULDER);
CORONA_KEY_CONSTANT_DEFINITION(kLeftJoystickButton, VK_PAD_LTHUMB_PRESS);
CORONA_KEY_CONSTANT_DEFINITION(kRightJoystickButton, VK_PAD_RTHUMB_PRESS);

#pragma endregion


#pragma region Constructors/Destructors
Key::Key(int nativeKeyCode)
:	fCoronaName(Rtt::KeyName::kUnknown),
	fNativeCode(nativeKeyCode)
{
	// If there is a known Corona key name for the given native key code, then use it.
	// Otherwise, leave the Corona key name as "unknown".
	auto iterator = sNativeCodeToKeyCollection.find(nativeKeyCode);
	if (iterator != sNativeCodeToKeyCollection.end())
	{
		fCoronaName = (*iterator).second->fCoronaName;
	}
}

Key::Key(const char* coronaKeyName, int nativeKeyCode)
:	fCoronaName(coronaKeyName),
	fNativeCode(nativeKeyCode)
{
#if Rtt_DEBUG
	// For debug builds...
	// Assert if the given key code or key name has already been added to this class' static collections.
	// We only support a 1-to-1 mapping of key codes and key names. No duplicates allowed.
	// This is intended to catch copy-and-paste mistakes when new key constants are made.
	{
		auto iterator = sNativeCodeToKeyCollection.find(nativeKeyCode);
		Rtt_ASSERT(iterator == sNativeCodeToKeyCollection.end());
	}
	{
		auto iterator = sCoronaNameToKeyCollection.find(coronaKeyName);
		Rtt_ASSERT(iterator == sCoronaNameToKeyCollection.end());
	}
#endif

	// Add this predefined Key object to the static collections for fast lookup later.
	sNativeCodeToKeyCollection.insert(NativeCodeToKeyPair(nativeKeyCode, this));
	sCoronaNameToKeyCollection.insert(CoronaNameToKeyPair(coronaKeyName, this));
}

Key::~Key()
{
}

#pragma endregion


#pragma region Public Methods
int Key::GetNativeCodeValue() const
{
	return fNativeCode;
}

Key::NativeCodeType Key::GetNativeCodeType() const
{
	if (fNativeCode & 0xFFFF0000)
	{
		return NativeCodeType::kCorona;
	}
	else if (fNativeCode & 0x5800)
	{
		return NativeCodeType::kXInput;
	}
	return NativeCodeType::kWin32;
}

const char* Key::GetCoronaName() const
{
	return fCoronaName;
}

std::string Key::GetCharacter()
{
	std::string sKeyboardState;
	sKeyboardState.reserve(256);
	unsigned char *keyboardState = (unsigned char*)sKeyboardState.data();
	GetKeyboardState(keyboardState);

	int scanCode = MapVirtualKey(this->GetNativeCodeValue(), 0x0); // 0x0 is MAPVK_VK_TO_VSC
	if (scanCode == 0)
	{
		return std::string();
	}
	else
	{
		const int BUFFER_LENGTH = 10;
		wchar_t chars[BUFFER_LENGTH] = {0};

		switch (ToUnicode(this->GetNativeCodeValue(), scanCode, keyboardState, chars, BUFFER_LENGTH - 1, 0))
		{
		case -1:
		case 0:
			return std::string();
		default:
			WinString stringConverter;
			stringConverter.SetUTF16(chars);
			return std::string(stringConverter.GetUTF8());
		}
	}
}

bool Key::Equals(const Key& value) const
{
	return (fNativeCode == value.fNativeCode);
}

bool Key::NotEquals(const Key& value) const
{
	return !(this->Equals(value));
}

bool Key::operator==(const Key& value) const
{
	return this->Equals(value);
}

bool Key::operator!=(const Key& value) const
{
	return this->NotEquals(value);
}

#pragma endregion


#pragma region Public Static Functions
const Key* Key::FromCoronaName(const char* name)
{
	// Do not continue if given a null or empty string.
	if (!name || ('\0' == name[0]))
	{
		return nullptr;
	}

	// Fetch a Key constant by its unique Corona key name.
	auto iterator = sCoronaNameToKeyCollection.find(name);
	if (iterator != sCoronaNameToKeyCollection.end())
	{
		return (*iterator).second;
	}

	// The given key name was not found.
	return nullptr;
}

Key Key::FromNativeCode(int value)
{
	return Key(value);
}

#pragma endregion

} }	// namespace Interop::Input
