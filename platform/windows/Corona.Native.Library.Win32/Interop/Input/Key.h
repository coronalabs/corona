//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <Windows.h>
#include <Xinput.h>
#include <string>


#pragma region XInput Native Key Codes
// Define XInput's native key codes in case we're building with an older version of the "xinput.h" header.
// These key codes are returned by the XInputGetKeystroke() function.
#ifndef VK_PAD_A
#	define VK_PAD_A 0x5800
#endif
#ifndef VK_PAD_B
#	define VK_PAD_B 0x5801
#endif
#ifndef VK_PAD_X
#	define VK_PAD_X 0x5802
#endif
#ifndef VK_PAD_Y
#	define VK_PAD_Y 0x5803
#endif
#ifndef VK_PAD_RSHOULDER
#	define VK_PAD_RSHOULDER 0x5804
#endif
#ifndef VK_PAD_LSHOULDER
#	define VK_PAD_LSHOULDER 0x5805
#endif
#ifndef VK_PAD_LTRIGGER
#	define VK_PAD_LTRIGGER 0x5806
#endif
#ifndef VK_PAD_RTRIGGER
#	define VK_PAD_RTRIGGER 0x5807
#endif
#ifndef VK_PAD_DPAD_UP
#	define VK_PAD_DPAD_UP 0x5810
#endif
#ifndef VK_PAD_DPAD_DOWN
#	define VK_PAD_DPAD_DOWN 0x5811
#endif
#ifndef VK_PAD_DPAD_LEFT
#	define VK_PAD_DPAD_LEFT 0x5812
#endif
#ifndef VK_PAD_DPAD_RIGHT
#	define VK_PAD_DPAD_RIGHT 0x5813
#endif
#ifndef VK_PAD_START
#	define VK_PAD_START 0x5814
#endif
#ifndef VK_PAD_BACK
#	define VK_PAD_BACK 0x5815
#endif
#ifndef VK_PAD_LTHUMB_PRESS
#	define VK_PAD_LTHUMB_PRESS 0x5816
#endif
#ifndef VK_PAD_RTHUMB_PRESS
#	define VK_PAD_RTHUMB_PRESS 0x5817
#endif
#ifndef VK_PAD_LTHUMB_UP
#	define VK_PAD_LTHUMB_UP 0x5820
#endif
#ifndef VK_PAD_LTHUMB_DOWN
#	define VK_PAD_LTHUMB_DOWN 0x5821
#endif
#ifndef VK_PAD_LTHUMB_RIGHT
#	define VK_PAD_LTHUMB_RIGHT 0x5822
#endif
#ifndef VK_PAD_LTHUMB_LEFT
#	define VK_PAD_LTHUMB_LEFT 0x5823
#endif
#ifndef VK_PAD_LTHUMB_UPLEFT
#	define VK_PAD_LTHUMB_UPLEFT 0x5824
#endif
#ifndef VK_PAD_LTHUMB_UPRIGHT
#	define VK_PAD_LTHUMB_UPRIGHT 0x5825
#endif
#ifndef VK_PAD_LTHUMB_DOWNRIGHT
#	define VK_PAD_LTHUMB_DOWNRIGHT 0x5826
#endif
#ifndef VK_PAD_LTHUMB_DOWNLEFT
#	define VK_PAD_LTHUMB_DOWNLEFT 0x5827
#endif
#ifndef VK_PAD_RTHUMB_UP
#	define VK_PAD_RTHUMB_UP 0x5830
#endif
#ifndef VK_PAD_RTHUMB_DOWN
#	define VK_PAD_RTHUMB_DOWN 0x5831
#endif
#ifndef VK_PAD_RTHUMB_RIGHT
#	define VK_PAD_RTHUMB_RIGHT 0x5832
#endif
#ifndef VK_PAD_RTHUMB_LEFT
#	define VK_PAD_RTHUMB_LEFT 0x5833
#endif
#ifndef VK_PAD_RTHUMB_UPLEFT
#	define VK_PAD_RTHUMB_UPLEFT 0x5834
#endif
#ifndef VK_PAD_RTHUMB_UPRIGHT
#	define VK_PAD_RTHUMB_UPRIGHT 0x5835
#endif
#ifndef VK_PAD_RTHUMB_DOWNRIGHT
#	define VK_PAD_RTHUMB_DOWNRIGHT 0x5836
#endif
#ifndef VK_PAD_RTHUMB_DOWNLEFT
#	define VK_PAD_RTHUMB_DOWNLEFT 0x5837
#endif

#pragma endregion


#pragma region Corona Native Key Codes
// These are Corona Labs "made up" native key codes that Windows does not provide.
// They pair with commonly used Corona key names, such as gamepad buttons.
// Note: Microsoft native key codes are 16-bit integers.
//       So, Corona key code values must start at the 3rd byte to avoid collision.
enum
{
	VK_CORONA_BUTTON1 = 0x00010000,
	VK_CORONA_BUTTON2,
	VK_CORONA_BUTTON3,
	VK_CORONA_BUTTON4,
	VK_CORONA_BUTTON5,
	VK_CORONA_BUTTON6,
	VK_CORONA_BUTTON7,
	VK_CORONA_BUTTON8,
	VK_CORONA_BUTTON9,
	VK_CORONA_BUTTON10,
	VK_CORONA_BUTTON11,
	VK_CORONA_BUTTON12,
	VK_CORONA_BUTTON13,
	VK_CORONA_BUTTON14,
	VK_CORONA_BUTTON15,
	VK_CORONA_BUTTON16
};

#pragma endregion


namespace Interop { namespace Input {

/// <summary>
///  Provides information about a key such as its native integer key code and its unique Corona key name.
/// </summary>
class Key
{
	private:
		#pragma region Private Constructors
		/// <summary>Creates a new key with the given native key code.</summary>
		/// <remarks>
		///  This is a private constructor that is used to create "unknown" key objects given a native key code
		///  that Corona does not have a key name constant for.
		/// </remarks>
		/// <param name="nativeKeyCode">
		///  The platform's unique integer ID for this key such as VK_SPACE, VK_RETURN, etc.
		/// </param>
		Key(int nativeKeyCode);

		/// <summary>Creates a new key with the given information.</summary>
		/// <remarks>
		///  This is a private constructor that is used to create the predefined key objects this class
		///  makes available publicly via its static constants.
		/// </remarks>
		/// <param name="coronaKeyName">
		///  <para>
		///   Corona's unique name for this key. This is expected to come from a string constant in Corona's
		///   "Rtt::KeyName" class.
		///  </para>
		///  <para>If the native key code is unknown, then this string should be set to Rtt::KeyName::kUnknown.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <param name="nativeKeyCode">
		///  The platform's unique integer ID for this key such as VK_SPACE, VK_RETURN, etc.
		/// </param>
		Key(const char* coronaKeyName, int nativeKeyCode);

		#pragma endregion

	public:
		#pragma region Destructor
		/// <summary>Destroys this object.</summary>
		virtual ~Key();

		#pragma endregion


		#pragma region NativeCodeType Enum
		/// <summary>
		///  Determines if the native key code is a Win32 virtual key, an XInput gamepad key/button,
		///  or a Corona Labs "made up" key.
		/// </summary>
		enum class NativeCodeType
		{
			/// <summary>
			///  <para>This is a standard Windows virtual key code which comes from the keyboard or mouse.</para>
			///  <para>These keys typically comes from a WM_KEYDOWN and WM_KEYUP Windows message.</para>
			///  <para>These key codes will work with Win32 APIs such as MapVirtualKey(), SetKeyboardState(), etc.</para>
			/// </summary>
			kWin32,

			/// <summary>
			///  <para>This is a Microsoft XInput key code that comes from a game controller.</para>
			///  <para>These keys come from the XInputGetKeystroke() function.</para>
			///  <para>These key codes are not compatible with Win32 APIs.</para>
			/// </summary>
			kXInput,

			/// <summary>
			///  <para>This is a Corona Labs invented key code that is not provided by Microsoft.</para>
			///  <para>These key codes pair with commonly used Corona key names such as "button1", "button2", etc.</para>
			///  <para>These key codes are not compatible with Win32 APIs.</para>
			/// </summary>
			kCorona
		};

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>Gets the platform's unique integer ID for this key such as VK_SPACE, VK_RETURN, etc.</para>
		///  <para>Corona's "key" event in Lua provides this value via the "event.nativeKeyCode" property.</para>
		/// </summary>
		/// <returns>
		///  The platform's unique integer ID for this key. This matches Windows' "virtual key constants"
		///  such as VK_SPACE for the spacebar key.
		/// </returns>
		int GetNativeCodeValue() const;

		/// <summary>
		///  <para>Determines what "kind" of native key code is returned by the GetNativeCodeValue() function.</para>
		///  <para>Only key codes of type "kWin32" are compatible with Microsoft's core Win32 APIs.</para>
		/// </summary>
		/// <returns>Returns the key code type such as kWin32, kXInput, or kCorona.</returns>
		NativeCodeType GetNativeCodeType() const;

		/// <summary>
		///  <para>Gets Corona's unique string ID for this key.</para>
		///  <para>Corona's "key" event in Lua provides this name via the "event.keyName" property.</para>
		/// </summary>
		/// <returns>
		///  Returns Corona's unique string ID for this key such as "enter", "tab", "buttonA", "buttonB", etc.
		/// </returns>
		const char* GetCoronaName() const;

		/// <summary>Retrieves the character which the Key corresponds to based on keyboard layout.</summary>
		/// <returns>
		///  <para>Returns the character which the Key corresponds to.</para>
		/// </returns>
		std::string GetCharacter();

		/// <summary>Determines if this key matches the given key.</summary>
		/// <param name="value">The key to be compared with.</param>
		/// <returns>Return true if the keys match. Returns false if not.</returns>
		bool Equals(const Key& value) const;

		/// <summary>Determines if this key does not match the given key.</summary>
		/// <param name"value">The key to be compared with.</param>
		/// <returns>Return true if the keys do not match. Returns false if they do match.</returns>
		bool NotEquals(const Key& value) const;

		/// <summary>Determines if this key matches the given key.</summary>
		/// <param name="value">The key to be compared with.</param>
		/// <returns>Return true if the keys match. Returns false if not.</returns>
		bool operator==(const Key& value) const;

		/// <summary>Determines if this key does not match the given key.</summary>
		/// <param name"value">The key to be compared with.</param>
		/// <returns>Return true if the keys do not match. Returns false if they do match.</returns>
		bool operator!=(const Key& value) const;

		#pragma endregion


		#pragma region Public Static Functions
		/// <summary>Fetches a key matching the given unique Corona key name.</summary>
		/// <param name="name">
		///  <para>The unique Corona string ID such as "space", "enter", "buttonA", etc.</para>
		///  <para>This is expected to match a string constant in Corona's Rtt::KeyName class.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a pointer to a matching Key constant such as kSpace, kEnter, etc.</para>
		///  <para>Returns null if the given Corona key name is "unknown" or not recognized.</para>
		/// </returns>
		static const Key* FromCoronaName(const char* name);

		/// <summary>Creates a key object from the given platform specific native key code.</summary>
		/// <param name="nativeKeyCode">
		///  The platform's unique integer ID for this key such as VK_SPACE, VK_RETURN, etc.
		/// </param>
		/// <returns>
		///  Returns a key object with the given native key code and Corona's matching unique string ID
		///  used by the Lua key events.
		/// </returns>
		static Key FromNativeCode(int value);

		#pragma endregion


		#pragma region Public Key Constants
		static const Key kA;
		static const Key kB;
		static const Key kC;
		static const Key kD;
		static const Key kE;
		static const Key kF;
		static const Key kG;
		static const Key kH;
		static const Key kI;
		static const Key kJ;
		static const Key kK;
		static const Key kL;
		static const Key kM;
		static const Key kN;
		static const Key kO;
		static const Key kP;
		static const Key kQ;
		static const Key kR;
		static const Key kS;
		static const Key kT;
		static const Key kU;
		static const Key kV;
		static const Key kW;
		static const Key kX;
		static const Key kY;
		static const Key kZ;

		static const Key k0;
		static const Key k1;
		static const Key k2;
		static const Key k3;
		static const Key k4;
		static const Key k5;
		static const Key k6;
		static const Key k7;
		static const Key k8;
		static const Key k9;

		static const Key kNumPad0;
		static const Key kNumPad1;
		static const Key kNumPad2;
		static const Key kNumPad3;
		static const Key kNumPad4;
		static const Key kNumPad5;
		static const Key kNumPad6;
		static const Key kNumPad7;
		static const Key kNumPad8;
		static const Key kNumPad9;

		static const Key kNumPadAdd;
		static const Key kNumPadSubtract;
		static const Key kNumPadMultiply;
		static const Key kNumPadDivide;
		static const Key kNumPadPeriod;
		static const Key kNumPadComma;

		static const Key kUp;
		static const Key kDown;
		static const Key kLeft;
		static const Key kRight;

		static const Key kLeftAlt;
		static const Key kRightAlt;
		static const Key kLeftControl;
		static const Key kRightControl;
		static const Key kLeftShift;
		static const Key kRightShift;
		static const Key kLeftCommand;
		static const Key kRightCommand;

		static const Key kF1;
		static const Key kF2;
		static const Key kF3;
		static const Key kF4;
		static const Key kF5;
		static const Key kF6;
		static const Key kF7;
		static const Key kF8;
		static const Key kF9;
		static const Key kF10;
		static const Key kF11;
		static const Key kF12;
		static const Key kF13;
		static const Key kF14;
		static const Key kF15;
		static const Key kF16;
		static const Key kF17;
		static const Key kF18;
		static const Key kF19;
		static const Key kF20;
		static const Key kF21;
		static const Key kF22;
		static const Key kF23;
		static const Key kF24;

		static const Key kBack;
		static const Key kForward;

		static const Key kMediaPlay;
		static const Key kMediaPause;
		static const Key kMediaPlayPause;
		static const Key kMediaStop;
		static const Key kMediaNext;
		static const Key kMediaPrevious;

		static const Key kVolumeUp;
		static const Key kVolumeDown;
		static const Key kVolumeMute;

		static const Key kBookmark;
		static const Key kMail;
		static const Key kMusic;
		static const Key kSearch;

		static const Key kApostrophe;
		static const Key kBackSlash;
		static const Key kForwardSlash;
		static const Key kBreak;
		static const Key kCapsLock;
		static const Key kClear;
		static const Key kComma;
		static const Key kDeleteBack;
		static const Key kDeleteForward;
		static const Key kEnter;
		static const Key kEscape;
		static const Key kBackTick;
		static const Key kInsert;
		static const Key kLeftBracket;
		static const Key kRightBracket;
		static const Key kNumLock;
		static const Key kPlus;
		static const Key kMinus;
		static const Key kHome;
		static const Key kEnd;
		static const Key kPageUp;
		static const Key kPageDown;
		static const Key kPeriod;
		static const Key kScrollLock;
		static const Key kSemicolon;
		static const Key kSpace;
		static const Key kPrintScreen;
		static const Key kTab;

		static const Key kButton1;
		static const Key kButton2;
		static const Key kButton3;
		static const Key kButton4;
		static const Key kButton5;
		static const Key kButton6;
		static const Key kButton7;
		static const Key kButton8;
		static const Key kButton9;
		static const Key kButton10;
		static const Key kButton11;
		static const Key kButton12;
		static const Key kButton13;
		static const Key kButton14;
		static const Key kButton15;
		static const Key kButton16;
		static const Key kButtonA;
		static const Key kButtonB;
		static const Key kButtonX;
		static const Key kButtonY;
		static const Key kButtonSelect;
		static const Key kButtonStart;
		static const Key kLeftShoulderButton1;
		static const Key kRightShoulderButton1;
		static const Key kLeftJoystickButton;
		static const Key kRightJoystickButton;

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>The unique Corona string ID for this key.</summary>
		const char* fCoronaName;

		/// <summary>The platform's native key code such as VK_SPACE, VK_RETURN, etc.</summary>
		int fNativeCode;

		#pragma endregion
};

} }	// namespace Interop::Input


namespace std {

#pragma region std::hash<Key> Template Specialization
template<>
/// <summary>
///  <para>Template specialization of "std::hash" for the Key class.</para>
///  <para>Provides a reasonably unique hash code for a Key object.</para>
///  <para>
///   Intended to be used by STL collections that store Key objects in a hash table such as
///   the std::unordered_map, std::unordered_set, etc.
///  </para>
/// </summary>
struct hash<Interop::Input::Key>
{
	/// <summary>Gets a reasonably unique hash code for the given object.</summary>
	/// <param name="value">The object to calculate a hash code for.</param>
	/// <returns>Returns a reasonably unique hash code for the given object.</returns>
	size_t operator()(const Interop::Input::Key& value) const
	{
		return (size_t)value.GetNativeCodeValue();
	}
};

#pragma endregion

}	// namespace std
