/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2007 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "config.h"

#include <stdlib.h>

#include "alMain.h"
#include "alThunk.h"

typedef struct {
    ALvoid *ptr;
    ALboolean InUse;
} ThunkEntry;

static ThunkEntry *g_ThunkArray;
static ALuint      g_ThunkArraySize;

static CRITICAL_SECTION g_ThunkLock;

void alThunkInit(void)
{
    InitializeCriticalSection(&g_ThunkLock);
    g_ThunkArraySize = 1;
    g_ThunkArray = calloc(1, g_ThunkArraySize * sizeof(ThunkEntry));
}

void alThunkExit(void)
{
    free(g_ThunkArray);
    g_ThunkArray = NULL;
    g_ThunkArraySize = 0;
    DeleteCriticalSection(&g_ThunkLock);
}

ALuint alThunkAddEntry(ALvoid *ptr)
{
    ALuint index;

    EnterCriticalSection(&g_ThunkLock);

    for(index = 0;index < g_ThunkArraySize;index++)
    {
        if(g_ThunkArray[index].InUse == AL_FALSE)
            break;
    }

    if(index == g_ThunkArraySize)
    {
        ThunkEntry *NewList;

        NewList = realloc(g_ThunkArray, g_ThunkArraySize*2 * sizeof(ThunkEntry));
        if(!NewList)
        {
            LeaveCriticalSection(&g_ThunkLock);
            AL_PRINT("Realloc failed to increase to %u enties!\n", g_ThunkArraySize*2);
            return 0;
        }
        memset(&NewList[g_ThunkArraySize], 0, g_ThunkArraySize*sizeof(ThunkEntry));
        g_ThunkArraySize *= 2;
        g_ThunkArray = NewList;
    }

    g_ThunkArray[index].ptr = ptr;
    g_ThunkArray[index].InUse = AL_TRUE;

    LeaveCriticalSection(&g_ThunkLock);

    return index+1;
}

void alThunkRemoveEntry(ALuint index)
{
    EnterCriticalSection(&g_ThunkLock);

    if(index > 0 && index <= g_ThunkArraySize)
        g_ThunkArray[index-1].InUse = AL_FALSE;

    LeaveCriticalSection(&g_ThunkLock);
}

ALvoid *alThunkLookupEntry(ALuint index)
{
    ALvoid *ptr = NULL;

    EnterCriticalSection(&g_ThunkLock);

    if(index > 0 && index <= g_ThunkArraySize)
        ptr = g_ThunkArray[index-1].ptr;

    LeaveCriticalSection(&g_ThunkLock);

    return ptr;
}
