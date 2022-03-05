#include "Rtt_KeyName.h"
#include "Rtt_LinuxApp.h"

using namespace std;

namespace Rtt
{

	map<int32_t, string> sKeyName
	{
		{SDLK_a, KeyName::kA },
		{SDLK_b, KeyName::kB },
		{SDLK_c, KeyName::kC },
		{SDLK_d, KeyName::kD },
		{SDLK_e, KeyName::kE },
		{SDLK_f, KeyName::kF },
		{SDLK_g, KeyName::kG },
		{SDLK_h, KeyName::kH },
		{SDLK_i, KeyName::kI },
		{SDLK_j, KeyName::kJ },
		{SDLK_k, KeyName::kK },
		{SDLK_l, KeyName::kL },
		{SDLK_m, KeyName::kM },
		{SDLK_n, KeyName::kN },
		{SDLK_o, KeyName::kO },
		{SDLK_p, KeyName::kP },
		{SDLK_q, KeyName::kQ },
		{SDLK_r, KeyName::kR },
		{SDLK_s, KeyName::kS },
		{SDLK_t, KeyName::kT },
		{SDLK_u, KeyName::kU },
		{SDLK_v, KeyName::kV },
		{SDLK_w, KeyName::kW },
		{SDLK_x, KeyName::kX },
		{SDLK_y, KeyName::kY },
		{SDLK_z, KeyName::kZ },
		{SDLK_0, KeyName::k0 },
		{SDLK_1, KeyName::k1 },
		{SDLK_2, KeyName::k2 },
		{SDLK_3, KeyName::k3 },
		{SDLK_4, KeyName::k4 },
		{SDLK_5, KeyName::k5 },
		{SDLK_6, KeyName::k6 },
		{SDLK_7, KeyName::k7 },
		{SDLK_8, KeyName::k8 },
		{SDLK_9, KeyName::k9 },
		{SDLK_F1, KeyName::kF1 },
		{SDLK_F2, KeyName::kF2 },
		{SDLK_F3, KeyName::kF3 },
		{SDLK_F4, KeyName::kF4 },
		{SDLK_F5, KeyName::kF5 },
		{SDLK_F6, KeyName::kF6 },
		{SDLK_F7, KeyName::kF7 },
		{SDLK_F8, KeyName::kF8 },
		{SDLK_F9, KeyName::kF9 },
		{SDLK_F10, KeyName::kF10 },
		{SDLK_F11, KeyName::kF11 },
		{SDLK_F12, KeyName::kF12 },
		{SDLK_PRINTSCREEN, KeyName::kPrintScreen},
		{SDLK_SCROLLLOCK, KeyName::kScrollLock},
		{SDLK_INSERT, KeyName::kInsert},
		{SDLK_HOME, KeyName::kHome},
		{SDLK_HOME, KeyName::kHomePage},
		{SDLK_PAGEUP, KeyName::kPageUp},
		{SDLK_DELETE, KeyName::kDeleteBack},
		{SDLK_END, KeyName::kEnd},
		{SDLK_PAGEDOWN, KeyName::kPageDown},
		{SDLK_RIGHT, KeyName::kRight},
		{SDLK_LEFT, KeyName::kLeft},
		{SDLK_DOWN, KeyName::kDown},
		{SDLK_UP, KeyName::kUp},
		{SDLK_SPACE, KeyName::kSpace},
		{SDLK_ESCAPE, KeyName::kEscape},
		{SDLK_CAPSLOCK, KeyName::kCapsLock},
		{SDLK_TAB, KeyName::kTab},
		{SDLK_BACKQUOTE, KeyName::kBackTick},

		{SDLK_LCTRL, KeyName::kLeftControl},
		{SDLK_LSHIFT, KeyName::kLeftShift},
		{SDLK_LALT, KeyName::kLeftAlt},
		{SDLK_LGUI, KeyName::kLeftCommand},
		{SDLK_RCTRL, KeyName::kRightControl},
		{SDLK_RSHIFT, KeyName::kRightShift},
		{SDLK_RALT, KeyName::kRightAlt},
		{SDLK_RGUI, KeyName::kRightCommand},

		{SDLK_NUMLOCKCLEAR, KeyName::kNumLock},
		{SDLK_KP_DIVIDE, KeyName::kNumPadDivide},
		{SDLK_KP_MULTIPLY, KeyName::kNumPadMultiply},
		{SDLK_KP_MINUS, KeyName::kNumPadSubtract},
		{SDLK_KP_PLUS, KeyName::kNumPadAdd},
		{SDLK_KP_ENTER, KeyName::kNumPadEnter},
		{SDLK_KP_1, KeyName::kNumPad1},
		{SDLK_KP_2, KeyName::kNumPad2},
		{SDLK_KP_3, KeyName::kNumPad3},
		{SDLK_KP_4, KeyName::kNumPad4},
		{SDLK_KP_5, KeyName::kNumPad5},
		{SDLK_KP_6, KeyName::kNumPad6},
		{SDLK_KP_7, KeyName::kNumPad7},
		{SDLK_KP_8, KeyName::kNumPad8},
		{SDLK_KP_9, KeyName::kNumPad9},
		{SDLK_KP_0, KeyName::kNumPad0},
		{SDLK_KP_PERIOD, KeyName::kNumPadComma },
	
		{SDLK_RETURN, KeyName::kEnter },
		{SDLK_BACKSPACE, KeyName::kBack },
		{SDLK_BACKSLASH, KeyName::kBackSlash }
	};

	const char* GetKeyName(int32_t code)
	{
		auto it = sKeyName.find(code);
		return it == sKeyName.end() ? KeyName::kUnknown : it->second.c_str();
	};

}
