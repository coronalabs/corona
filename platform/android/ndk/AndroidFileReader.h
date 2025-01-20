//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidFileReader_H__
#define _AndroidFileReader_H__

#include "Core/Rtt_Types.h"
#include "AndroidBinaryReader.h"

// Forward declarations.
struct Rtt_Allocator;


class AndroidFileReader : public AndroidBinaryReader
{
	public:
		AndroidFileReader(Rtt_Allocator *allocatorPointer);
		virtual ~AndroidFileReader();

		AndroidOperationResult Open(const char *filePath);
		AndroidOperationResult Open(FILE *filePointer);
		virtual bool IsOpen();
		virtual void Close();

	protected:
		virtual AndroidBinaryReadResult OnStreamTo(U8 *bytes, U32 count);

	private:
		Rtt::String fFilePath;
		FILE *fFilePointer;
};

#endif // _AndroidFileReader_H__
