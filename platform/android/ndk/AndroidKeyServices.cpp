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


#include "AndroidKeyServices.h"
#include "Rtt_KeyName.h"
#include <hash_map>


/// Android key code and name mapping type.
typedef std::hash_map<int, const char*> AndroidKeyCollection;

/// Map entry type for instances of the AndroidKeyCollection type.
typedef std::pair<int, const char*> AndroidKeyCollectionPair;


/// Gets a static collection which stores a mapping between Android key codes and Corona's key names.
/// @return Returns a reference to the static key mapping.
static AndroidKeyCollection& GetCollection()
{
	static AndroidKeyCollection sKeyCollection;

	// Populate the key collection, if not done already.
	// The key values match the KEYCODE constants in Android's "KeyEvent" class in Java.
	if (sKeyCollection.empty())
	{
		sKeyCollection.insert(AndroidKeyCollectionPair(29, Rtt::KeyName::kA));
		sKeyCollection.insert(AndroidKeyCollectionPair(30, Rtt::KeyName::kB));
		sKeyCollection.insert(AndroidKeyCollectionPair(31, Rtt::KeyName::kC));
		sKeyCollection.insert(AndroidKeyCollectionPair(32, Rtt::KeyName::kD));
		sKeyCollection.insert(AndroidKeyCollectionPair(33, Rtt::KeyName::kE));
		sKeyCollection.insert(AndroidKeyCollectionPair(34, Rtt::KeyName::kF));
		sKeyCollection.insert(AndroidKeyCollectionPair(35, Rtt::KeyName::kG));
		sKeyCollection.insert(AndroidKeyCollectionPair(36, Rtt::KeyName::kH));
		sKeyCollection.insert(AndroidKeyCollectionPair(37, Rtt::KeyName::kI));
		sKeyCollection.insert(AndroidKeyCollectionPair(38, Rtt::KeyName::kJ));
		sKeyCollection.insert(AndroidKeyCollectionPair(39, Rtt::KeyName::kK));
		sKeyCollection.insert(AndroidKeyCollectionPair(40, Rtt::KeyName::kL));
		sKeyCollection.insert(AndroidKeyCollectionPair(41, Rtt::KeyName::kM));
		sKeyCollection.insert(AndroidKeyCollectionPair(42, Rtt::KeyName::kN));
		sKeyCollection.insert(AndroidKeyCollectionPair(43, Rtt::KeyName::kO));
		sKeyCollection.insert(AndroidKeyCollectionPair(44, Rtt::KeyName::kP));
		sKeyCollection.insert(AndroidKeyCollectionPair(45, Rtt::KeyName::kQ));
		sKeyCollection.insert(AndroidKeyCollectionPair(46, Rtt::KeyName::kR));
		sKeyCollection.insert(AndroidKeyCollectionPair(47, Rtt::KeyName::kS));
		sKeyCollection.insert(AndroidKeyCollectionPair(48, Rtt::KeyName::kT));
		sKeyCollection.insert(AndroidKeyCollectionPair(49, Rtt::KeyName::kU));
		sKeyCollection.insert(AndroidKeyCollectionPair(50, Rtt::KeyName::kV));
		sKeyCollection.insert(AndroidKeyCollectionPair(51, Rtt::KeyName::kW));
		sKeyCollection.insert(AndroidKeyCollectionPair(52, Rtt::KeyName::kX));
		sKeyCollection.insert(AndroidKeyCollectionPair(53, Rtt::KeyName::kY));
		sKeyCollection.insert(AndroidKeyCollectionPair(54, Rtt::KeyName::kZ));

		sKeyCollection.insert(AndroidKeyCollectionPair(7, Rtt::KeyName::k0));
		sKeyCollection.insert(AndroidKeyCollectionPair(8, Rtt::KeyName::k1));
		sKeyCollection.insert(AndroidKeyCollectionPair(9, Rtt::KeyName::k2));
		sKeyCollection.insert(AndroidKeyCollectionPair(10, Rtt::KeyName::k3));
		sKeyCollection.insert(AndroidKeyCollectionPair(11, Rtt::KeyName::k4));
		sKeyCollection.insert(AndroidKeyCollectionPair(12, Rtt::KeyName::k5));
		sKeyCollection.insert(AndroidKeyCollectionPair(13, Rtt::KeyName::k6));
		sKeyCollection.insert(AndroidKeyCollectionPair(14, Rtt::KeyName::k7));
		sKeyCollection.insert(AndroidKeyCollectionPair(15, Rtt::KeyName::k8));
		sKeyCollection.insert(AndroidKeyCollectionPair(16, Rtt::KeyName::k9));

		sKeyCollection.insert(AndroidKeyCollectionPair(144, Rtt::KeyName::kNumPad0));
		sKeyCollection.insert(AndroidKeyCollectionPair(145, Rtt::KeyName::kNumPad1));
		sKeyCollection.insert(AndroidKeyCollectionPair(146, Rtt::KeyName::kNumPad2));
		sKeyCollection.insert(AndroidKeyCollectionPair(147, Rtt::KeyName::kNumPad3));
		sKeyCollection.insert(AndroidKeyCollectionPair(148, Rtt::KeyName::kNumPad4));
		sKeyCollection.insert(AndroidKeyCollectionPair(149, Rtt::KeyName::kNumPad5));
		sKeyCollection.insert(AndroidKeyCollectionPair(150, Rtt::KeyName::kNumPad6));
		sKeyCollection.insert(AndroidKeyCollectionPair(151, Rtt::KeyName::kNumPad7));
		sKeyCollection.insert(AndroidKeyCollectionPair(152, Rtt::KeyName::kNumPad8));
		sKeyCollection.insert(AndroidKeyCollectionPair(153, Rtt::KeyName::kNumPad9));
		
		sKeyCollection.insert(AndroidKeyCollectionPair(157, Rtt::KeyName::kNumPadAdd));
		sKeyCollection.insert(AndroidKeyCollectionPair(156, Rtt::KeyName::kNumPadSubtract));
		sKeyCollection.insert(AndroidKeyCollectionPair(155, Rtt::KeyName::kNumPadMultiply));
		sKeyCollection.insert(AndroidKeyCollectionPair(154, Rtt::KeyName::kNumPadDivide));
		sKeyCollection.insert(AndroidKeyCollectionPair(161, Rtt::KeyName::kNumPadEquals));
		sKeyCollection.insert(AndroidKeyCollectionPair(160, Rtt::KeyName::kNumPadEnter));
		sKeyCollection.insert(AndroidKeyCollectionPair(158, Rtt::KeyName::kNumPadPeriod));
		sKeyCollection.insert(AndroidKeyCollectionPair(159, Rtt::KeyName::kNumPadComma));
		sKeyCollection.insert(AndroidKeyCollectionPair(162, Rtt::KeyName::kNumPadLeftParentheses));
		sKeyCollection.insert(AndroidKeyCollectionPair(163, Rtt::KeyName::kNumPadRightParentheses));

		sKeyCollection.insert(AndroidKeyCollectionPair(19, Rtt::KeyName::kUp));
		sKeyCollection.insert(AndroidKeyCollectionPair(20, Rtt::KeyName::kDown));
		sKeyCollection.insert(AndroidKeyCollectionPair(21, Rtt::KeyName::kLeft));
		sKeyCollection.insert(AndroidKeyCollectionPair(22, Rtt::KeyName::kRight));
		sKeyCollection.insert(AndroidKeyCollectionPair(23, Rtt::KeyName::kCenter));

		sKeyCollection.insert(AndroidKeyCollectionPair(57, Rtt::KeyName::kLeftAlt));
		sKeyCollection.insert(AndroidKeyCollectionPair(58, Rtt::KeyName::kRightAlt));
		sKeyCollection.insert(AndroidKeyCollectionPair(113, Rtt::KeyName::kLeftControl));
		sKeyCollection.insert(AndroidKeyCollectionPair(114, Rtt::KeyName::kRightControl));
		sKeyCollection.insert(AndroidKeyCollectionPair(59, Rtt::KeyName::kLeftShift));
		sKeyCollection.insert(AndroidKeyCollectionPair(60, Rtt::KeyName::kRightShift));
		sKeyCollection.insert(AndroidKeyCollectionPair(117, Rtt::KeyName::kLeftCommand));
		sKeyCollection.insert(AndroidKeyCollectionPair(118, Rtt::KeyName::kRightCommand));

		sKeyCollection.insert(AndroidKeyCollectionPair(131, Rtt::KeyName::kF1));
		sKeyCollection.insert(AndroidKeyCollectionPair(132, Rtt::KeyName::kF2));
		sKeyCollection.insert(AndroidKeyCollectionPair(133, Rtt::KeyName::kF3));
		sKeyCollection.insert(AndroidKeyCollectionPair(134, Rtt::KeyName::kF4));
		sKeyCollection.insert(AndroidKeyCollectionPair(135, Rtt::KeyName::kF5));
		sKeyCollection.insert(AndroidKeyCollectionPair(136, Rtt::KeyName::kF6));
		sKeyCollection.insert(AndroidKeyCollectionPair(137, Rtt::KeyName::kF7));
		sKeyCollection.insert(AndroidKeyCollectionPair(138, Rtt::KeyName::kF8));
		sKeyCollection.insert(AndroidKeyCollectionPair(139, Rtt::KeyName::kF9));
		sKeyCollection.insert(AndroidKeyCollectionPair(140, Rtt::KeyName::kF10));
		sKeyCollection.insert(AndroidKeyCollectionPair(141, Rtt::KeyName::kF11));
		sKeyCollection.insert(AndroidKeyCollectionPair(142, Rtt::KeyName::kF12));

		sKeyCollection.insert(AndroidKeyCollectionPair(4, Rtt::KeyName::kBack));
		sKeyCollection.insert(AndroidKeyCollectionPair(125, Rtt::KeyName::kForward));

		sKeyCollection.insert(AndroidKeyCollectionPair(126, Rtt::KeyName::kMediaPlay));
		sKeyCollection.insert(AndroidKeyCollectionPair(127, Rtt::KeyName::kMediaPause));
		sKeyCollection.insert(AndroidKeyCollectionPair(85, Rtt::KeyName::kMediaPlayPause));
		sKeyCollection.insert(AndroidKeyCollectionPair(86, Rtt::KeyName::kMediaStop));
		sKeyCollection.insert(AndroidKeyCollectionPair(87, Rtt::KeyName::kMediaNext));
		sKeyCollection.insert(AndroidKeyCollectionPair(88, Rtt::KeyName::kMediaPrevious));
		sKeyCollection.insert(AndroidKeyCollectionPair(89, Rtt::KeyName::kMediaRewind));
		sKeyCollection.insert(AndroidKeyCollectionPair(90, Rtt::KeyName::kMediaFastForward));
		sKeyCollection.insert(AndroidKeyCollectionPair(128, Rtt::KeyName::kMediaClose));
		sKeyCollection.insert(AndroidKeyCollectionPair(129, Rtt::KeyName::kMediaEject));
		sKeyCollection.insert(AndroidKeyCollectionPair(130, Rtt::KeyName::kMediaRecord));

		sKeyCollection.insert(AndroidKeyCollectionPair(24, Rtt::KeyName::kVolumeUp));
		sKeyCollection.insert(AndroidKeyCollectionPair(25, Rtt::KeyName::kVolumeDown));
		sKeyCollection.insert(AndroidKeyCollectionPair(164, Rtt::KeyName::kVolumeMute));
		sKeyCollection.insert(AndroidKeyCollectionPair(91, Rtt::KeyName::kMicrophoneMute));

		sKeyCollection.insert(AndroidKeyCollectionPair(183, Rtt::KeyName::kRed));
		sKeyCollection.insert(AndroidKeyCollectionPair(184, Rtt::KeyName::kGreen));
		sKeyCollection.insert(AndroidKeyCollectionPair(185, Rtt::KeyName::kYellow));
		sKeyCollection.insert(AndroidKeyCollectionPair(186, Rtt::KeyName::kBlue));

		sKeyCollection.insert(AndroidKeyCollectionPair(188, Rtt::KeyName::kButton1));
		sKeyCollection.insert(AndroidKeyCollectionPair(189, Rtt::KeyName::kButton2));
		sKeyCollection.insert(AndroidKeyCollectionPair(190, Rtt::KeyName::kButton3));
		sKeyCollection.insert(AndroidKeyCollectionPair(191, Rtt::KeyName::kButton4));
		sKeyCollection.insert(AndroidKeyCollectionPair(192, Rtt::KeyName::kButton5));
		sKeyCollection.insert(AndroidKeyCollectionPair(193, Rtt::KeyName::kButton6));
		sKeyCollection.insert(AndroidKeyCollectionPair(194, Rtt::KeyName::kButton7));
		sKeyCollection.insert(AndroidKeyCollectionPair(195, Rtt::KeyName::kButton8));
		sKeyCollection.insert(AndroidKeyCollectionPair(196, Rtt::KeyName::kButton9));
		sKeyCollection.insert(AndroidKeyCollectionPair(197, Rtt::KeyName::kButton10));
		sKeyCollection.insert(AndroidKeyCollectionPair(198, Rtt::KeyName::kButton11));
		sKeyCollection.insert(AndroidKeyCollectionPair(199, Rtt::KeyName::kButton12));
		sKeyCollection.insert(AndroidKeyCollectionPair(200, Rtt::KeyName::kButton13));
		sKeyCollection.insert(AndroidKeyCollectionPair(201, Rtt::KeyName::kButton14));
		sKeyCollection.insert(AndroidKeyCollectionPair(202, Rtt::KeyName::kButton15));
		sKeyCollection.insert(AndroidKeyCollectionPair(203, Rtt::KeyName::kButton16));
		sKeyCollection.insert(AndroidKeyCollectionPair(96, Rtt::KeyName::kButtonA));
		sKeyCollection.insert(AndroidKeyCollectionPair(97, Rtt::KeyName::kButtonB));
		sKeyCollection.insert(AndroidKeyCollectionPair(98, Rtt::KeyName::kButtonC));
		sKeyCollection.insert(AndroidKeyCollectionPair(99, Rtt::KeyName::kButtonX));
		sKeyCollection.insert(AndroidKeyCollectionPair(100, Rtt::KeyName::kButtonY));
		sKeyCollection.insert(AndroidKeyCollectionPair(101, Rtt::KeyName::kButtonZ));
		sKeyCollection.insert(AndroidKeyCollectionPair(109, Rtt::KeyName::kButtonSelect));
		sKeyCollection.insert(AndroidKeyCollectionPair(108, Rtt::KeyName::kButtonStart));
		sKeyCollection.insert(AndroidKeyCollectionPair(110, Rtt::KeyName::kButtonMode));
		sKeyCollection.insert(AndroidKeyCollectionPair(102, Rtt::KeyName::kLeftShoulderButton1));
		sKeyCollection.insert(AndroidKeyCollectionPair(104, Rtt::KeyName::kLeftShoulderButton2));
		sKeyCollection.insert(AndroidKeyCollectionPair(103, Rtt::KeyName::kRightShoulderButton1));
		sKeyCollection.insert(AndroidKeyCollectionPair(105, Rtt::KeyName::kRightShoulderButton2));
		sKeyCollection.insert(AndroidKeyCollectionPair(106, Rtt::KeyName::kLeftJoystickButton));
		sKeyCollection.insert(AndroidKeyCollectionPair(107, Rtt::KeyName::kRightJoystickButton));

		sKeyCollection.insert(AndroidKeyCollectionPair(206, Rtt::KeyName::k3dMode));
		sKeyCollection.insert(AndroidKeyCollectionPair(174, Rtt::KeyName::kBookmark));
		sKeyCollection.insert(AndroidKeyCollectionPair(210, Rtt::KeyName::kCalculator));
		sKeyCollection.insert(AndroidKeyCollectionPair(208, Rtt::KeyName::kCalendar));
		sKeyCollection.insert(AndroidKeyCollectionPair(5, Rtt::KeyName::kCall));
		sKeyCollection.insert(AndroidKeyCollectionPair(27, Rtt::KeyName::kCamera));
		sKeyCollection.insert(AndroidKeyCollectionPair(175, Rtt::KeyName::kCaptions));
		sKeyCollection.insert(AndroidKeyCollectionPair(166, Rtt::KeyName::kChannelUp));
		sKeyCollection.insert(AndroidKeyCollectionPair(167, Rtt::KeyName::kChannelDown));
		sKeyCollection.insert(AndroidKeyCollectionPair(207, Rtt::KeyName::kContacts));
		sKeyCollection.insert(AndroidKeyCollectionPair(173, Rtt::KeyName::kDvr));
		sKeyCollection.insert(AndroidKeyCollectionPair(6, Rtt::KeyName::kEndCall));
		sKeyCollection.insert(AndroidKeyCollectionPair(64, Rtt::KeyName::kExplorer));
		sKeyCollection.insert(AndroidKeyCollectionPair(80, Rtt::KeyName::kFocusCamera));
		sKeyCollection.insert(AndroidKeyCollectionPair(172, Rtt::KeyName::kGuide));
		sKeyCollection.insert(AndroidKeyCollectionPair(165, Rtt::KeyName::kInfo));
		sKeyCollection.insert(AndroidKeyCollectionPair(65, Rtt::KeyName::kMail));
		sKeyCollection.insert(AndroidKeyCollectionPair(82, Rtt::KeyName::kMenu));
		sKeyCollection.insert(AndroidKeyCollectionPair(209, Rtt::KeyName::kMusic));
		sKeyCollection.insert(AndroidKeyCollectionPair(83, Rtt::KeyName::kNotification));
		sKeyCollection.insert(AndroidKeyCollectionPair(84, Rtt::KeyName::kSearch));
		sKeyCollection.insert(AndroidKeyCollectionPair(170, Rtt::KeyName::kTv));

		sKeyCollection.insert(AndroidKeyCollectionPair(75, Rtt::KeyName::kApostrophe));
		sKeyCollection.insert(AndroidKeyCollectionPair(77, Rtt::KeyName::kAt));
		sKeyCollection.insert(AndroidKeyCollectionPair(73, Rtt::KeyName::kBackSlash));
		sKeyCollection.insert(AndroidKeyCollectionPair(76, Rtt::KeyName::kForwardSlash));
		sKeyCollection.insert(AndroidKeyCollectionPair(121, Rtt::KeyName::kBreak));
		sKeyCollection.insert(AndroidKeyCollectionPair(115, Rtt::KeyName::kCapsLock));
		sKeyCollection.insert(AndroidKeyCollectionPair(28, Rtt::KeyName::kClear));
		sKeyCollection.insert(AndroidKeyCollectionPair(55, Rtt::KeyName::kComma));
		sKeyCollection.insert(AndroidKeyCollectionPair(67, Rtt::KeyName::kDeleteBack));
		sKeyCollection.insert(AndroidKeyCollectionPair(112, Rtt::KeyName::kDeleteForward));
		sKeyCollection.insert(AndroidKeyCollectionPair(66, Rtt::KeyName::kEnter));
		sKeyCollection.insert(AndroidKeyCollectionPair(70, Rtt::KeyName::kEquals));
		sKeyCollection.insert(AndroidKeyCollectionPair(111, Rtt::KeyName::kEscape));
		sKeyCollection.insert(AndroidKeyCollectionPair(119, Rtt::KeyName::kFunction));
		sKeyCollection.insert(AndroidKeyCollectionPair(68, Rtt::KeyName::kBackTick));
		sKeyCollection.insert(AndroidKeyCollectionPair(79, Rtt::KeyName::kHeadsetHook));
		sKeyCollection.insert(AndroidKeyCollectionPair(124, Rtt::KeyName::kInsert));
		sKeyCollection.insert(AndroidKeyCollectionPair(71, Rtt::KeyName::kLeftBracket));
		sKeyCollection.insert(AndroidKeyCollectionPair(72, Rtt::KeyName::kRightBracket));
		sKeyCollection.insert(AndroidKeyCollectionPair(143, Rtt::KeyName::kNumLock));
		sKeyCollection.insert(AndroidKeyCollectionPair(81, Rtt::KeyName::kPlus));
		sKeyCollection.insert(AndroidKeyCollectionPair(69, Rtt::KeyName::kMinus));
		sKeyCollection.insert(AndroidKeyCollectionPair(122, Rtt::KeyName::kHome));
		sKeyCollection.insert(AndroidKeyCollectionPair(123, Rtt::KeyName::kEnd));
		sKeyCollection.insert(AndroidKeyCollectionPair(92, Rtt::KeyName::kPageUp));
		sKeyCollection.insert(AndroidKeyCollectionPair(93, Rtt::KeyName::kPageDown));
		sKeyCollection.insert(AndroidKeyCollectionPair(56, Rtt::KeyName::kPeriod));
		sKeyCollection.insert(AndroidKeyCollectionPair(18, Rtt::KeyName::kPound));
		sKeyCollection.insert(AndroidKeyCollectionPair(116, Rtt::KeyName::kScrollLock));
		sKeyCollection.insert(AndroidKeyCollectionPair(74, Rtt::KeyName::kSemicolon));
		sKeyCollection.insert(AndroidKeyCollectionPair(62, Rtt::KeyName::kSpace));
		sKeyCollection.insert(AndroidKeyCollectionPair(17, Rtt::KeyName::kAsterisk));
		sKeyCollection.insert(AndroidKeyCollectionPair(120, Rtt::KeyName::kPrintScreen));
		sKeyCollection.insert(AndroidKeyCollectionPair(61, Rtt::KeyName::kTab));
		sKeyCollection.insert(AndroidKeyCollectionPair(168, Rtt::KeyName::kZoomIn));
		sKeyCollection.insert(AndroidKeyCollectionPair(169, Rtt::KeyName::kZoomOut));
	}

	// Return a reference to the key collection.
	return sKeyCollection;
}

/// Constructor made private to prevent objects from being made.
AndroidKeyServices::AndroidKeyServices()
{
}

/// Fetches Corona's unique key name for the given Android key code.
/// @param keyCode Android's unique integer ID for the key.
///                Matches a KEYCODE constant in Android's "KeyEvent" class in Java.
/// @return Returns Corona unique name for the given key code.
///         Returns "unknown" if the given key code is not supported by Corona.
const char* AndroidKeyServices::GetNameFromKeyCode(int keyCode)
{
	const char* keyName = NULL;

	// Fetch Corona's unique key name for the given Android key code.
	AndroidKeyCollection& collection = GetCollection();
	AndroidKeyCollection::const_iterator iterator = collection.find(keyCode);
	if (iterator != collection.end())
	{
		keyName = iterator->second;
	}

	// If a key name was not found, then set it to unknown.
	if (!keyName)
	{
		keyName = Rtt::KeyName::kUnknown;
	}

	// Return the result.
	return keyName;
}

/// Determines if the given Android key code is supported by Corona.
/// @param keyCode Android's unique integer ID for the key.
///                Matches a KEYCODE constant in Android's "KeyEvent" class in Java.
/// @return Returns true if Corona has a unique name for the given key code.
///         Returns false if "unknown".
bool AndroidKeyServices::IsKeyCodeKnown(int keyCode)
{
	AndroidKeyCollection& collection = GetCollection();
	AndroidKeyCollection::const_iterator iterator = collection.find(keyCode);
	return (iterator != collection.end());
}
