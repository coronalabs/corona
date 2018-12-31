#ifndef ALMIXER_RWOPS
#define ALMIXER_RWOPS

	#if defined(_WIN32) || defined(WINAPI_FAMILY)
		#if defined(ALMIXER_RWOPS_BUILD_LIBRARY)
			#define ALMIXER_RWOPS_DECLSPEC __declspec(dllexport)
		#else
			#define ALMIXER_RWOPS_DECLSPEC
		#endif
	#else
		#if defined(ALMIXER_RWOPS_BUILD_LIBRARY)
			#if defined (__GNUC__) && __GNUC__ >= 4
				#define ALMIXER_RWOPS_DECLSPEC __attribute__((visibility("default")))
			#else
				#define ALMIXER_RWOPS_DECLSPEC
			#endif
		#else
			#define ALMIXER_RWOPS_DECLSPEC
		#endif
	#endif

	#if defined(_WIN32) || defined(WINAPI_FAMILY)
		#define ALMIXER_RWOPS_CALL __cdecl
	#else
		#define ALMIXER_RWOPS_CALL
	#endif

#ifdef __cplusplus
extern "C" {
#endif

	/* Trying to keep compatibility with SDL_RWops,
	 * but I don't plan on reimplementing everything.
	 */
	#include <stdio.h>
	#include <stddef.h>
	/* The 'type' parameter needs to be 32-bits to match SDL_RWops.
	 * <stdint.h> has been problematic for Visual Studio for years.
	 * It appears that Visual Studio 2010 finally includes this.
	 */
	#include <stdint.h>
	typedef struct ALmixer_RWops 
	{
		/** Seek to 'offset' relative to whence, one of stdio's whence values:
		 *	SEEK_SET, SEEK_CUR, SEEK_END
		 *  Returns the final offset in the data source.
		 *  (Note this is different than stdio's seek. This returns ftell.)
		 */
		long (ALMIXER_RWOPS_CALL *seek)(struct ALmixer_RWops* the_context, long offset, int whence);

		/** Read up to 'nitems' objects each of size 'size' from the data
		 *  source to the area pointed at by 'ptr'.
		 *  Returns the number of objects read, or -1 if the read failed.
		 */
		size_t (ALMIXER_RWOPS_CALL *read)(struct ALmixer_RWops* the_context, void* ptr, size_t size, size_t nitems);

		/** Write exactly 'nitems' objects each of size 'size' from the area
		 *  pointed at by 'ptr' to data source.
		 *  Returns 'num', or -1 if the write failed.
		 */
		size_t (ALMIXER_RWOPS_CALL *write)(struct ALmixer_RWops* the_context, const void* ptr, size_t size, size_t nitems);

		/** Close and free an allocated ALmixer_RWops structure */
		int (ALMIXER_RWOPS_CALL *close)(struct ALmixer_RWops* the_context);

		/* type Needs to be a 32-bit to be compatible with SDL */
		uint32_t type;
		union 
		{
#if defined(__WIN32__) && !defined(__SYMBIAN32__)
			/* Chances are that I will not implement this path.
			 * But I want the padding to be the same as SDL.
			 */
			struct 
			{
				int   append;
				void *h;
				struct
				{
					void *data;
					size_t size;
					size_t left;
				} buffer;
			} win32io;
#endif
	    	struct 
			{
				int autoclose;
	 			FILE* fp;
	    	} stdio;
	   		struct 
			{
				unsigned char* base;
			 	unsigned char* here;
				unsigned char* stop;
			} mem;
		  	struct 
			{
				void* data1;
	    	} unknown;
		} hidden;
	} ALmixer_RWops;

	extern ALMIXER_RWOPS_DECLSPEC ALmixer_RWops* ALMIXER_RWOPS_CALL ALmixer_RWFromFile(const char* file_name, const char* file_mode);
	extern ALMIXER_RWOPS_DECLSPEC ALmixer_RWops* ALMIXER_RWOPS_CALL ALmixer_RWFromFP(FILE* file_pointer, char autoclose_flag);

#define ALmixer_RWseek(rwops, offset, whence) (rwops)->seek(rwops, offset, whence)
#define ALmixer_RWtell(rwops) (rwops)->seek(rwops, 0, SEEK_CUR)
#define ALmixer_RWread(rwops, ptr, size, nitems) (rwops)->read(rwops, ptr, size, nitems)
	
/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#ifdef NINTENDO_LIB
#define fclose	nFileClose
#define fopen	nFileOpen
#define freopen	nFileReopen
#define fdopen	nFileReopen
#define remove	nFileRemove
#define rename	nFileRename
#define rewind nFileRewind
#define tmpfile nFileTmpfile
#define clearerr nFileClearerr
#define feof nFileEof
#define ferror nFileError
#define fflush nFileFlush
#define fgetpos nFileFgetpos
#define fgetc nFileFgetC
#define fgets nFileFgetS
#define fputc nFileFputC
#define fputs nFileFputS
#define ftell    nFileTell
#define fseek    nFileSeek
#define fsetpos   nFileSetpos
#define fread    nFileRead
#define fwrite   nFileWrite
#define getc    nFileGetC
#define getchar    nFileGetC
#define putc    nFilePutC
#define putchar    nFilePutC
#define fputchar    nFilePutC
//#define fprintf    nFilePrintf
#define ungetc    nFileUngetC
#define lstat nFileStatus
#endif

#endif

