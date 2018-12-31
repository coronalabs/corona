//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_Archive.h"
#include "Rtt_FileSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#include "Rtt_Car.h"
 
// ----------------------------------------------------------------------------

using namespace Rtt;

static void
Usage( const char* arg0 )
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  %s {-a|--add} dest.car srcfile0 [srcfile1 ...]\n", arg0);
	fprintf(stderr, "  %s {-f|--filelist} filelist dest.car\n", arg0);
	fprintf(stderr, "  %s {-x|--extract} src.car destdir\n", arg0);
	fprintf(stderr, "  %s {-l|--list} src.car\n", arg0);
}

// ----------------------------------------------------------------------------

Rtt_EXPORT int
Rtt_CarMain( int argc, const char *argv[] )
{
	int result = 0;

	if ( argc < 3 )
	{
		Usage( argv[0] );
		result = -1;
	}
	else
	{
		if (0 == strcmp(argv[1], "-x") || 0 == strcmp(argv[1], "--extract"))
		{
			if ( argc < 4 )
			{
				Usage( argv[0] );
				result = -1;
			}
			else
			{
				Archive::Deserialize( argv[3], argv[2] );
			}
		}
		else if (0 == strcmp(argv[1], "-l") || 0 == strcmp(argv[1], "--list"))
		{
			if (argc < 3)
			{
				Usage(argv[0]);
				result = -1;
			}
			else
			{
				Archive::List(argv[2]);
			}
		}
		else if (0 == strcmp(argv[1], "-f") || 0 == strcmp(argv[1], "--filelist"))
		{
			if ( argc != 4 )
			{
				Usage( argv[0] );
				result = -1;
			}
			else
			{
				FILE *inFile = NULL;
				
				// If the filename is "-" read file list from stdin
				if ( 0 == strcmp( argv[2], "-" ) )
				{
					inFile = stdin;
				}
				else
				{
					if ((inFile = fopen(argv[2], "r")) == NULL)
					{
						fprintf(stderr, "%s: cannot open '%s' for reading\n", argv[0], argv[2]);
						
						return -1;
					}
				}
				
				int numSrcPaths = 0;
				int allocStride = 100;
				int numAlloced = allocStride;
				int spaceLeft = numAlloced;
				char buf[BUFSIZ];
				const char **srcPaths = (const char **) calloc(numAlloced, sizeof(char *));
				
				if (srcPaths == NULL)
				{
					fprintf(stderr, "%s: out of memory allocating %d filenames\n", argv[0], numAlloced);
					
					return -1;
				}
				
				while (fgets(buf, BUFSIZ, inFile) != NULL)
				{
					// zap the newline
					size_t len = strlen(buf);
					if (buf[len-1] == '\n')
					{
						buf[len-1] = '\0';
					}
					
					if (spaceLeft == 0)
					{
						numAlloced += allocStride;
						spaceLeft = allocStride;
						
						srcPaths = (const char **) realloc( srcPaths, (numAlloced * sizeof(char *)));
					}
					
					if (srcPaths == NULL)
					{
						fprintf(stderr, "%s: out of memory allocating %d filenames\n", argv[0], numAlloced);
						
						return -1;
					}
					
					if ((srcPaths[numSrcPaths++] = strdup(buf)) == NULL)
					{
						fprintf(stderr, "%s: out of memory after processing %d filenames\n", argv[0], numSrcPaths);
						
						return -1;
					}
					
					// printf("srcPath '%s', spaceLeft %d, numAlloced %d, numSrcPaths %d\n", srcPaths[numSrcPaths-1], spaceLeft,numAlloced,numSrcPaths);
										
					--spaceLeft;
				}
				
				fclose( inFile );
				
				Archive::Serialize( argv[3], numSrcPaths, srcPaths );
				
				// Free the memory we allocated
				for (int i = 0; i < numSrcPaths; i++)
				{
					free( const_cast<char *>(srcPaths[i]) );
				}
				free( srcPaths );
			}
		}
		else
		{
			int argOffset = (0 == strcmp(argv[1], "-a") || 0 == strcmp(argv[1], "--add")) ? 1 : 0;
			int numSrcPaths = argc - (2 + argOffset);
			const char **srcPaths = argv + (2 + argOffset);
			#if 0
				for (int i = 0; i < argc; i++ )
				{
					printf( "argv[%d] = %s\n", i, argv[i] );
				}
			#endif
			Archive::Serialize( argv[argOffset+1], numSrcPaths, srcPaths );
		}
	}

    return result;
}
