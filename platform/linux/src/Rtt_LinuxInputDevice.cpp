//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Config.h"
#include "Rtt_LinuxInputDeviceManager.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "Rtt_KeyName.h"
#include "Rtt_Lua.h"

#ifdef _WIN32 // what??

namespace Rtt
{
	LinuxInputDevice::LinuxInputDevice(const InputDeviceDescriptor &descriptor)
		: Super(descriptor), fConnected(InputDeviceConnectionState::kConnected), fCanVibrate(false), fd(-1), fAxesCount(0)
	{
	}

	LinuxInputDevice::~LinuxInputDevice()
	{
	}

	const char *LinuxInputDevice::getAxisName(int i)
	{
		return "?";
	}

	void LinuxInputDevice::init(const char *dev)
	{
	}

	void LinuxInputDevice::dispatchEvents(Runtime *runtime)
	{
	}

	const char *LinuxInputDevice::GetProductName()
	{
		return fSerialNumber.c_str();
	}

	bool LinuxInputDevice::AddNamedAxis(const char *axisName)
	{
		return false;
	}

	const char *LinuxInputDevice::GetDisplayName()
	{
		return fSerialNumber.c_str();
	}

	const char *LinuxInputDevice::GetPermanentStringId()
	{
		return fSerialNumber.c_str();
	}

	InputDeviceConnectionState LinuxInputDevice::GetConnectionState()
	{
		return fConnected;
	}

	bool LinuxInputDevice::CanVibrate()
	{
		return fCanVibrate;
	}

	PlatformInputAxis *LinuxInputDevice::OnCreateAxisUsing(const InputAxisDescriptor &descriptor)
	{
		return NULL;
	}

	const char *LinuxInputDevice::GetDriverName()
	{
		return fDriverName.c_str();
	}

	void LinuxInputDevice::Vibrate()
	{
	}

	LinuxInputAxis::LinuxInputAxis(const InputAxisDescriptor &descriptor)
		: PlatformInputAxis(descriptor), centerPoint0(true)
	{
	}

	Rtt_Real LinuxInputAxis::GetNormalizedValue(Rtt_Real rawValue)
	{
		return 0;
	}
} // namespace Rtt

#else

#include <linux/joystick.h>

// Axis map max size
#define AXMAP_SIZE ABS_MAX + 1

namespace Rtt
{
	static char const *sAxisNames[ABS_MAX + 1] =
	{
		"X", "Y", "Z", "Rx", "Ry", "Rz", "Throttle", "Rudder", "Wheel", "Gas", "Brake", "?", "?", "?", "?", "?", "Hat0X", "Hat0Y", "Hat1X", "Hat1Y", "Hat2X",
		"Hat2Y", "Hat3X", "Hat3Y", "?", "?", "?", "?", "?", "?", "?"
	};

	static std::map<std::string, InputAxisType> sAxisTypes =
	{
		{"X", InputAxisType::kX},
		{"Y", InputAxisType::kY},
		{"Z", InputAxisType::kZ},
		{"Rx", InputAxisType::kRotationX},
		{"Ry", InputAxisType::kRotationY},
		{"Rz", InputAxisType::kRotationZ},
		{"Hat0X", InputAxisType::kHatX},
		{"Hat0Y", InputAxisType::kHatY},
	};

	LinuxInputDevice::LinuxInputDevice(const InputDeviceDescriptor &descriptor)
		: Super(descriptor), fConnected(InputDeviceConnectionState::kConnected), fCanVibrate(false), fd(-1), fAxesCount(0)
	{
		fSerialNumber = "Unknown";
		fConnected = InputDeviceConnectionState::kDisconnected;

		fAxesMap = new uint8_t[AXMAP_SIZE];

		for (int i = 0; i < AXMAP_SIZE; i++)
		{
			fAxesMap[i] = static_cast<uint8_t>(-1);
		}
	}

	LinuxInputDevice::~LinuxInputDevice()
	{
		if (fd >= 0)
		{
			close(fd);
		}

		delete[] fAxesMap;
	}

	const char *LinuxInputDevice::getAxisName(int i)
	{
		// sanity check
		if (i >= 0 && i < AXMAP_SIZE)
		{
			int k = fAxesMap[i];

			if (k != static_cast<uint8_t>(-1) && k >= 0 && k < ARRAYSIZE(sAxisNames))
			{
				return sAxisNames[k];
			}
		}

		return "?";
	}

	void LinuxInputDevice::init(const char *dev)
	{
		fd = open(dev, O_RDONLY | O_NONBLOCK);

		if (fd >= 0)
		{
			char name[128] = {0};
			ioctl(fd, JSIOCGNAME(sizeof(name)), name);
			ioctl(fd, JSIOCGAXES, &fAxesCount);
			ioctl(fd, JSIOCGAXMAP, fAxesMap);

			int driverVer = 0;
			ioctl(fd, JSIOCGVERSION, driverVer);
			fDriverName = "driver-";
			fDriverName += std::to_string(driverVer);

			//printf("Joystick (%s) has %d axes (", name, fAxesCount);
			//for (int i = 0; i < fAxesCount; i++)
			//{
			//printf("%s%s", i > 0 ? ", " : "", getAxisName(i));
			//}
			//puts(")");

			fSerialNumber = name;
			fCanVibrate = false; // fixme

			//Set device properties
			fConnected = InputDeviceConnectionState::kConnected;

			//Setting up axis
			RemoveAllAxes();
			S64 deviceDescriptorId = GetDescriptor().GetIntegerId();

			bool axesNeutral0 = false; //commented below

			for (int i = 0; i < fAxesCount; i++)
			{
				axesNeutral0 = AddNamedAxis(getAxisName(i)) || axesNeutral0;
			}

			// HACK: most of devices have bad habbit of reporting all axes from 0 to 255. It means that
			// neutral position is 0.5, which is not nice.
			// So, if any of the axes reports it's minimal position as negative, we assume that device knows where it's neutral position is
			// and reports it as 0. So we would set old way of scaling for all axis on device
			if (!axesNeutral0)
			{
				const ReadOnlyInputAxisCollection &axes = GetAxes();

				for (S32 index = axes.GetCount() - 1; index >= 0; index--)
				{
					LinuxInputAxis *axis = (LinuxInputAxis *)axes.GetByIndex(index);

					if (axis)
					{
						axis->centerPoint0 = false;
					}
				}
			}

#ifdef Rtt_DEBUG
			Rtt_LogException("Joystick %s on %s found\n", name, dev);
#endif
		}
		else
		{
			//	Rtt_LogException("no joystick on %s\n", dev);
		}
	}

	void LinuxInputDevice::dispatchEvents(Runtime *runtime)
	{
		struct js_event e;

		while (fd >= 0 && read(fd, &e, sizeof(e)) > 0)
		{
			if (e.type & JS_EVENT_INIT)
			{
				continue;
			}

			switch (e.type)
			{
				case JS_EVENT_BUTTON:
				{
					//printf("JS_EVENT_BUTTON: type=%d, value=%d, button=%d\n", e.type, e.value, e.number);
					bool pressed = e.value == 1;
					unsigned int key = e.number;
					KeyEvent::Phase phase = e.value == 1 ? KeyEvent::kDown : KeyEvent::kUp;

					// disabled system button-map

					char buttonName[25];

					//this part is sketcy. I'm not really sure how to tell which button is it...
					const char *keyNames[] = {KeyName::kButton1, KeyName::kButton2, KeyName::kButton3, KeyName::kButton4, KeyName::kButton5, KeyName::kButton6, KeyName::kButton7, KeyName::kButton8, KeyName::kButton9, KeyName::kButton10, KeyName::kButton11, KeyName::kButton12, KeyName::kButton13, KeyName::kButton14, KeyName::kButton15, KeyName::kButton16};
					if (key < ARRAYSIZE(keyNames))
					{
						strncpy(buttonName, keyNames[key], sizeof(buttonName));
					}
					else
					{
						snprintf(buttonName, sizeof(buttonName), "button%d", key + 1);
					}

					// 188 - copied from android. Joystick buttons should not use KeyCodes, but it is required. So joystick keycodes would
					// start from 188 as they do on Android.
					KeyEvent event(this, phase, buttonName, 188 + key % 100, false, false, false, false);
					runtime->DispatchEvent(event);
					break;
				}
				case JS_EVENT_AXIS:
				{
					//	printf("JS_EVENT_AXIS: type=%d, value=%d, axis=%d, axisname=%s\n", e.type, e.value, e.number, getAxisName(e.number));
					PlatformInputAxis *axis = GetAxes().GetByIndex(e.number);
					if (axis)
					{
						AxisEvent event(this, axis, e.value);
						runtime->DispatchEvent(event);
					}
					break;
				}
				default:
					Rtt_ASSERT(0);
			}
		}
	}

	const char *LinuxInputDevice::GetProductName()
	{
		return fSerialNumber.c_str();
	}

	bool LinuxInputDevice::AddNamedAxis(const char *axisName)
	{
		PlatformInputAxis *axis = AddAxis();

		auto it = sAxisTypes.find(axisName);

		if (it != sAxisTypes.end())
		{
			axis->SetType(it->second);
		}
		else
		{
			Rtt_LogException("unsupported axisName: %s\n", axisName);
			axis->SetType(InputAxisType::kGeneric1);
		}

		float physicalMax = 32767;
		float physicalMin = -32767;
		bool absolute = true;

		axis->SetMinValue(physicalMin);
		axis->SetMaxValue(physicalMax);
		axis->SetIsAbsolute(absolute);

		// TODO: find a (better) way to do this
		// axis->SetAccuracy( 0.01 );

		return physicalMax < 0 || physicalMin < 0;
	}

	const char *LinuxInputDevice::GetDisplayName()
	{
		return fSerialNumber.c_str();
	}

	const char *LinuxInputDevice::GetPermanentStringId()
	{
		return fSerialNumber.c_str();
	}

	InputDeviceConnectionState LinuxInputDevice::GetConnectionState()
	{
		return fConnected;
	}

	bool LinuxInputDevice::CanVibrate()
	{
		return fCanVibrate;
	}

	PlatformInputAxis *LinuxInputDevice::OnCreateAxisUsing(const InputAxisDescriptor &descriptor)
	{
		return Rtt_NEW(GetAllocator(), LinuxInputAxis(descriptor));
	}

	const char *LinuxInputDevice::GetDriverName()
	{
		return fDriverName.c_str();
	}

	void LinuxInputDevice::Vibrate()
	{
	}

	LinuxInputAxis::LinuxInputAxis(const InputAxisDescriptor &descriptor)
		: PlatformInputAxis(descriptor), centerPoint0(true)
	{
	}

	Rtt_Real LinuxInputAxis::GetNormalizedValue(Rtt_Real rawValue)
	{
		if (centerPoint0)
		{
			return PlatformInputAxis::GetNormalizedValue(rawValue);
		}

		Rtt_Real physicalMax = GetMaxValue();
		Rtt_Real physicalMin = GetMinValue();
		Rtt_Real scaledMin = Rtt_REAL_NEG_1;
		Rtt_Real scaledMax = Rtt_REAL_1;
		Rtt_Real ret = Rtt_RealDiv(Rtt_RealMul(rawValue - physicalMin, scaledMax - scaledMin), physicalMax - physicalMin) + scaledMin;
		ret = Clamp(ret, Rtt_REAL_NEG_1, Rtt_REAL_1);
		return ret;
	}
} // namespace Rtt

#endif
