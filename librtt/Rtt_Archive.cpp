//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Solar2D game engine.
// With contributions from Dianchu Technology
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_Archive.h"

#include "Rtt_LuaFile.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_FileSystem.h"

#if !defined( Rtt_NO_ARCHIVE )
	#include "Rtt_LuaContext.h"
	#include "Rtt_Runtime.h"
#endif

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#if defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV ) // || defined( Rtt_NXS_ENV )
	#include <io.h>
	#include <sys/stat.h>
	static const unsigned S_IRUSR = _S_IREAD;     ///< read by user
	static const unsigned S_IWUSR = _S_IWRITE;    ///< write by user
#elif defined( Rtt_ANDROID_ENV )
	#include "NativeToJavaBridge.h"
	#include <sys/mman.h>
#else
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/mman.h>
#endif

#include <errno.h>
#include <sys/stat.h>

// #define Rtt_DEBUG_ARCHIVE 1

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static bool
CopyFile( FILE *src, FILE *dst, long *numBytes )
{
	bool result = true;
	Rtt_ASSERT( src && dst );

	long startPos = ftell( dst );

	int c;
	while( (c = getc( src )) != EOF )
	{
		if ( ! Rtt_VERIFY( EOF != putc( c, dst ) ) )
		{
			Rtt_TRACE( ( "ERROR: Copy could not be copied!\n" ) );
			result = false;
			goto exit_gracefully;
		}
	}

/*
// Only works for plain text files
#if 1
    char buf[256];
    const size_t kNumElem = sizeof(buf) / sizeof(buf[0]);

    const size_t kElemSize = sizeof( buf[0] ); // obj size

    size_t objectsRead = 0;
    while ( ( objectsRead = fread( buf, kElemSize, kNumElem, src ) ) )
    {
        if ( fwrite( buf, kElemSize, objectsRead, dst ) < objectsRead )
        {
            result = false;
            goto exit_gracefully;
        }
    } 
#else
	char buf[256];
	const size_t kBufSize = sizeof(buf) / sizeof(buf[0]);

	while ( fgets( buf, kBufSize, src ) )
	{
		if ( ! Rtt_VERIFY( EOF != fputs( buf, dst ) ) )
		{
			Rtt_TRACE( ( "ERROR: Copy could not be copied!\n" ) );
			result = false;
			goto exit_gracefully;
		}
	}
#endif
*/

exit_gracefully:
	if ( numBytes )
	{
		*numBytes = ftell( dst ) - startPos;
	}
	return result;
}

template < size_t N >
static size_t
GetByteAlignedValue( size_t x )
{
	Rtt_STATIC_ASSERT( 0 == (N & (N-1)) ); // Ensure that N is a power of 2

	return ( x + (N-1) ) & (~(N-1));
}

static size_t
GetFileSize( const char *filepath )
{
	Rtt_ASSERT( filepath );

	struct stat statbuf;
	int result = stat( filepath, & statbuf );

	if (result != 0)
	{
		fprintf(stderr, "car: cannot stat file '%s'\n", filepath);
	}

	Rtt_UNUSED( result ); Rtt_ASSERT( 0 == result );

	return statbuf.st_size;
}

/*
#if defined( Rtt_DEBUG ) && !defined( Rtt_ANDROID_ENV )
static bool
Diff( const char *path1, const char *path2 )
{
	size_t len1 = GetFileSize( path1 );
	size_t len2 = GetFileSize( path2 );

	bool result = ( len1 == len2 );
	if ( result )
	{
#if defined( Rtt_WIN_DESKTOP_ENV ) || defined( Rtt_POWERVR_ENV )
		WinFile file1;
		WinFile file2;

		file1.Open( path1 );
		file1.Open( path2 );
		if ( !file1.IsOpen() || !file2.IsOpen() )
			return false;

		if ( file1.GetFileSize() != file2.GetFileSize() )
			return false;

		result = ( 0 == memcmp( file1.GetContents(), file2.GetContents(), file1.GetFileSize() ) );
#elif defined( Rtt_WIN_PHONE_ENV )
		//TODO: To be implemented later...
		Rtt_ASSERT_NOT_IMPLEMENTED();
#else
		int fd1 = open( path1, O_RDONLY );
		void *p1 = mmap( NULL, len1, PROT_READ, MAP_SHARED, fd1, 0 );

		int fd2 = open( path2, O_RDONLY );
		void *p2 = mmap( NULL, len2, PROT_READ, MAP_SHARED, fd2, 0 );

		result = ( 0 == memcmp( p1, p2, len1 ) );

		munmap( p2, len2 );
		close( fd2 );

		munmap( p1, len1 );
		close( fd1 );
#endif
	}

	return result;
}
#endif
*/

static char
DirSeparator()
{
#if defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV )
	return '\\';
#else
	return '/';
#endif
}

static const char*
GetBasename( const char* path )
{
    const char kDirSeparator = DirSeparator();

    const char* result = path;
    for ( path = strchr( path, kDirSeparator );
          path && '\0' != *path;
          path = strchr( path, kDirSeparator ) )
    {
         ++path;
         result = path;
    }

    return result;
}

// ----------------------------------------------------------------------------

struct ArchiveWriterEntry
{
	U32 type;
	U32 offset;
	const char* name;
	size_t nameLen;
	const char* srcPath;
	size_t srcLen;
};

class ArchiveWriter
{
	public:
		enum
		{
			kTagSize = sizeof(U32)*2,
			kVersion = 0x1
		};

	public:
		ArchiveWriter();
		~ArchiveWriter();

	public:
		int Initialize( const char *dstPath );

	public:
		int Serialize( Archive::Tag tag, U32 len ) const;
		int Serialize( U32 value ) const;
		int Serialize( const char *value, size_t len ) const;
		int Serialize( const char *filepath ) const;

	public:
//		int Serialize( ArchiveWriterEntry& entry );

	public:
		S32 GetPosition() const;

	private:
		FILE *fDst;
};

// ----------------------------------------------------------------------------

ArchiveWriter::ArchiveWriter()
:	fDst( NULL )
{
}

ArchiveWriter::~ArchiveWriter()
{
	if ( Rtt_VERIFY( fDst ) )
	{
		Rtt_FileClose( fDst );
	}
}

int
ArchiveWriter::Initialize( const char *dstPath )
{
	int result = 0;

	fDst = Rtt_FileOpen(dstPath, "wb");

	if (fDst == NULL)
	{
		fprintf(stderr, "car: cannot open archive '%s' for writing\n", dstPath);
	}
	else
	{
		FILE *dst = fDst;
		result += fprintf( dst, "%c", 'r');
		result += fprintf( dst, "%c", 'a');
		result += fprintf( dst, "%c", 'c');
		result += fprintf( dst, "%c", kVersion );
	}

	return result;
}

int
ArchiveWriter::Serialize( Archive::Tag tag, U32 len ) const
{
	Rtt_ASSERT( fDst );

	int result = Serialize( tag );
	result += Serialize( len );
	return result;
}

int
ArchiveWriter::Serialize( U32 value ) const
{
	Rtt_ASSERT( fDst );
	return fprintf( fDst, "%c%c%c%c",
		(unsigned char)(value & 0xFF),
		(unsigned char)(value >> 8 & 0xFF),
		(unsigned char)(value >> 16 & 0xFF),
		(unsigned char)(value >> 24 & 0xFF) );
}

int
ArchiveWriter::Serialize( const char *value, size_t len ) const
{
	Rtt_ASSERT( fDst );

	FILE *dst = fDst;
	int result = Serialize( (U32) len );

	// size_t len4 = (len + 3) & (~0x3); // next multiple of 4 if len is not a multiple of 4

	++len; // increment to include space for '\0'-termination
	size_t len4 = GetByteAlignedValue< 4 >( len );
	for ( size_t i = 0; i < len; i++ )
	{
		result += fprintf( dst, "%c", value[i] );
	}

	// Pad 0's to 4-byte align
	for ( size_t i = len; i < len4; i++ )
	{
		result += fprintf( dst, "%c", 0 );
	}

	return result;
}

int
ArchiveWriter::Serialize( const char *filepath ) const
{
	Rtt_ASSERT( fDst );
	Rtt_ASSERT( filepath );

	int result = 0;

	FILE *src = Rtt_FileOpen( filepath, "rb" );

	if (src == NULL)
	{
		fprintf(stderr, "car: cannot serialize file '%s' (%s)\n", filepath, strerror(errno));

		return 0;
	}
	else
	{
		size_t len = GetFileSize( filepath );
		size_t len4 = GetByteAlignedValue< 4 >( len );

		FILE *dst = fDst;
		long numBytes = 0;
		if ( Rtt_VERIFY( CopyFile( src, dst, & numBytes ) ) )
		{
			Rtt_ASSERT( numBytes >= 0 && (size_t)numBytes == len );

			size_t numChars = len4 - len;
			Rtt_ASSERT( numChars < 4 );
			switch( numChars )
			{
				case 3:
					fprintf( dst, "%c", '\0' );
				case 2:
					fprintf( dst, "%c", '\0' );
				case 1:
					fprintf( dst, "%c", '\0' );
				default:
					break;
			}

			result += len4;
		}
		else
		{
			result += numBytes;
		}

		Rtt_FileClose(src);
	}

	return result;
}

/*
int
ArchiveWriter::Serialize( ArchiveWriterEntry& entry )
{
}
*/

S32
ArchiveWriter::GetPosition() const
{
	Rtt_ASSERT( fDst );
	return (S32) ftell( fDst );
}

// ----------------------------------------------------------------------------

class ArchiveReader
{
	public:
		ArchiveReader();
//		~ArchiveReader();

		bool Initialize( const void* data, size_t numBytes );

	public:
		U32 ParseTag( U32& rLength );
		U32 ParseU32();
		const char* ParseString();
		void* ParseData( U32& rLength );

	public:
		bool Seek( S32 offset, bool fromOrigin );

	protected:
		void VerifyBounds() const;

	private:
		const void *fPos;
		const void *fData;
		size_t fDataLen;
		U8 fVersion;
};

ArchiveReader::ArchiveReader()
:	fPos( NULL ),
	fData( NULL ),
	fDataLen( 0 ),
	fVersion( 0 )
{
}

bool
ArchiveReader::Initialize( const void* data, size_t numBytes )
{
	const U8 kHeader[] = { 'r', 'a', 'c', ArchiveWriter::kVersion };
	const size_t kHeaderSize = sizeof( kHeader );
	bool result = ( data && numBytes > kHeaderSize && 0 == memcmp( data, kHeader, kHeaderSize ) );
	if ( result )
	{
		fPos = ((U8*)data) + kHeaderSize;
		fData = data;
		fDataLen = numBytes;
		fVersion = ArchiveWriter::kVersion;

#if Rtt_DEBUG_ARCHIVE
		Rtt_TRACE( ( "[ArchiveReader::Initialize] inData(%p) fPos(%p) fData(%p) headerSize(%ld) fDataLen(%ld)\n",
			data, fPos, fData, kHeaderSize, fDataLen ) );
#endif
	}
#if Rtt_DEBUG_ARCHIVE
	else
	{
		Rtt_TRACE( ( "[ArchiveReader::Initialize] header check failed: numBytes %ld, kHeaderSize %ld\n",
			numBytes, kHeaderSize ) );
	}
#endif

	return result;
}

static U32
ReadU32( U32 *p )
{
	#ifdef Rtt_LITTLE_ENDIAN
		return *p;
	#else
		U8 *pp = (U8*)p;
		return ((U32)pp[0])
				| (((U32)pp[1]) << 8)
				| (((U32)pp[2]) << 16)
				| (((U32)pp[3]) << 24);
	#endif
}

U32
ArchiveReader::ParseTag( U32& rLength )
{
	VerifyBounds();

	U32 *p = (U32*)fPos;

	U32 tag = ReadU32( p ); p++; // *p++;
	rLength = ReadU32( p ); p++; // *p++;
	fPos = p;

	VerifyBounds();
	return tag;
}

U32
ArchiveReader::ParseU32()
{
	VerifyBounds();

	U32 *p = (U32*)fPos;

	U32 result = ReadU32( p ); p++; // *p++;
	fPos = p;

	VerifyBounds();
	return result;
}

const char*
ArchiveReader::ParseString()
{
	VerifyBounds();

	U32 *p = (U32*)fPos;

	U32 len = ReadU32( p ); p++; // *p++;
	const char* result = reinterpret_cast< char* >( p );
	Rtt_ASSERT( strlen( result ) == len );

	// 4-byte alignment was calculated using string size (which includes '\0' termination byte)
	fPos = p + GetByteAlignedValue< 4 >( len + 1 ) / sizeof( *p );

	VerifyBounds();
	return result;
}

void*
ArchiveReader::ParseData( U32& rLength )
{
	VerifyBounds();

	U32 *p = (U32*)fPos;

	U32 len = ReadU32( p ); p++; // *p++;
	rLength = len;
	void* result = p;

	fPos = p + GetByteAlignedValue< 4 >( len ) / sizeof( *p );

	VerifyBounds();
	return result;
}

bool
ArchiveReader::Seek( S32 offset, bool fromOrigin )
{
	VerifyBounds();

	bool result = false;

	if ( fromOrigin )
	{
		result = ( offset >= 0 );
		if ( result )
		{
			fPos = ((U8*)fData) + offset;
		}
	}
	else
	{
		void *p = ((U8*)fPos) + offset;
		void *pEnd = ((U8*)fData) + fDataLen;
		result = ( p >= fData && p < pEnd );
		if ( result )
		{
			fPos = p;
		}
	}

	VerifyBounds();
	return result;
}

void
ArchiveReader::VerifyBounds() const
{
	// fprintf(stderr, "fPos %p, fData %p, fDataLen 0x%lx (%d)\n", fPos, fData, fDataLen, fDataLen);
	Rtt_ASSERT( fPos >= fData && fPos < (((U8*)fData) + fDataLen ) );
}

// ----------------------------------------------------------------------------

void
Archive::Serialize( const char *dstPath, int numSrcPaths, const char *srcPaths[] )
{
	std::vector<std::string> fileList;
	size_t fileCount = 0;

#ifdef WIN32
	char tmpDirTemplate[_MAX_PATH];
	const char* tmp = getenv("TMP");
	if (tmp == NULL)
	{
		tmp = getenv("TEMP");
	}

	if (tmp)
	{
		_snprintf(tmpDirTemplate, sizeof(tmpDirTemplate), "%s\\CBXXXXXX", tmp);
	}
	else
	{
		strcpy(tmpDirTemplate, "\\tmp\\CBXXXXXX");
	}
#else
	char tmpDirTemplate[] = "/tmp/CBXXXXXX";
#endif

	const char *tmpDirName = Rtt_MakeTempDirectory(tmpDirTemplate);

	if (Rtt_FileExists(dstPath))
	{
		// Archive already exists, extract the current contents so we can overwrite
		// old files with fresh copies and add any new ones (this is done to avoid
		// implementing actual archive editing)

		// First extract any files already in the archive to preserve them
		Deserialize(tmpDirName, dstPath);

		// Copy all the new files to the temporary directory (overwriting any with the same
		// name that might have been extracted to preserve the semantics of car files)
		for ( int i = 0; i < numSrcPaths; i++ )
		{
			String tmpFileCopy(tmpDirName);
			tmpFileCopy.AppendPathComponent(GetBasename(srcPaths[i]));

			if ( ! Rtt_CopyFile(srcPaths[i], tmpFileCopy.GetString()))
			{
				fprintf(stderr, "car: cannot open '%s' for reading\n", srcPaths[i]);

				return;
			}
		}

		// Enumerate the files now in the temporary directory and make the new archive with them
		fileList = Rtt_ListFiles(tmpDirName);
		fileCount = fileList.size();
	}
	else
	{
		// Archive doesn't already exist so we can't be updating it,
		// just copy the file paths to our file list

		for ( int i = 0; i < numSrcPaths; i++ )
		{
			fileList.push_back(srcPaths[i]);
		}

		fileCount = numSrcPaths;
	}

	ArchiveWriter writer;
	int startPos = writer.Initialize( dstPath );
	if ( Rtt_VERIFY( startPos > 0 ) )
	{
		ArchiveWriterEntry* entries = new ArchiveWriterEntry[fileCount];

		U32 contentsLen = sizeof(U32); // numElements
		size_t entryIdx = 0;
		for (std::vector<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
		{
			ArchiveWriterEntry& entry = entries[entryIdx++];
			entry.type = kLuaObjectResource;
			entry.offset = 0;

			const char* path = it->c_str();
			entry.name = GetBasename( path );
			entry.nameLen = strlen( entry.name );
			entry.srcPath = path;
			entry.srcLen = GetFileSize( path );

			// type, offset, numChars, string data
			contentsLen += 3*sizeof(U32) + GetByteAlignedValue< 4 >( entry.nameLen + 1 );
		}

		U32 offsetBase = startPos + contentsLen;

		offsetBase += writer.Serialize( Archive::kContentsTag, contentsLen );

		// Contents
		// --------------------------
		//   U32        numElements
		//   Record[]   {
		//                U32 type
		//                U32 offset
		//                String name
		//              }
		// 
		// String
		// --------------------------
		//   U32        length
		//   U8[]	    bytes (4 byte-aligned padding)
		writer.Serialize( (int)fileCount );
		for ( size_t i = 0; i < fileCount; i++ )
		{
			ArchiveWriterEntry& entry = entries[i];
			writer.Serialize( entry.type );
			writer.Serialize( offsetBase );
			writer.Serialize( entry.name, entry.nameLen );

			// store offset for this entry
			entry.offset = offsetBase;

			// For next offset, add srcLen *and* bytes for tag, length 
			offsetBase +=
				GetByteAlignedValue< 4 >( entry.srcLen )
				+ ArchiveWriter::kTagSize
				+ sizeof(U32);
		}

		// Data
		// --------------------------
		//   String     data
		for ( size_t i = 0; i < fileCount; i++ )
		{
			ArchiveWriterEntry& entry = entries[i];

			Rtt_ASSERT(
				writer.GetPosition() >= 0
				&& (size_t)writer.GetPosition() == entry.offset );

			// data tag length = sizeof( length ) + byte-aligned len of bytes buffer
			writer.Serialize( kDataTag, sizeof( U32 ) + (U32) GetByteAlignedValue< 4 >( entry.srcLen ) );
			writer.Serialize( (U32) entry.srcLen );
			writer.Serialize( entry.srcPath );
		}

		// EOF
		writer.Serialize( kEOFTag, 0 );

		delete [] entries;
	}

	Rtt_DeleteDirectory(tmpDirName);
}

static void
WriteFile( const char *dstDir, const char *filename, const void *src, size_t srcNumBytes )
{
	Rtt_ASSERT( dstDir );
	Rtt_ASSERT( filename );
	Rtt_ASSERT( src );

	String path(dstDir);
	path.AppendPathComponent(filename);

	int fd = Rtt_FileDescriptorOpen( path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );

	if (fd == -1)
	{
		fprintf(stderr, "car: cannot open '%s' for writing\n", path.GetString());

		return;
	}
	else
	{
		// Set size of file
		_lseek( fd, srcNumBytes - 1, SEEK_SET );
		_write( fd, "", 1 );

		int canWrite = 1;
		void *dst = Rtt_FileMemoryMap( fd, 0, srcNumBytes, canWrite );
		if (dst == NULL)
		{
			fprintf(stderr, "car: cannot map '%s' for writing\n", path.GetString());

			return;
		}
		else
		{
			*(U32*)dst = 0xdeadbeef;
			memcpy( dst, src, srcNumBytes );
			Rtt_FileMemoryUnmap( dst, srcNumBytes );
		}

		Rtt_FileDescriptorClose( fd );
	}
}

size_t
Archive::Deserialize( const char *dstDir, const char *srcCarFile )
{
	int fd = Rtt_FileDescriptorOpen(srcCarFile, O_RDONLY, S_IRUSR);
	struct stat statbuf;
	size_t count = 0;

	if (fd == -1)
	{
		fprintf(stderr, "car: cannot open archive '%s'\n", srcCarFile);

		return 0;
	}

	if (fstat( fd, & statbuf ) == -1)
	{
		fprintf(stderr, "car: cannot stat archive '%s'\n", srcCarFile);

		return 0;
	}

	size_t dataLen = statbuf.st_size;

	void *data = Rtt_FileMemoryMap(fd, 0, dataLen, false);

	Rtt_FileDescriptorClose(fd);

	ArchiveReader reader;
	if (reader.Initialize(data, dataLen) == 0)
	{
		fprintf(stderr, "car: file '%s' is not a car archive\n", srcCarFile);
	}
	else
	{
		U32 tag = kUnknownTag;
		//while( kEOFTag != tag )
		{
			U32 tagLen;
			tag = reader.ParseTag( tagLen );
			switch( tag )
			{
				case kContentsTag:
					{
						U32 numElements = reader.ParseU32();
						ArchiveEntry *entries = (ArchiveEntry*)Rtt_MALLOC( & allocator, sizeof( ArchiveEntry )*numElements );
						for ( U32 i = 0; i < numElements; i++ )
						{
							ArchiveEntry& entry = entries[i];
							entry.type = reader.ParseU32();
							entry.offset = reader.ParseU32();
							entry.name = reader.ParseString();
						}

						for ( U32 i = 0; i < numElements; i++ )
						{
							ArchiveEntry& entry = entries[i];

							reader.Seek( entry.offset, true );
							U32 tagLen;
							U32 tag = reader.ParseTag( tagLen );
							if ( Rtt_VERIFY( Archive::kDataTag == tag ) )
							{
								U32 resourceLen = 0;
								void* resource = reader.ParseData( resourceLen );
								WriteFile( dstDir, entry.name, resource, resourceLen );
								++count;
							}
						}

						Rtt_FREE( entries );
					}
					break;
				default:
					Rtt_ASSERT_NOT_REACHED();
					break;
			}
		}
	}

	if (data != NULL)
	{
		Rtt_FileMemoryUnmap(data, dataLen);
	}

	return count;
}

void
Archive::List(const char *srcCarFile)
{
	int fd = Rtt_FileDescriptorOpen(srcCarFile, O_RDONLY, S_IRUSR);
	struct stat statbuf;

	if (fd == -1)
	{
		fprintf(stderr, "car: cannot open archive '%s'\n", srcCarFile);

		return;
	}

	if (fstat( fd, & statbuf ) == -1)
	{
		fprintf(stderr, "car: cannot stat archive '%s'\n", srcCarFile);

		return;
	}
	size_t dataLen = statbuf.st_size;

	void *data = Rtt_FileMemoryMap(fd, 0, dataLen, false);

	Rtt_FileDescriptorClose(fd);

	ArchiveReader reader;
	if (reader.Initialize(data, dataLen) == 0)
	{
		fprintf(stderr, "car: file '%s' is not a car archive\n", srcCarFile);
	}
	else
	{
		U32 tag = kUnknownTag;
		//while( kEOFTag != tag )
		{
			U32 tagLen;
			tag = reader.ParseTag( tagLen );
			switch( tag )
			{
			case kContentsTag:
			{
				U32 numElements = reader.ParseU32();
				ArchiveEntry *entries = (ArchiveEntry*)Rtt_MALLOC( & allocator, sizeof( ArchiveEntry )*numElements );
				for ( U32 i = 0; i < numElements; i++ )
				{
					ArchiveEntry& entry = entries[i];
					entry.type = reader.ParseU32();
					entry.offset = reader.ParseU32();
					entry.name = reader.ParseString();
				}

				for ( U32 i = 0; i < numElements; i++ )
				{
					ArchiveEntry& entry = entries[i];

					reader.Seek( entry.offset, true );
					U32 tagLen;
					U32 tag = reader.ParseTag( tagLen );
					if ( Rtt_VERIFY( Archive::kDataTag == tag ) )
					{
						U32 resourceLen = 0;
						reader.ParseData( resourceLen );
						printf("%7d %s\n", resourceLen, entry.name);
					}
				}

				Rtt_FREE( entries );
			}
			break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
			}
		}
	}

	if (data != NULL)
	{
		Rtt_FileMemoryUnmap(data, dataLen);
	}
}

// ----------------------------------------------------------------------------

#if !defined( Rtt_NO_ARCHIVE )

// ----------------------------------------------------------------------------

Archive::Archive( Rtt_Allocator& allocator, const char *srcPath )
:	fAllocator( allocator ),
	fEntries( NULL ),
	fNumEntries( 0 ),
#if defined( Rtt_ARCHIVE_COPY_DATA )
	fBits( &allocator ),
#endif
	fData( NULL )
{
#if defined( Rtt_WIN_PHONE_ENV ) || defined(Rtt_NXS_ENV)
	FILE* filePointer = Rtt_FileOpen(srcPath, "rb");
	if (filePointer)
	{
		fseek(filePointer, 0, SEEK_END);
		fDataLen = ftell(filePointer);
		if (fDataLen > 0)
		{
			const size_t MAX_BYTES_PER_READ = 1024;
			rewind(filePointer);
			fData = Rtt_MALLOC(&allocator, fDataLen);
			for (long totalBytesRead = 0; totalBytesRead < fDataLen;)
			{
				size_t bytesRead = fread(((U8*)fData + totalBytesRead), 1, MAX_BYTES_PER_READ, filePointer);
				if (bytesRead < MAX_BYTES_PER_READ)
				{
					int errorNumber = errno;
					if (ferror(filePointer) && errorNumber)
					{
						Rtt_FREE((void*)fData);
						fData = NULL;
						fDataLen = 0;
						Rtt_LogException(strerror(errorNumber));
						break;
					}
				}
				totalBytesRead += (long)bytesRead;
				if (feof(filePointer))
				{
					fDataLen = totalBytesRead;
					break;
				}
			}
		}
		Rtt_FileClose(filePointer);
	}
#else
	int fileDescriptor = Rtt_FileDescriptorOpen(srcPath, O_RDONLY, S_IRUSR);
	struct stat statbuf;
	int result = fstat(fileDescriptor, &statbuf); Rtt_ASSERT(result >= 0);

	int canWrite = 0;
	fData = Rtt_FileMemoryMap(fileDescriptor, 0, statbuf.st_size, canWrite);

	fDataLen = statbuf.st_size;
	Rtt_FileDescriptorClose(fileDescriptor);
#endif

#if defined( EMSCRIPTEN )
	// On browser, mmap is not reliable wrt byte-alignment,
	// so ensure byte-alignment by copying data to a malloc'd buffer
	fBits.Set( (const char *)fData, fDataLen );
	munmap( (void *)fData, fDataLen );
	fData = fBits.Get();
#endif


#if Rtt_DEBUG_ARCHIVE

	Rtt_TRACE( ( "[Archive] fDataLen(%ld)\n", fDataLen ) );
	const U8 *bytes = (const U8 *)fData;
	for ( size_t i = 0; i < fDataLen; i++ )
	{
		if ( i % 16 == 0 )
		{
			if ( i > 0 )
			{
				Rtt_TRACE( ( "\n" ) );
			}
			Rtt_TRACE( ( "%08lx  ", i ) );
		}

		Rtt_TRACE( ( "%02x ", bytes[i] ) );

		if ( i > 0 && (i+1) % 8 == 0 )
		{
			Rtt_TRACE( ( " " ) );
		}
	}
	Rtt_TRACE( ( "\n" ) );
#endif

	ArchiveReader reader;
	if ( Rtt_VERIFY( reader.Initialize( fData, fDataLen ) ) )
	{
		U32 tag = kUnknownTag;
		//while( kEOFTag != tag )
		{
			U32 tagLen;
			tag = reader.ParseTag( tagLen );
			switch( tag )
			{
				case kContentsTag:
					{
						U32 numElements = reader.ParseU32();
						fEntries = (ArchiveEntry*)Rtt_MALLOC( & allocator, sizeof( ArchiveEntry )*numElements );
						fNumEntries = numElements;
#if Rtt_DEBUG_ARCHIVE
						Rtt_TRACE( ( "[Archive::Archive] fNumEntries %ld, fEntries %p\n", fNumEntries, fEntries ) );
#endif

						for ( U32 i = 0; i < numElements; i++ )
						{
							ArchiveEntry& entry = fEntries[i];
							entry.type = reader.ParseU32();
							entry.offset = reader.ParseU32();
							entry.name = reader.ParseString();
						}
					}
					break;
				default:
					Rtt_TRACE( ( "Unknown tag: %d\n", tag ) );
					Rtt_ASSERT_NOT_REACHED();
					break;
			}
		}
	}
}

Archive::~Archive()
{
#if defined( Rtt_EMSCRIPTEN_ENV ) || defined( Rtt_NXS_ENV )
	// Do nothing.
#elif defined( Rtt_WIN_PHONE_ENV )
	Rtt_FREE((void*)fData);
#else
	if ( Rtt_VERIFY( fData ) )
	{
#if defined( Rtt_WIN_DESKTOP_ENV ) || defined( Rtt_POWERVR_ENV )
		Rtt_FileMemoryUnmap( fData, fDataLen );
#else
//TODO: Merge this code with the Windows code block up above using the new Rtt_File*() functions.
		munmap( (void*)fData, fDataLen );
#endif
	}
#endif

	Rtt_FREE( fEntries );

}

// This will be added to the list of Lua loaders called via "require"
int
Archive::ResourceLoader( lua_State *L )
{
	const char *name = luaL_checkstring(L, 1);

	Runtime* runtime = LuaContext::GetRuntime( L );

	Archive* archive = runtime->GetArchive();

	const char kExtension[] = "." Rtt_LUA_OBJECT_FILE_EXTENSION;
	const size_t nameLen = strlen( name );
	const size_t filenameByteLength = nameLen + sizeof( kExtension );
	char *filename = (char*)malloc( filenameByteLength );
	snprintf( filename, filenameByteLength, "%s%s", name, kExtension );

	// Actually, we shouldn't throw an error, so ignore the status.
	// As long as we push an error string, ll_require will take care of
	// appending the strings and throw an error if all loaders fail to
	// find the module.
	int status = archive->LoadResource( L, filename ); Rtt_UNUSED( status );

	free( filename );

/*
	// Should not throw an error b/c the module could be part of the Corona preloaded libs.
	if ( 0 != status )
	{
		luaL_error(L, "error loading module " LUA_QS ":\n\t%s", name, lua_tostring(L, -1));
	}
*/

	return 1;
}

int
Archive::LoadResource( lua_State *L, const char *name )
{
	int status = LUA_ERRFILE;

	const char kFormatResourceNotFound[] = "resource (%s) does not exist in archive";
	const char kFormatAchiveCorrupted[] = "archive is corrupted. could not resolve resource (%s)";
	const char *errorFormat = kFormatResourceNotFound;

	ArchiveReader reader;

	if ( fData == NULL )
		goto exit_gracefully;

	reader.Initialize( fData, fDataLen );

	for ( size_t i = 0, iMax = fNumEntries; i < iMax; i++ )
	{
		ArchiveEntry& entry = fEntries[i];
		if ( 0 == Rtt_StringCompare( entry.name, name ) )
		{
			reader.Seek( entry.offset, true );
			U32 tagLen;
			U32 tag = reader.ParseTag( tagLen );
			if ( Rtt_VERIFY( Archive::kDataTag == tag ) )
			{
				U32 resourceLen = 0;
				void* resource = reader.ParseData( resourceLen );
				status = luaL_loadbuffer( L, static_cast< const char* >( resource ), resourceLen, name );
				goto exit_gracefully;
			}
			errorFormat = kFormatAchiveCorrupted;
		}
	}

#if defined( Rtt_DEBUG ) && defined( Rtt_ANDROID_ENV )
	Rtt_Log( errorFormat, name );
#endif

	lua_pushfstring( L, errorFormat, name );

exit_gracefully:
	return status;
}

int
Archive::DoResource( lua_State *L, const char *name, int narg )
{
#if Rtt_DEBUG_ARCHIVE
	Rtt_TRACE( ( "[Archive::DoResource] name(%s)\n", name ) );
#endif
	int status = LoadResource( L, name );

	if ( 0 == status )
	{
		int base = lua_gettop( L ) - narg;
		lua_insert( L, base ); // move chunk underneath args

		status = LuaContext::DoCall( L, narg, 0 );
#ifdef Rtt_DEBUG
		if ( status && LuaContext::HasRuntime( L ) )
		{
			// We should only assert if the call failed on a Lua state associated with a Corona runtime.
			// Note: Mac and Win32 desktop apps need to load a "resource.car" before creating a Corona runtime
			//       in order to read its archived "build.settings" to pre-configure the desktop window.
			Rtt_ASSERT( 0 );
		}
#endif
	}

	return status;
}

// ----------------------------------------------------------------------------

#endif // Rtt_NO_ARCHIVE

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
