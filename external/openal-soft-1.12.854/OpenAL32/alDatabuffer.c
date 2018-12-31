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
#include <stdio.h>
#include <assert.h>
#include "alMain.h"
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#include "alError.h"
#include "alDatabuffer.h"
#include "alThunk.h"


DECL_VERIFIER(Databuffer, ALdatabuffer, databuffer)

/*
*    alGenDatabuffersEXT(ALsizei n, ALuint *puiBuffers)
*
*    Generates n AL Databuffers, and stores the Databuffers Names in the array pointed to by puiBuffers
*/
AL_API ALvoid AL_APIENTRY alGenDatabuffersEXT(ALsizei n,ALuint *puiBuffers)
{
    ALCcontext *Context;
    ALsizei i=0;

    Context = GetContextSuspended();
    if(!Context) return;

    /* Check that we are actually generation some Databuffers */
    if(n > 0)
    {
        ALCdevice *device = Context->Device;

        /* Check the pointer is valid (and points to enough memory to store
         * Databuffer Names) */
        if(!IsBadWritePtr((void*)puiBuffers, n * sizeof(ALuint)))
        {
            ALdatabuffer *end;
            ALdatabuffer **list = &device->DatabufferList;
            while(*list)
                list = &(*list)->next;

            /* Create all the new Databuffers */
            end = *list;
            while(i < n)
            {
                *list = calloc(1, sizeof(ALdatabuffer));
                if(!(*list))
                {
                    while(end->next)
                    {
                        ALdatabuffer *temp = end->next;
                        end->next = temp->next;

                        ALTHUNK_REMOVEENTRY(temp->databuffer);
                        device->DatabufferCount--;
                        free(temp);
                    }
                    alSetError(Context, AL_OUT_OF_MEMORY);
                    break;
                }

                puiBuffers[i] = (ALuint)ALTHUNK_ADDENTRY(*list);
                (*list)->databuffer = puiBuffers[i];
                (*list)->state = UNMAPPED;
                device->DatabufferCount++;
                i++;

                list = &(*list)->next;
            }
        }
        else
            alSetError(Context, AL_INVALID_VALUE);
    }

    ProcessContext(Context);
}

/*
*    alDatabeleteBuffersEXT(ALsizei n, ALuint *puiBuffers)
*
*    Deletes the n AL Databuffers pointed to by puiBuffers
*/
AL_API ALvoid AL_APIENTRY alDeleteDatabuffersEXT(ALsizei n, const ALuint *puiBuffers)
{
    ALCcontext *Context;
    ALdatabuffer *ALBuf;
    ALsizei i;
    ALboolean bFailed = AL_FALSE;

    Context = GetContextSuspended();
    if(!Context) return;

    /* Check we are actually Deleting some Databuffers */
    if(n >= 0)
    {
        ALCdevice *device = Context->Device;

        /* Check that all the databuffers are valid and can actually be
         * deleted */
        for(i = 0;i < n;i++)
        {
            if(!puiBuffers[i])
                continue;

            /* Check for valid Buffer ID */
            if((ALBuf=VerifyDatabuffer(device->DatabufferList, puiBuffers[i])) != NULL)
            {
                if(ALBuf->state != UNMAPPED)
                {
                    /* Databuffer still in use, cannot be deleted */
                    alSetError(Context, AL_INVALID_OPERATION);
                    bFailed = AL_TRUE;
                    break;
                }
            }
            else
            {
                /* Invalid Databuffer */
                alSetError(Context, AL_INVALID_NAME);
                bFailed = AL_TRUE;
                break;
            }
        }

        /* If all the Databuffers were valid (and unmapped), then we can
         * delete them */
        if(!bFailed)
        {
            for(i = 0;i < n;i++)
            {
                if((ALBuf=VerifyDatabuffer(device->DatabufferList, puiBuffers[i])) != NULL)
                {
                    ALdatabuffer **list = &device->DatabufferList;

                    while(*list && *list != ALBuf)
                        list = &(*list)->next;

                    if(*list)
                        *list = (*list)->next;

                    if(ALBuf == Context->SampleSource)
                        Context->SampleSource = NULL;
                    if(ALBuf == Context->SampleSink)
                        Context->SampleSink = NULL;

                    // Release the memory used to store audio data
                    free(ALBuf->data);

                    // Release buffer structure
                    ALTHUNK_REMOVEENTRY(puiBuffers[i]);
                    memset(ALBuf, 0, sizeof(ALdatabuffer));
                    device->DatabufferCount--;
                    free(ALBuf);
                }
            }
        }
    }
    else
        alSetError(Context, AL_INVALID_VALUE);

    ProcessContext(Context);
}

/*
*    alIsDatabufferEXT(ALuint uiBuffer)
*
*    Checks if ulBuffer is a valid Databuffer Name
*/
AL_API ALboolean AL_APIENTRY alIsDatabufferEXT(ALuint uiBuffer)
{
    ALCcontext *Context;
    ALboolean  result = AL_TRUE;
    ALCdevice *device;

    Context = GetContextSuspended();
    if(!Context) return AL_FALSE;

    device = Context->Device;
    if(uiBuffer)
        result = (VerifyDatabuffer(device->DatabufferList, uiBuffer) ?
                  AL_TRUE : AL_FALSE);

    ProcessContext(Context);

    return result;
}

/*
*    alDatabufferDataEXT(ALuint buffer,ALvoid *data,ALsizei size,ALenum usage)
*
*    Fill databuffer with data
*/
AL_API ALvoid AL_APIENTRY alDatabufferDataEXT(ALuint buffer,const ALvoid *data,ALsizeiptrEXT size,ALenum usage)
{
    ALCcontext *Context;
    ALdatabuffer *ALBuf;
    ALCdevice *Device;
    ALvoid *temp;

    Context = GetContextSuspended();
    if(!Context) return;

    Device = Context->Device;
    if((ALBuf=VerifyDatabuffer(Device->DatabufferList, buffer)) != NULL)
    {
        if(ALBuf->state == UNMAPPED)
        {
            if(usage == AL_STREAM_WRITE_EXT || usage == AL_STREAM_READ_EXT ||
               usage == AL_STREAM_COPY_EXT || usage == AL_STATIC_WRITE_EXT ||
               usage == AL_STATIC_READ_EXT || usage == AL_STATIC_COPY_EXT ||
               usage == AL_DYNAMIC_WRITE_EXT || usage == AL_DYNAMIC_READ_EXT ||
               usage == AL_DYNAMIC_COPY_EXT)
            {
                if(size >= 0)
                {
                    /* (Re)allocate data */
                    temp = realloc(ALBuf->data, size);
                    if(temp)
                    {
                        ALBuf->data = temp;
                        ALBuf->size = size;
                        ALBuf->usage = usage;
                        if(data)
                            memcpy(ALBuf->data, data, size);
                    }
                    else
                        alSetError(Context, AL_OUT_OF_MEMORY);
                }
                else
                    alSetError(Context, AL_INVALID_VALUE);
            }
            else
                alSetError(Context, AL_INVALID_ENUM);
        }
        else
            alSetError(Context, AL_INVALID_OPERATION);
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alDatabufferSubDataEXT(ALuint uiBuffer, ALintptrEXT start, ALsizeiptrEXT length, const ALvoid *data)
{
    ALCcontext    *pContext;
    ALdatabuffer  *pBuffer;
    ALCdevice     *Device;

    pContext = GetContextSuspended();
    if(!pContext) return;

    Device = pContext->Device;
    if((pBuffer=VerifyDatabuffer(Device->DatabufferList, uiBuffer)) != NULL)
    {
        if(start >= 0 && length >= 0 && start+length <= pBuffer->size)
        {
            if(pBuffer->state == UNMAPPED)
                memcpy(pBuffer->data+start, data, length);
            else
                alSetError(pContext, AL_INVALID_OPERATION);
        }
        else
            alSetError(pContext, AL_INVALID_VALUE);
    }
    else
        alSetError(pContext, AL_INVALID_NAME);

    ProcessContext(pContext);
}

AL_API ALvoid AL_APIENTRY alGetDatabufferSubDataEXT(ALuint uiBuffer, ALintptrEXT start, ALsizeiptrEXT length, ALvoid *data)
{
    ALCcontext    *pContext;
    ALdatabuffer  *pBuffer;
    ALCdevice     *Device;

    pContext = GetContextSuspended();
    if(!pContext) return;

    Device = pContext->Device;
    if((pBuffer=VerifyDatabuffer(Device->DatabufferList, uiBuffer)) != NULL)
    {
        if(start >= 0 && length >= 0 && start+length <= pBuffer->size)
        {
            if(pBuffer->state == UNMAPPED)
                memcpy(data, pBuffer->data+start, length);
            else
                alSetError(pContext, AL_INVALID_OPERATION);
        }
        else
            alSetError(pContext, AL_INVALID_VALUE);
    }
    else
        alSetError(pContext, AL_INVALID_NAME);

    ProcessContext(pContext);
}


AL_API ALvoid AL_APIENTRY alDatabufferfEXT(ALuint buffer, ALenum eParam, ALfloat flValue)
{
    ALCcontext    *pContext;
    ALCdevice     *Device;

    (void)flValue;

    pContext = GetContextSuspended();
    if(!pContext) return;

    Device = pContext->Device;
    if(VerifyDatabuffer(Device->DatabufferList, buffer) != NULL)
    {
        switch(eParam)
        {
        default:
            alSetError(pContext, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(pContext, AL_INVALID_NAME);

    ProcessContext(pContext);
}

AL_API ALvoid AL_APIENTRY alDatabufferfvEXT(ALuint buffer, ALenum eParam, const ALfloat* flValues)
{
    ALCcontext    *pContext;
    ALCdevice     *Device;

    (void)flValues;

    pContext = GetContextSuspended();
    if(!pContext) return;

    Device = pContext->Device;
    if(VerifyDatabuffer(Device->DatabufferList, buffer) != NULL)
    {
        switch(eParam)
        {
        default:
            alSetError(pContext, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(pContext, AL_INVALID_NAME);

    ProcessContext(pContext);
}


AL_API ALvoid AL_APIENTRY alDatabufferiEXT(ALuint buffer, ALenum eParam, ALint lValue)
{
    ALCcontext    *pContext;
    ALCdevice     *Device;

    (void)lValue;

    pContext = GetContextSuspended();
    if(!pContext) return;

    Device = pContext->Device;
    if(VerifyDatabuffer(Device->DatabufferList, buffer) != NULL)
    {
        switch(eParam)
        {
        default:
            alSetError(pContext, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(pContext, AL_INVALID_NAME);

    ProcessContext(pContext);
}

AL_API ALvoid AL_APIENTRY alDatabufferivEXT(ALuint buffer, ALenum eParam, const ALint* plValues)
{
    ALCcontext    *pContext;
    ALCdevice     *Device;

    (void)plValues;

    pContext = GetContextSuspended();
    if(!pContext) return;

    Device = pContext->Device;
    if(VerifyDatabuffer(Device->DatabufferList, buffer) != NULL)
    {
        switch(eParam)
        {
        default:
            alSetError(pContext, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(pContext, AL_INVALID_NAME);

    ProcessContext(pContext);
}


AL_API ALvoid AL_APIENTRY alGetDatabufferfEXT(ALuint buffer, ALenum eParam, ALfloat *pflValue)
{
    ALCcontext    *pContext;
    ALCdevice     *Device;

    pContext = GetContextSuspended();
    if(!pContext) return;

    if(pflValue)
    {
        Device = pContext->Device;
        if(VerifyDatabuffer(Device->DatabufferList, buffer) != NULL)
        {
            switch(eParam)
            {
            default:
                alSetError(pContext, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(pContext, AL_INVALID_NAME);
    }
    else
        alSetError(pContext, AL_INVALID_VALUE);

    ProcessContext(pContext);
}

AL_API ALvoid AL_APIENTRY alGetDatabufferfvEXT(ALuint buffer, ALenum eParam, ALfloat* pflValues)
{
    ALCcontext    *pContext;
    ALCdevice     *Device;

    pContext = GetContextSuspended();
    if(!pContext) return;

    if(pflValues)
    {
        Device = pContext->Device;
        if(VerifyDatabuffer(Device->DatabufferList, buffer) != NULL)
        {
            switch(eParam)
            {
            default:
                alSetError(pContext, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(pContext, AL_INVALID_NAME);
    }
    else
        alSetError(pContext, AL_INVALID_VALUE);

    ProcessContext(pContext);
}

AL_API ALvoid AL_APIENTRY alGetDatabufferiEXT(ALuint buffer, ALenum eParam, ALint *plValue)
{
    ALCcontext    *pContext;
    ALdatabuffer  *pBuffer;
    ALCdevice     *Device;

    pContext = GetContextSuspended();
    if(!pContext) return;

    if(plValue)
    {
        Device = pContext->Device;
        if((pBuffer=VerifyDatabuffer(Device->DatabufferList, buffer)) != NULL)
        {
            switch(eParam)
            {
            case AL_SIZE:
                *plValue = (ALint)pBuffer->size;
                break;

            default:
                alSetError(pContext, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(pContext, AL_INVALID_NAME);
    }
    else
        alSetError(pContext, AL_INVALID_VALUE);

    ProcessContext(pContext);
}

AL_API ALvoid AL_APIENTRY alGetDatabufferivEXT(ALuint buffer, ALenum eParam, ALint* plValues)
{
    ALCcontext    *pContext;
    ALCdevice     *Device;

    pContext = GetContextSuspended();
    if(!pContext) return;

    if(plValues)
    {
        Device = pContext->Device;
        if(VerifyDatabuffer(Device->DatabufferList, buffer) != NULL)
        {
            switch (eParam)
            {
            case AL_SIZE:
                alGetDatabufferiEXT(buffer, eParam, plValues);
                break;

            default:
                alSetError(pContext, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(pContext, AL_INVALID_NAME);
    }
    else
        alSetError(pContext, AL_INVALID_VALUE);

    ProcessContext(pContext);
}


AL_API ALvoid AL_APIENTRY alSelectDatabufferEXT(ALenum target, ALuint uiBuffer)
{
    ALCcontext    *pContext;
    ALdatabuffer  *pBuffer = NULL;
    ALCdevice     *Device;

    pContext = GetContextSuspended();
    if(!pContext) return;

    Device = pContext->Device;
    if(uiBuffer == 0 ||
       (pBuffer=VerifyDatabuffer(Device->DatabufferList, uiBuffer)) != NULL)
    {
        if(target == AL_SAMPLE_SOURCE_EXT)
            pContext->SampleSource = pBuffer;
        else if(target == AL_SAMPLE_SINK_EXT)
            pContext->SampleSink = pBuffer;
        else
            alSetError(pContext, AL_INVALID_VALUE);
    }
    else
        alSetError(pContext, AL_INVALID_NAME);

    ProcessContext(pContext);
}


AL_API ALvoid* AL_APIENTRY alMapDatabufferEXT(ALuint uiBuffer, ALintptrEXT start, ALsizeiptrEXT length, ALenum access)
{
    ALCcontext    *pContext;
    ALdatabuffer  *pBuffer;
    ALvoid        *ret = NULL;
    ALCdevice     *Device;

    pContext = GetContextSuspended();
    if(!pContext) return NULL;

    Device = pContext->Device;
    if((pBuffer=VerifyDatabuffer(Device->DatabufferList, uiBuffer)) != NULL)
    {
        if(start >= 0 && length >= 0 && start+length <= pBuffer->size)
        {
            if(access == AL_READ_ONLY_EXT || access == AL_WRITE_ONLY_EXT ||
               access == AL_READ_WRITE_EXT)
            {
                if(pBuffer->state == UNMAPPED)
                {
                    ret = pBuffer->data + start;
                    pBuffer->state = MAPPED;
                }
                else
                    alSetError(pContext, AL_INVALID_OPERATION);
            }
            else
                alSetError(pContext, AL_INVALID_ENUM);
        }
        else
            alSetError(pContext, AL_INVALID_VALUE);
    }
    else
        alSetError(pContext, AL_INVALID_NAME);

    ProcessContext(pContext);

    return ret;
}

AL_API ALvoid AL_APIENTRY alUnmapDatabufferEXT(ALuint uiBuffer)
{
    ALCcontext    *pContext;
    ALdatabuffer  *pBuffer;
    ALCdevice     *Device;

    pContext = GetContextSuspended();
    if(!pContext) return;

    Device = pContext->Device;
    if((pBuffer=VerifyDatabuffer(Device->DatabufferList, uiBuffer)) != NULL)
    {
        if(pBuffer->state == MAPPED)
            pBuffer->state = UNMAPPED;
        else
            alSetError(pContext, AL_INVALID_OPERATION);
    }
    else
        alSetError(pContext, AL_INVALID_NAME);

    ProcessContext(pContext);
}


/*
*    ReleaseALDatabuffers()
*
*    INTERNAL FN : Called by DLLMain on exit to destroy any buffers that still exist
*/
ALvoid ReleaseALDatabuffers(ALCdevice *device)
{
    while(device->DatabufferList)
    {
        ALdatabuffer *temp = device->DatabufferList;
        device->DatabufferList = temp->next;

        // Release buffer data
        free(temp->data);

        // Release Buffer structure
        ALTHUNK_REMOVEENTRY(temp->databuffer);
        memset(temp, 0, sizeof(ALdatabuffer));
        free(temp);
    }
    device->DatabufferCount = 0;
}
