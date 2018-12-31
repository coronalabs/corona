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

