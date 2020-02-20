//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidZipFileEntry_H__
#define _AndroidZipFileEntry_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"


/// Store information about one zip entry within a zip file.
class AndroidZipFileEntry
{
	public:
		AndroidZipFileEntry(Rtt_Allocator *allocatorPointer);
		virtual ~AndroidZipFileEntry();

		const char* GetPackageFilePath() const;
		void SetPackageFilePath(const char *path);
		const char* GetEntryName() const;
		void SetEntryName(const char *name);
		long GetByteOffsetInPackage() const;
		void SetByteOffsetInPackage(long value);
		long GetByteCountInPackage() const;
		void SetByteCountInPackage(long value);
		bool IsCompressed() const;
		void SetIsCompressed(bool value);

	private:
		Rtt::String fPackageFilePath;
		Rtt::String fEntryName;
		long fByteOffsetInPackage;
		long fByteCountInPackage;
		bool fIsCompressed;
};

#endif // _AndroidZipFileEntry_H__
