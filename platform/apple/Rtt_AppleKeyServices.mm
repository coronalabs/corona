//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Rtt_KeyName.h"

#import "Rtt_AppleKeyServices.h"

#ifdef Rtt_MAC_ENV
#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#elif defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
#import <UIKit/UIKit.h>
#endif

@implementation AppleKeyServices

static NSDictionary *keyNameDictionary = nil;

+ (NSString*)getNameForKey:(NSNumber*)keyCode
{
    if ( nil == keyNameDictionary )
    {
        #ifdef Rtt_MAC_ENV
        keyNameDictionary = @{
            [NSNumber numberWithInteger:(kVK_ANSI_A)] : [NSString stringWithUTF8String:Rtt::KeyName::kA],
            [NSNumber numberWithInteger:(kVK_ANSI_B)] : [NSString stringWithUTF8String:Rtt::KeyName::kB],
            [NSNumber numberWithInteger:(kVK_ANSI_C)] : [NSString stringWithUTF8String:Rtt::KeyName::kC],
            [NSNumber numberWithInteger:(kVK_ANSI_D)] : [NSString stringWithUTF8String:Rtt::KeyName::kD],
            [NSNumber numberWithInteger:(kVK_ANSI_E)] : [NSString stringWithUTF8String:Rtt::KeyName::kE],
            [NSNumber numberWithInteger:(kVK_ANSI_F)] : [NSString stringWithUTF8String:Rtt::KeyName::kF],
            [NSNumber numberWithInteger:(kVK_ANSI_G)] : [NSString stringWithUTF8String:Rtt::KeyName::kG],
            [NSNumber numberWithInteger:(kVK_ANSI_H)] : [NSString stringWithUTF8String:Rtt::KeyName::kH],
            [NSNumber numberWithInteger:(kVK_ANSI_I)] : [NSString stringWithUTF8String:Rtt::KeyName::kI],
            [NSNumber numberWithInteger:(kVK_ANSI_J)] : [NSString stringWithUTF8String:Rtt::KeyName::kJ],
            [NSNumber numberWithInteger:(kVK_ANSI_K)] : [NSString stringWithUTF8String:Rtt::KeyName::kK],
            [NSNumber numberWithInteger:(kVK_ANSI_L)] : [NSString stringWithUTF8String:Rtt::KeyName::kL],
            [NSNumber numberWithInteger:(kVK_ANSI_M)] : [NSString stringWithUTF8String:Rtt::KeyName::kM],
            [NSNumber numberWithInteger:(kVK_ANSI_N)] : [NSString stringWithUTF8String:Rtt::KeyName::kN],
            [NSNumber numberWithInteger:(kVK_ANSI_O)] : [NSString stringWithUTF8String:Rtt::KeyName::kO],
            [NSNumber numberWithInteger:(kVK_ANSI_P)] : [NSString stringWithUTF8String:Rtt::KeyName::kP],
            [NSNumber numberWithInteger:(kVK_ANSI_Q)] : [NSString stringWithUTF8String:Rtt::KeyName::kQ],
            [NSNumber numberWithInteger:(kVK_ANSI_R)] : [NSString stringWithUTF8String:Rtt::KeyName::kR],
            [NSNumber numberWithInteger:(kVK_ANSI_S)] : [NSString stringWithUTF8String:Rtt::KeyName::kS],
            [NSNumber numberWithInteger:(kVK_ANSI_T)] : [NSString stringWithUTF8String:Rtt::KeyName::kT],
            [NSNumber numberWithInteger:(kVK_ANSI_U)] : [NSString stringWithUTF8String:Rtt::KeyName::kU],
            [NSNumber numberWithInteger:(kVK_ANSI_V)] : [NSString stringWithUTF8String:Rtt::KeyName::kV],
            [NSNumber numberWithInteger:(kVK_ANSI_W)] : [NSString stringWithUTF8String:Rtt::KeyName::kW],
            [NSNumber numberWithInteger:(kVK_ANSI_X)] : [NSString stringWithUTF8String:Rtt::KeyName::kX],
            [NSNumber numberWithInteger:(kVK_ANSI_Y)] : [NSString stringWithUTF8String:Rtt::KeyName::kY],
            [NSNumber numberWithInteger:(kVK_ANSI_Z)] : [NSString stringWithUTF8String:Rtt::KeyName::kZ],

            [NSNumber numberWithInteger:(kVK_ANSI_0)] : [NSString stringWithUTF8String:Rtt::KeyName::k0],
            [NSNumber numberWithInteger:(kVK_ANSI_1)] : [NSString stringWithUTF8String:Rtt::KeyName::k1],
            [NSNumber numberWithInteger:(kVK_ANSI_2)] : [NSString stringWithUTF8String:Rtt::KeyName::k2],
            [NSNumber numberWithInteger:(kVK_ANSI_3)] : [NSString stringWithUTF8String:Rtt::KeyName::k3],
            [NSNumber numberWithInteger:(kVK_ANSI_4)] : [NSString stringWithUTF8String:Rtt::KeyName::k4],
            [NSNumber numberWithInteger:(kVK_ANSI_5)] : [NSString stringWithUTF8String:Rtt::KeyName::k5],
            [NSNumber numberWithInteger:(kVK_ANSI_6)] : [NSString stringWithUTF8String:Rtt::KeyName::k6],
            [NSNumber numberWithInteger:(kVK_ANSI_7)] : [NSString stringWithUTF8String:Rtt::KeyName::k7],
            [NSNumber numberWithInteger:(kVK_ANSI_8)] : [NSString stringWithUTF8String:Rtt::KeyName::k8],
            [NSNumber numberWithInteger:(kVK_ANSI_9)] : [NSString stringWithUTF8String:Rtt::KeyName::k9],
            [NSNumber numberWithInteger:(kVK_ANSI_Minus)] : [NSString stringWithUTF8String:Rtt::KeyName::kMinus],
            [NSNumber numberWithInteger:(kVK_ANSI_Equal)] : [NSString stringWithUTF8String:Rtt::KeyName::kEquals],

            [NSNumber numberWithInteger:(kVK_ANSI_Keypad0)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad0],
            [NSNumber numberWithInteger:(kVK_ANSI_Keypad1)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad1],
            [NSNumber numberWithInteger:(kVK_ANSI_Keypad2)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad2],
            [NSNumber numberWithInteger:(kVK_ANSI_Keypad3)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad3],
            [NSNumber numberWithInteger:(kVK_ANSI_Keypad4)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad4],
            [NSNumber numberWithInteger:(kVK_ANSI_Keypad5)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad5],
            [NSNumber numberWithInteger:(kVK_ANSI_Keypad6)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad6],
            [NSNumber numberWithInteger:(kVK_ANSI_Keypad7)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad7],
            [NSNumber numberWithInteger:(kVK_ANSI_Keypad8)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad8],
            [NSNumber numberWithInteger:(kVK_ANSI_Keypad9)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad9],

            [NSNumber numberWithInteger:(kVK_ANSI_KeypadPlus)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadAdd],
            [NSNumber numberWithInteger:(kVK_ANSI_KeypadMinus)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadSubtract],
            [NSNumber numberWithInteger:(kVK_ANSI_KeypadMultiply)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadMultiply],
            [NSNumber numberWithInteger:(kVK_ANSI_KeypadDivide)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadDivide],
            [NSNumber numberWithInteger:(kVK_ANSI_KeypadDecimal)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadPeriod],
            [NSNumber numberWithInteger:(kVK_JIS_KeypadComma)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadComma],
            [NSNumber numberWithInteger:(kVK_ANSI_KeypadClear)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumLock],
            [NSNumber numberWithInteger:(kVK_ANSI_KeypadEquals)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadEquals],
            [NSNumber numberWithInteger:(kVK_ANSI_KeypadEnter)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadEnter],

            [NSNumber numberWithInteger:(kVK_UpArrow)] : [NSString stringWithUTF8String:Rtt::KeyName::kUp],
            [NSNumber numberWithInteger:(kVK_DownArrow)] : [NSString stringWithUTF8String:Rtt::KeyName::kDown],
            [NSNumber numberWithInteger:(kVK_LeftArrow)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeft],
            [NSNumber numberWithInteger:(kVK_RightArrow)] : [NSString stringWithUTF8String:Rtt::KeyName::kRight],

            [NSNumber numberWithInteger:(kVK_Option)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftAlt],
            [NSNumber numberWithInteger:(kVK_RightOption)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightAlt],
            [NSNumber numberWithInteger:(kVK_Control)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftControl],
            [NSNumber numberWithInteger:(kVK_RightControl)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightControl],
            [NSNumber numberWithInteger:(kVK_Shift)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftShift],
            [NSNumber numberWithInteger:(kVK_RightShift)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightShift],
            [NSNumber numberWithInteger:(kVK_CapsLock)] : [NSString stringWithUTF8String:Rtt::KeyName::kCapsLock],
            [NSNumber numberWithInteger:(kVK_Command)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftCommand],
            [NSNumber numberWithInteger:(kVK_RightCommand)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightCommand],
            [NSNumber numberWithInteger:(kVK_Menu)] : [NSString stringWithUTF8String:Rtt::KeyName::kMenu],

            [NSNumber numberWithInteger:(kVK_F1)] : [NSString stringWithUTF8String:Rtt::KeyName::kF1],
            [NSNumber numberWithInteger:(kVK_F2)] : [NSString stringWithUTF8String:Rtt::KeyName::kF2],
            [NSNumber numberWithInteger:(kVK_F3)] : [NSString stringWithUTF8String:Rtt::KeyName::kF3],
            [NSNumber numberWithInteger:(kVK_F4)] : [NSString stringWithUTF8String:Rtt::KeyName::kF4],
            [NSNumber numberWithInteger:(kVK_F5)] : [NSString stringWithUTF8String:Rtt::KeyName::kF5],
            [NSNumber numberWithInteger:(kVK_F6)] : [NSString stringWithUTF8String:Rtt::KeyName::kF6],
            [NSNumber numberWithInteger:(kVK_F7)] : [NSString stringWithUTF8String:Rtt::KeyName::kF7],
            [NSNumber numberWithInteger:(kVK_F8)] : [NSString stringWithUTF8String:Rtt::KeyName::kF8],
            [NSNumber numberWithInteger:(kVK_F9)] : [NSString stringWithUTF8String:Rtt::KeyName::kF9],
            [NSNumber numberWithInteger:(kVK_F10)] : [NSString stringWithUTF8String:Rtt::KeyName::kF10],
            [NSNumber numberWithInteger:(kVK_F11)] : [NSString stringWithUTF8String:Rtt::KeyName::kF11],
            [NSNumber numberWithInteger:(kVK_F12)] : [NSString stringWithUTF8String:Rtt::KeyName::kF12],
            [NSNumber numberWithInteger:(kVK_F13)] : [NSString stringWithUTF8String:Rtt::KeyName::kF13],
            [NSNumber numberWithInteger:(kVK_F14)] : [NSString stringWithUTF8String:Rtt::KeyName::kF14],
            [NSNumber numberWithInteger:(kVK_F15)] : [NSString stringWithUTF8String:Rtt::KeyName::kF15],
            [NSNumber numberWithInteger:(kVK_F16)] : [NSString stringWithUTF8String:Rtt::KeyName::kF16],
            [NSNumber numberWithInteger:(kVK_F17)] : [NSString stringWithUTF8String:Rtt::KeyName::kF17],
            [NSNumber numberWithInteger:(kVK_F18)] : [NSString stringWithUTF8String:Rtt::KeyName::kF18],
            [NSNumber numberWithInteger:(kVK_F19)] : [NSString stringWithUTF8String:Rtt::KeyName::kF19],
            [NSNumber numberWithInteger:(kVK_F20)] : [NSString stringWithUTF8String:Rtt::KeyName::kF20],

            [NSNumber numberWithInteger:(kVK_VolumeUp)] : [NSString stringWithUTF8String:Rtt::KeyName::kVolumeUp],
            [NSNumber numberWithInteger:(kVK_VolumeDown)] : [NSString stringWithUTF8String:Rtt::KeyName::kVolumeDown],
            [NSNumber numberWithInteger:(kVK_Mute)] : [NSString stringWithUTF8String:Rtt::KeyName::kVolumeMute],

            [NSNumber numberWithInteger:(kVK_ANSI_LeftBracket)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftBracket],
            [NSNumber numberWithInteger:(kVK_ANSI_RightBracket)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightBracket],
            [NSNumber numberWithInteger:(kVK_ANSI_Semicolon)] : [NSString stringWithUTF8String:Rtt::KeyName::kSemicolon],
            [NSNumber numberWithInteger:(kVK_ANSI_Quote)] : [NSString stringWithUTF8String:Rtt::KeyName::kApostrophe],
            [NSNumber numberWithInteger:(kVK_ANSI_Backslash)] : [NSString stringWithUTF8String:Rtt::KeyName::kBackSlash],
            [NSNumber numberWithInteger:(kVK_ANSI_Slash)] : [NSString stringWithUTF8String:Rtt::KeyName::kForwardSlash],
            [NSNumber numberWithInteger:(kVK_ANSI_Comma)] : [NSString stringWithUTF8String:Rtt::KeyName::kComma],
            [NSNumber numberWithInteger:(kVK_ANSI_Period)] : [NSString stringWithUTF8String:Rtt::KeyName::kPeriod],
            [NSNumber numberWithInteger:(kVK_ANSI_Grave)] : [NSString stringWithUTF8String:Rtt::KeyName::kBackTick],
            [NSNumber numberWithInteger:(kVK_Escape)] : [NSString stringWithUTF8String:Rtt::KeyName::kEscape],

            [NSNumber numberWithInteger:(kVK_Return)] : [NSString stringWithUTF8String:Rtt::KeyName::kEnter],
            [NSNumber numberWithInteger:(kVK_Tab)] : [NSString stringWithUTF8String:Rtt::KeyName::kTab],
            [NSNumber numberWithInteger:(kVK_Space)] : [NSString stringWithUTF8String:Rtt::KeyName::kSpace],
            [NSNumber numberWithInteger:(kVK_Delete)] : [NSString stringWithUTF8String:Rtt::KeyName::kDeleteBack],

            [NSNumber numberWithInteger:(kVK_Function)] : [NSString stringWithUTF8String:Rtt::KeyName::kFunction],
            [NSNumber numberWithInteger:(kVK_Help)] : [NSString stringWithUTF8String:Rtt::KeyName::kInsert],
            [NSNumber numberWithInteger:(kVK_ForwardDelete)] : [NSString stringWithUTF8String:Rtt::KeyName::kDeleteForward],
            [NSNumber numberWithInteger:(kVK_Home)] : [NSString stringWithUTF8String:Rtt::KeyName::kHome],
            [NSNumber numberWithInteger:(kVK_End)] : [NSString stringWithUTF8String:Rtt::KeyName::kEnd],
            [NSNumber numberWithInteger:(kVK_PageUp)] : [NSString stringWithUTF8String:Rtt::KeyName::kPageUp],
            [NSNumber numberWithInteger:(kVK_PageDown)] : [NSString stringWithUTF8String:Rtt::KeyName::kPageDown],
            [NSNumber numberWithInteger:(kVK_Back)] : [NSString stringWithUTF8String:Rtt::KeyName::kBack],
        };
        #elif defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
        keyNameDictionary = @{
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardA)] : [NSString stringWithUTF8String:Rtt::KeyName::kA],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardB)] : [NSString stringWithUTF8String:Rtt::KeyName::kB],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardC)] : [NSString stringWithUTF8String:Rtt::KeyName::kC],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardD)] : [NSString stringWithUTF8String:Rtt::KeyName::kD],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardE)] : [NSString stringWithUTF8String:Rtt::KeyName::kE],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF)] : [NSString stringWithUTF8String:Rtt::KeyName::kF],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardG)] : [NSString stringWithUTF8String:Rtt::KeyName::kG],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardH)] : [NSString stringWithUTF8String:Rtt::KeyName::kH],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardI)] : [NSString stringWithUTF8String:Rtt::KeyName::kI],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardJ)] : [NSString stringWithUTF8String:Rtt::KeyName::kJ],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardK)] : [NSString stringWithUTF8String:Rtt::KeyName::kK],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardL)] : [NSString stringWithUTF8String:Rtt::KeyName::kL],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardM)] : [NSString stringWithUTF8String:Rtt::KeyName::kM],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardN)] : [NSString stringWithUTF8String:Rtt::KeyName::kN],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardO)] : [NSString stringWithUTF8String:Rtt::KeyName::kO],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardP)] : [NSString stringWithUTF8String:Rtt::KeyName::kP],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardQ)] : [NSString stringWithUTF8String:Rtt::KeyName::kQ],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardR)] : [NSString stringWithUTF8String:Rtt::KeyName::kR],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardS)] : [NSString stringWithUTF8String:Rtt::KeyName::kS],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardT)] : [NSString stringWithUTF8String:Rtt::KeyName::kT],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardU)] : [NSString stringWithUTF8String:Rtt::KeyName::kU],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardV)] : [NSString stringWithUTF8String:Rtt::KeyName::kV],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardW)] : [NSString stringWithUTF8String:Rtt::KeyName::kW],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardX)] : [NSString stringWithUTF8String:Rtt::KeyName::kX],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardY)] : [NSString stringWithUTF8String:Rtt::KeyName::kY],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardZ)] : [NSString stringWithUTF8String:Rtt::KeyName::kZ],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard0)] : [NSString stringWithUTF8String:Rtt::KeyName::k0],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard1)] : [NSString stringWithUTF8String:Rtt::KeyName::k1],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard2)] : [NSString stringWithUTF8String:Rtt::KeyName::k2],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard3)] : [NSString stringWithUTF8String:Rtt::KeyName::k3],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard4)] : [NSString stringWithUTF8String:Rtt::KeyName::k4],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard5)] : [NSString stringWithUTF8String:Rtt::KeyName::k5],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard6)] : [NSString stringWithUTF8String:Rtt::KeyName::k6],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard7)] : [NSString stringWithUTF8String:Rtt::KeyName::k7],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard8)] : [NSString stringWithUTF8String:Rtt::KeyName::k8],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboard9)] : [NSString stringWithUTF8String:Rtt::KeyName::k9],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardHyphen)] : [NSString stringWithUTF8String:Rtt::KeyName::kMinus],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardEqualSign)] : [NSString stringWithUTF8String:Rtt::KeyName::kEquals],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad0)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad0],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad1)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad1],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad2)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad2],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad3)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad3],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad4)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad4],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad5)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad5],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad6)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad6],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad7)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad7],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad8)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad8],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypad9)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPad9],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypadPlus)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadAdd],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypadHyphen)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadSubtract],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypadAsterisk)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadMultiply],
            [NSNumber numberWithInteger:UIKeyboardHIDUsageKeypadSlash] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadDivide],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypadPeriod)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadPeriod],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypadComma)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadComma],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypadNumLock)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumLock],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypadEqualSign)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadEquals],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeypadEnter)] : [NSString stringWithUTF8String:Rtt::KeyName::kNumPadEnter],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardUpArrow)] : [NSString stringWithUTF8String:Rtt::KeyName::kUp],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardDownArrow)] : [NSString stringWithUTF8String:Rtt::KeyName::kDown],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardLeftArrow)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeft],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardRightArrow)] : [NSString stringWithUTF8String:Rtt::KeyName::kRight],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardLeftAlt)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftAlt],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardRightAlt)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightAlt],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardLeftControl)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftControl],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardRightControl)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightControl],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardLeftShift)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftShift],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardRightShift)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightShift],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardCapsLock)] : [NSString stringWithUTF8String:Rtt::KeyName::kCapsLock],
            [NSNumber numberWithInteger:(227)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftCommand],
            [NSNumber numberWithInteger:(231)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightCommand],
            [NSNumber numberWithInteger:(669)] : [NSString stringWithUTF8String:Rtt::KeyName::kFunction],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardMenu)] : [NSString stringWithUTF8String:Rtt::KeyName::kMenu],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF1)] : [NSString stringWithUTF8String:Rtt::KeyName::kF1],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF2)] : [NSString stringWithUTF8String:Rtt::KeyName::kF2],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF3)] : [NSString stringWithUTF8String:Rtt::KeyName::kF3],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF4)] : [NSString stringWithUTF8String:Rtt::KeyName::kF4],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF5)] : [NSString stringWithUTF8String:Rtt::KeyName::kF5],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF6)] : [NSString stringWithUTF8String:Rtt::KeyName::kF6],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF7)] : [NSString stringWithUTF8String:Rtt::KeyName::kF7],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF8)] : [NSString stringWithUTF8String:Rtt::KeyName::kF8],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF9)] : [NSString stringWithUTF8String:Rtt::KeyName::kF9],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF10)] : [NSString stringWithUTF8String:Rtt::KeyName::kF10],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF11)] : [NSString stringWithUTF8String:Rtt::KeyName::kF11],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF12)] : [NSString stringWithUTF8String:Rtt::KeyName::kF12],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF13)] : [NSString stringWithUTF8String:Rtt::KeyName::kF13],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF14)] : [NSString stringWithUTF8String:Rtt::KeyName::kF14],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF15)] : [NSString stringWithUTF8String:Rtt::KeyName::kF15],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF16)] : [NSString stringWithUTF8String:Rtt::KeyName::kF16],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF17)] : [NSString stringWithUTF8String:Rtt::KeyName::kF17],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF18)] : [NSString stringWithUTF8String:Rtt::KeyName::kF18],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF19)] : [NSString stringWithUTF8String:Rtt::KeyName::kF19],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardF20)] : [NSString stringWithUTF8String:Rtt::KeyName::kF20],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardVolumeUp)] : [NSString stringWithUTF8String:Rtt::KeyName::kVolumeUp],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardVolumeDown)] : [NSString stringWithUTF8String:Rtt::KeyName::kVolumeDown],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardMute)] : [NSString stringWithUTF8String:Rtt::KeyName::kVolumeMute],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardOpenBracket)] : [NSString stringWithUTF8String:Rtt::KeyName::kLeftBracket],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardCloseBracket)] : [NSString stringWithUTF8String:Rtt::KeyName::kRightBracket],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardSemicolon)] : [NSString stringWithUTF8String:Rtt::KeyName::kSemicolon],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardQuote)] : [NSString stringWithUTF8String:Rtt::KeyName::kApostrophe],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardBackslash)] : [NSString stringWithUTF8String:Rtt::KeyName::kBackSlash],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardSlash)] : [NSString stringWithUTF8String:Rtt::KeyName::kForwardSlash],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardComma)] : [NSString stringWithUTF8String:Rtt::KeyName::kComma],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardPeriod)] : [NSString stringWithUTF8String:Rtt::KeyName::kPeriod],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardGraveAccentAndTilde)] : [NSString stringWithUTF8String:Rtt::KeyName::kBackTick],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardEscape)] : [NSString stringWithUTF8String:Rtt::KeyName::kEscape],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardReturnOrEnter)] : [NSString stringWithUTF8String:Rtt::KeyName::kEnter],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardTab)] : [NSString stringWithUTF8String:Rtt::KeyName::kTab],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardSpacebar)] : [NSString stringWithUTF8String:Rtt::KeyName::kSpace],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardDeleteOrBackspace)] : [NSString stringWithUTF8String:Rtt::KeyName::kDeleteBack],

            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardInsert)] : [NSString stringWithUTF8String:Rtt::KeyName::kInsert],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardDeleteForward)] : [NSString stringWithUTF8String:Rtt::KeyName::kDeleteForward],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardHome)] : [NSString stringWithUTF8String:Rtt::KeyName::kHome],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardEnd)] : [NSString stringWithUTF8String:Rtt::KeyName::kEnd],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardPageUp)] : [NSString stringWithUTF8String:Rtt::KeyName::kPageUp],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardPageDown)] : [NSString stringWithUTF8String:Rtt::KeyName::kPageDown],
            [NSNumber numberWithInteger:(UIKeyboardHIDUsageKeyboardReturn)] : [NSString stringWithUTF8String:Rtt::KeyName::kBack],
        };
        #endif
        
        [keyNameDictionary retain];
    }

    NSString* ret = [keyNameDictionary objectForKey:keyCode];
    if ( ! ret )
    {
        ret = @"";
    }
    return ret;
}


+ (NSUInteger)getModifierMaskForKey:(unsigned short)keyCode
{
    // Masks from NSEventModifierFlags are not used here because they don't reflect
    // if the left or right modifier key was pressed. This masks contain NSEventModifierFlags mask
    // and an additional bit showing if left or right key was pressed. This feature is undocumented
    // but works.
    switch ( keyCode )
    {
        
            
    #ifdef Rtt_MAC_ENV
        case kVK_Shift:
            return 0x20102;
        case kVK_RightShift:
            return 0x20104;
        case kVK_Control:
            return 0x40101;
        case kVK_RightControl:
            return 0x42100;
        case kVK_Option:
            return 0x80120;
        case kVK_RightOption:
            return 0x80140;
        case kVK_Command:
            return 0x100108;
        case kVK_RightCommand:
            return 0x100110;
        case kVK_CapsLock:
                    return NSAlphaShiftKeyMask;
        case kVK_Function:
            return NSFunctionKeyMask;
        case kVK_Menu:
            return NSHelpKeyMask;
    #endif

    }
    return 0;
}

@end
