//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CoronaLuaLibraryMetadata_H__
#define _CoronaLuaLibraryMetadata_H__

// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------

class LuaLibraryMetadata
{
	public:
		typedef LuaLibraryMetadata Self;

	public:
		LuaLibraryMetadata( const char *name, const char *publisherId, int version, int revision );
		~LuaLibraryMetadata();

	public:
		const char *GetName() const { return fName; }
		const char *GetPublisherId() const { return fPublisherId; }
		int GetVersion() const { return fVersion; }
		int GetRevision() const { return fRevision; }

	private:
		char *fName;
		char *fPublisherId;
		int fVersion;
		int fRevision;
};

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

#endif // _CoronaLuaLibraryMetadata_H__
