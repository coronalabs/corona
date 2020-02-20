//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_FilePath__
#define __Rtt_FilePath__

#include <string>
#include "Rtt_MPlatform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class UserdataWrapper;

// ----------------------------------------------------------------------------

// TODO: Rename to BitmapName
class FilePath
{
	public:
		typedef FilePath Self;

	public:
		static const char kMetatableName[];

	public:
		static int CreateAndPush(
			lua_State *L,
			Rtt_Allocator *allocator,
			const char *filename,
			MPlatform::Directory baseDir );

	// Metatable methods
	protected:
		static int Finalizer( lua_State *L );

	public:
		static void Initialize( lua_State *L );

	protected:
		FilePath( const char *filename, MPlatform::Directory baseDir );

	public:
		const char *GetFilename() const { return fFilename.c_str(); }
		MPlatform::Directory GetBaseDir() const { return (MPlatform::Directory)fBaseDir; }

	private:
		std::string fFilename;
		int fBaseDir;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_FilePath__
