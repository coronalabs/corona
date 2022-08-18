//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && _MSC_VER >= 1400
#include <vld.h>
#include <direct.h>
#endif

#include <assert.h>
#include "Rtt_EmscriptenContext.h"

int main(int argc, char *argv[])
{
	using namespace Rtt;

	#ifdef EMSCRIPTEN
		CoronaAppContext context("");
	#else
		// for debugging

		Rtt_ASSERT(argc > 1);
		const char* app = argv[1];

//	  char* workdir = _getcwd( NULL, 0 );
		_chdir(app);  
		CoronaAppContext context(app);
	#endif

	context.Start();



	return 0;
}
