#ifndef Rtt_Linux_Runtime
#define Rtt_Linux_Runtime

#include "Rtt_Runtime.h"

namespace Rtt
{
	class LinuxRuntime : public Runtime
	{
	public:
		LinuxRuntime(const MPlatform &platform, MCallback *viewCallback = NULL);
	};
};

#endif // Rtt_Linux_Runtime
