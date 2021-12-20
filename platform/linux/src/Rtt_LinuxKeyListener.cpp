#include "Rtt_LinuxKeyListener.h"
#include "Rtt_KeyName.h"
#include "Rtt_LinuxContext.h"

namespace Rtt
{
	LinuxKeyListener::LinuxKeyListener(Runtime &runtime)
		: fRuntime(runtime)
	{
		fKeyName['A'] = KeyName::kA;
		fKeyName['B'] = KeyName::kB;
		fKeyName['C'] = KeyName::kC;
		fKeyName['D'] = KeyName::kD;
		fKeyName['E'] = KeyName::kE;
		fKeyName['F'] = KeyName::kF;
		fKeyName['G'] = KeyName::kG;
		fKeyName['H'] = KeyName::kH;
		fKeyName['I'] = KeyName::kI;
		fKeyName['J'] = KeyName::kJ;
		fKeyName['K'] = KeyName::kK;
		fKeyName['L'] = KeyName::kL;
		fKeyName['M'] = KeyName::kM;
		fKeyName['N'] = KeyName::kN;
		fKeyName['O'] = KeyName::kO;
		fKeyName['P'] = KeyName::kP;
		fKeyName['Q'] = KeyName::kQ;
		fKeyName['R'] = KeyName::kR;
		fKeyName['S'] = KeyName::kS;
		fKeyName['T'] = KeyName::kT;
		fKeyName['U'] = KeyName::kU;
		fKeyName['V'] = KeyName::kV;
		fKeyName['W'] = KeyName::kW;
		fKeyName['X'] = KeyName::kX;
		fKeyName['Y'] = KeyName::kY;
		fKeyName['Z'] = KeyName::kZ;
		fKeyName['0'] = KeyName::k0;
		fKeyName['1'] = KeyName::k1;
		fKeyName['2'] = KeyName::k2;
		fKeyName['3'] = KeyName::k3;
		fKeyName['4'] = KeyName::k4;
		fKeyName['5'] = KeyName::k5;
		fKeyName['6'] = KeyName::k6;
		fKeyName['7'] = KeyName::k7;
		fKeyName['8'] = KeyName::k8;
		fKeyName['9'] = KeyName::k9;
		fKeyName[340] = KeyName::kF1;
		fKeyName[341] = KeyName::kF2;
		fKeyName[342] = KeyName::kF3;
		fKeyName[343] = KeyName::kF4;
		fKeyName[344] = KeyName::kF5;
		fKeyName[345] = KeyName::kF6;
		fKeyName[346] = KeyName::kF7;
		fKeyName[347] = KeyName::kF8;
		fKeyName[348] = KeyName::kF9;
		fKeyName[349] = KeyName::kF10;
		fKeyName[350] = KeyName::kF11;
		fKeyName[351] = KeyName::kF12;
		fKeyName[32] = KeyName::kSpace;
		fKeyName[307] = KeyName::kLeftAlt;
		//fKeyName["Right Alt"] = KeyName::kRightAlt;
		fKeyName[308] = KeyName::kLeftControl;
		//fKeyName["Right Ctrl"] = KeyName::kRightControl;
		fKeyName[306] = KeyName::kLeftShift;
		//fKeyName["Right Shift"] = KeyName::kRightShift;
		fKeyName[393] = KeyName::kLeftCommand;
		//fKeyName["Right Windows"] = KeyName::kRightCommand;
		fKeyName[315] = KeyName::kUp;
		fKeyName[317] = KeyName::kDown;
		fKeyName[314] = KeyName::kLeft;
		fKeyName[316] = KeyName::kRight;
		fKeyName[313] = KeyName::kHome;
		fKeyName[312] = KeyName::kEnd;
		fKeyName[366] = KeyName::kPageUp;
		fKeyName[367] = KeyName::kPageDown;
		fKeyName[322] = KeyName::kInsert;
		fKeyName[127] = KeyName::kDeleteForward;
		fKeyName[8] = KeyName::kDeleteBack;
		fKeyName[47] = KeyName::kForwardSlash;
		fKeyName[92] = KeyName::kBackSlash;
		fKeyName['='] = KeyName::kPlus;
		fKeyName['-'] = KeyName::kMinus;
		fKeyName[','] = ",";
		fKeyName['.'] = KeyName::kPeriod;
		fKeyName['`'] = "`";
		fKeyName[';'] = ";";
		fKeyName['['] = KeyName::kLeftBracket;
		fKeyName[']'] = KeyName::kRightBracket;
		fKeyName['\''] = KeyName::kApostrophe;
		fKeyName[27] = KeyName::kEscape;
		fKeyName[13] = KeyName::kEnter;
		fKeyName[321] = KeyName::kPrintScreen;
		fKeyName[310] = KeyName::kMediaPause;
		fKeyName[365] = KeyName::kScrollLock;
		fKeyName[384] = KeyName::kNumPad0;
		fKeyName[382] = KeyName::kNumPad1;
		fKeyName[379] = KeyName::kNumPad2;
		fKeyName[381] = KeyName::kNumPad3;
		fKeyName[376] = KeyName::kNumPad4;
		fKeyName[383] = KeyName::kNumPad5;
		fKeyName[378] = KeyName::kNumPad6;
		fKeyName[375] = KeyName::kNumPad7;
		fKeyName[377] = KeyName::kNumPad8;
		fKeyName[380] = KeyName::kNumPad9;
		fKeyName[392] = KeyName::kNumPadDivide;
		fKeyName[387] = KeyName::kNumPadMultiply;
		fKeyName[390] = KeyName::kNumPadSubtract;
		fKeyName[388] = KeyName::kNumPadAdd;
		fKeyName[370] = KeyName::kNumPadEnter;
		fKeyName[385] = KeyName::kNumPadComma;
		//fKeyName["Keypad ."] = KeyName::kNumPadPeriod;
		//fKeyName["Keypad ("] = KeyName::kNumPadLeftParentheses;
		//fKeyName["Keypad )"] = KeyName::kNumPadRightParentheses;
		fKeyName[364] = KeyName::kNumLock;
		fKeyName[9] = KeyName::kTab;
		fKeyName[426] = KeyName::kVolumeUp;
		fKeyName[425] = KeyName::kVolumeDown;
		fKeyName[424] = KeyName::kVolumeMute;
		//fKeyName["Left GUI"] = KeyName::kLeftCommand; // web
		//fKeyName["Right GUI"] = KeyName::kRightCommand; // web
		fKeyName[311] = KeyName::kCapsLock;
		//fKeyName["ZoomIn"] = KeyName::kZoomIn;
		//fKeyName["ZoomOut"] = KeyName::kZoomOut;
		//fKeyName["Break"] = KeyName::kBreak;
		fKeyName[309] = KeyName::kMenu;
		//fKeyName["Application"] = KeyName::kMenu; // web
	}

	void LinuxKeyListener::notifyCharEvent(wxKeyEvent &e)
	{
		wxChar unicodeCharacter = e.GetUnicodeKey();

		if (unicodeCharacter != WXK_NONE)
		{
			wxCharBuffer utf8Buffer = wxString(e.GetUnicodeKey()).ToUTF8();
			const char *utf8Character = utf8Buffer.data();

			if (strlen(utf8Character) > 1 || isprint(utf8Character[0]))
			{
				CharacterEvent characterEvent(NULL, utf8Character);
				fRuntime.DispatchEvent(characterEvent);
			}
		}
	}

	void LinuxKeyListener::notifyKeyEvent(wxKeyEvent &e, bool down)
	{
		int mod = e.GetModifiers();
		bool isNumLockDown = false; // fixme
		bool isCapsLockDown = false; // fixme
		bool isShiftDown = mod & wxMOD_SHIFT ? true : false;
		bool isCtrlDown = mod & wxMOD_CONTROL ? true : false;
		bool isAltDown = mod & wxMOD_ALT ? true : false;
		bool isCommandDown = mod & wxMOD_WIN ? true : false;
		int keycode = e.GetKeyCode();
		PlatformInputDevice *dev = NULL;
		auto it = fKeyName.find(keycode);
		const char *keyName = it == fKeyName.end() ? KeyName::kUnknown : it->second.c_str();

		KeyEvent ke(dev, down ? KeyEvent::kDown : KeyEvent::kUp, keyName, keycode, isShiftDown, isAltDown, isCtrlDown, isCommandDown);
		fRuntime.DispatchEvent(ke);
	}

	void LinuxKeyListener::OnChar(wxKeyEvent &event)
	{
		event.Skip();
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		context->GetKeyListener()->notifyCharEvent(event);
	}

	void LinuxKeyListener::OnKeyDown(wxKeyEvent &event)
	{
		event.Skip();
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;

		if (event.GetKeyCode() == WXK_ESCAPE)
		{
			wxGetApp().GetFrame()->Close(); // close main window
		}
		else
		{
			context->GetKeyListener()->notifyKeyEvent(event, true);
		}
	}

	void LinuxKeyListener::OnKeyUp(wxKeyEvent &event)
	{
		event.Skip();
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;

		if (context && event.GetKeyCode() != WXK_ESCAPE)
		{
			context->GetKeyListener()->notifyKeyEvent(event, false);
		}
	}
};
