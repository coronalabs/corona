//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_LinuxInputDevice__
#define __Rtt_LinuxInputDevice__

#include "Input/Rtt_PlatformInputDeviceManager.h"
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

namespace Rtt
{
	class Runtime;

	class LinuxInputDevice : public PlatformInputDevice
	{
	public:
		typedef PlatformInputDevice Super;
		LinuxInputDevice(const InputDeviceDescriptor &descriptor);
		virtual ~LinuxInputDevice();
		virtual const char *GetProductName() override;
		virtual const char *GetDisplayName() override;
		virtual const char *GetPermanentStringId() override;
		virtual InputDeviceConnectionState GetConnectionState() override;
		virtual void Vibrate() override;
		virtual bool CanVibrate() override;
		virtual const char *GetDriverName() override;
		PlatformInputAxis *OnCreateAxisUsing(const InputAxisDescriptor &descriptor) override;
		bool AddNamedAxis(const char *axis);
		void init(const char *dev);
		void dispatchEvents(Runtime *runtime);
		const char *getAxisName(int i);

	public:
		InputDeviceConnectionState fConnected;
		std::string fSerialNumber;
		bool fCanVibrate;
		int fd;
		int fAxesCount;
		std::string fDriverName;
		uint8_t *fAxesMap;
	};

	class LinuxInputAxis : public PlatformInputAxis
	{
	public:
		bool centerPoint0;
		LinuxInputAxis(const InputAxisDescriptor &descriptor);
		virtual ~LinuxInputAxis() {};
		virtual Rtt_Real GetNormalizedValue(Rtt_Real rawValue) override;
	};
}; // namespace Rtt

#endif //__Rtt_LinuxInputDevice__
