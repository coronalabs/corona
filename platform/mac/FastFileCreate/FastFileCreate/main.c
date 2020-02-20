//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#import "NativeFileCreate.h"

int main (int argc, const char * argv[])
{

    if(4 != argc)
    {
        printf("FastFileTouch requires 3 parameters: (1) an input file containing a list of files to create, (2) the target directory to write files to, (3) The file name sanitation mode, where 0 is do nothing, and 1 makes things lowercase and replaces spaces with underscores.\n");
        return 1;
    }

    // fprintf(stderr, "Input file is %s, target dir is %s", argv[1], argv[2]);
    
	bool ret_val = NativeFileCreate(argv[1], argv[2], (int)strtol(argv[3], NULL, 10));
	if(false == ret_val)
	{
		return 2;
	}
    
    return 0;
}

