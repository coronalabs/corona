/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
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
#include "AL/alc.h"
#include "AL/alext.h"
#include "alError.h"
#include "alSource.h"
#include "alState.h"
#include "alDatabuffer.h"

static const ALchar alVendor[] = "OpenAL Community";
static const ALchar alVersion[] = "1.1 ALSOFT "ALSOFT_VERSION;
static const ALchar alRenderer[] = "OpenAL Soft";

// Error Messages
static const ALchar alNoError[] = "No Error";
static const ALchar alErrInvalidName[] = "Invalid Name";
static const ALchar alErrInvalidEnum[] = "Invalid Enum";
static const ALchar alErrInvalidValue[] = "Invalid Value";
static const ALchar alErrInvalidOp[] = "Invalid Operation";
static const ALchar alErrOutOfMemory[] = "Out of Memory";

AL_API ALvoid AL_APIENTRY alEnable(ALenum capability)
{
    ALCcontext *Context;
    ALboolean  updateSources = AL_FALSE;

    Context = GetContextSuspended();
    if(!Context) return;

    switch(capability)
    {
        case AL_SOURCE_DISTANCE_MODEL:
            Context->SourceDistanceModel = AL_TRUE;
            updateSources = AL_TRUE;
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
    }

    if(updateSources)
    {
        ALsource *source = Context->SourceList;
        while(source)
        {
            source->NeedsUpdate = AL_TRUE;
            source = source->next;
        }
    }

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alDisable(ALenum capability)
{
    ALCcontext *Context;
    ALboolean  updateSources = AL_FALSE;

    Context = GetContextSuspended();
    if(!Context) return;

    switch(capability)
    {
        case AL_SOURCE_DISTANCE_MODEL:
            Context->SourceDistanceModel = AL_FALSE;
            updateSources = AL_TRUE;
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
    }

    if(updateSources)
    {
        ALsource *source = Context->SourceList;
        while(source)
        {
            source->NeedsUpdate = AL_TRUE;
            source = source->next;
        }
    }

    ProcessContext(Context);
}

AL_API ALboolean AL_APIENTRY alIsEnabled(ALenum capability)
{
    ALCcontext *Context;
    ALboolean value=AL_FALSE;

    Context = GetContextSuspended();
    if(!Context) return AL_FALSE;

    switch(capability)
    {
        case AL_SOURCE_DISTANCE_MODEL:
            value = Context->SourceDistanceModel;
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
    }

    ProcessContext(Context);

    return value;
}

AL_API ALboolean AL_APIENTRY alGetBoolean(ALenum pname)
{
    ALCcontext *Context;
    ALboolean value=AL_FALSE;

    Context = GetContextSuspended();
    if(!Context) return AL_FALSE;

    switch(pname)
    {
        case AL_DOPPLER_FACTOR:
            if(Context->DopplerFactor != 0.0f)
                value = AL_TRUE;
            break;

        case AL_DOPPLER_VELOCITY:
            if(Context->DopplerVelocity != 0.0f)
                value = AL_TRUE;
            break;

        case AL_DISTANCE_MODEL:
            if(Context->DistanceModel == AL_INVERSE_DISTANCE_CLAMPED)
                value = AL_TRUE;
            break;

        case AL_SPEED_OF_SOUND:
            if(Context->flSpeedOfSound != 0.0f)
                value = AL_TRUE;
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
    }

    ProcessContext(Context);

    return value;
}

AL_API ALdouble AL_APIENTRY alGetDouble(ALenum pname)
{
    ALCcontext *Context;
    ALdouble value = 0.0;

    Context = GetContextSuspended();
    if(!Context) return 0.0;

    switch(pname)
    {
        case AL_DOPPLER_FACTOR:
            value = (double)Context->DopplerFactor;
            break;

        case AL_DOPPLER_VELOCITY:
            value = (double)Context->DopplerVelocity;
            break;

        case AL_DISTANCE_MODEL:
            value = (double)Context->DistanceModel;
            break;

        case AL_SPEED_OF_SOUND:
            value = (double)Context->flSpeedOfSound;
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
    }

    ProcessContext(Context);

    return value;
}

AL_API ALfloat AL_APIENTRY alGetFloat(ALenum pname)
{
    ALCcontext *Context;
    ALfloat value = 0.0f;

    Context = GetContextSuspended();
    if(!Context) return 0.0f;

    switch(pname)
    {
        case AL_DOPPLER_FACTOR:
            value = Context->DopplerFactor;
            break;

        case AL_DOPPLER_VELOCITY:
            value = Context->DopplerVelocity;
            break;

        case AL_DISTANCE_MODEL:
            value = (float)Context->DistanceModel;
            break;

        case AL_SPEED_OF_SOUND:
            value = Context->flSpeedOfSound;
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
    }

    ProcessContext(Context);

    return value;
}

AL_API ALint AL_APIENTRY alGetInteger(ALenum pname)
{
    ALCcontext *Context;
    ALint value = 0;

    Context = GetContextSuspended();
    if(!Context) return 0;

    switch(pname)
    {
        case AL_DOPPLER_FACTOR:
            value = (ALint)Context->DopplerFactor;
            break;

        case AL_DOPPLER_VELOCITY:
            value = (ALint)Context->DopplerVelocity;
            break;

        case AL_DISTANCE_MODEL:
            value = (ALint)Context->DistanceModel;
            break;

        case AL_SPEED_OF_SOUND:
            value = (ALint)Context->flSpeedOfSound;
            break;

        case AL_SAMPLE_SOURCE_EXT:
            if(Context->SampleSource)
                value = (ALint)Context->SampleSource->databuffer;
            else
                value = 0;
            break;

        case AL_SAMPLE_SINK_EXT:
            if(Context->SampleSink)
                value = (ALint)Context->SampleSink->databuffer;
            else
                value = 0;
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
    }

    ProcessContext(Context);

    return value;
}

AL_API ALvoid AL_APIENTRY alGetBooleanv(ALenum pname,ALboolean *data)
{
    ALCcontext *Context;

    Context = GetContextSuspended();
    if(!Context) return;

    if(data)
    {
        switch(pname)
        {
            case AL_DOPPLER_FACTOR:
                *data = (ALboolean)((Context->DopplerFactor != 0.0f) ? AL_TRUE : AL_FALSE);
                break;

            case AL_DOPPLER_VELOCITY:
                *data = (ALboolean)((Context->DopplerVelocity != 0.0f) ? AL_TRUE : AL_FALSE);
                break;

            case AL_DISTANCE_MODEL:
                *data = (ALboolean)((Context->DistanceModel == AL_INVERSE_DISTANCE_CLAMPED) ? AL_TRUE : AL_FALSE);
                break;

            case AL_SPEED_OF_SOUND:
                *data = (ALboolean)((Context->flSpeedOfSound != 0.0f) ? AL_TRUE : AL_FALSE);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
        }
    }
    else
    {
        // data is a NULL pointer
        alSetError(Context, AL_INVALID_VALUE);
    }

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetDoublev(ALenum pname,ALdouble *data)
{
    ALCcontext *Context;

    Context = GetContextSuspended();
    if(!Context) return;

    if(data)
    {
        switch(pname)
        {
            case AL_DOPPLER_FACTOR:
                *data = (double)Context->DopplerFactor;
                break;

            case AL_DOPPLER_VELOCITY:
                *data = (double)Context->DopplerVelocity;
                break;

            case AL_DISTANCE_MODEL:
                *data = (double)Context->DistanceModel;
                break;

            case AL_SPEED_OF_SOUND:
                *data = (double)Context->flSpeedOfSound;
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
        }
    }
    else
    {
        // data is a NULL pointer
        alSetError(Context, AL_INVALID_VALUE);
    }

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetFloatv(ALenum pname,ALfloat *data)
{
    ALCcontext *Context;

    Context = GetContextSuspended();
    if(!Context) return;

    if(data)
    {
        switch(pname)
        {
            case AL_DOPPLER_FACTOR:
                *data = Context->DopplerFactor;
                break;

            case AL_DOPPLER_VELOCITY:
                *data = Context->DopplerVelocity;
                break;

            case AL_DISTANCE_MODEL:
                *data = (float)Context->DistanceModel;
                break;

            case AL_SPEED_OF_SOUND:
                *data = Context->flSpeedOfSound;
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
        }
    }
    else
    {
        // data is a NULL pointer
        alSetError(Context, AL_INVALID_VALUE);
    }

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetIntegerv(ALenum pname,ALint *data)
{
    ALCcontext *Context;

    Context = GetContextSuspended();
    if(!Context) return;

    if(data)
    {
        switch(pname)
        {
            case AL_DOPPLER_FACTOR:
                *data = (ALint)Context->DopplerFactor;
                break;

            case AL_DOPPLER_VELOCITY:
                *data = (ALint)Context->DopplerVelocity;
                break;

            case AL_DISTANCE_MODEL:
                *data = (ALint)Context->DistanceModel;
                break;

            case AL_SPEED_OF_SOUND:
                *data = (ALint)Context->flSpeedOfSound;
                break;

            case AL_SAMPLE_SOURCE_EXT:
                if(Context->SampleSource)
                    *data = (ALint)Context->SampleSource->databuffer;
                else
                    *data = 0;
                break;

            case AL_SAMPLE_SINK_EXT:
                if(Context->SampleSink)
                    *data = (ALint)Context->SampleSink->databuffer;
                else
                    *data = 0;
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
        }
    }
    else
    {
        // data is a NULL pointer
        alSetError(Context, AL_INVALID_VALUE);
    }

    ProcessContext(Context);
}

AL_API const ALchar* AL_APIENTRY alGetString(ALenum pname)
{
    const ALchar *value;
    ALCcontext *pContext;

    pContext = GetContextSuspended();
    if(!pContext) return NULL;

    switch(pname)
    {
        case AL_VENDOR:
            value=alVendor;
            break;

        case AL_VERSION:
            value=alVersion;
            break;

        case AL_RENDERER:
            value=alRenderer;
            break;

        case AL_EXTENSIONS:
            value=pContext->ExtensionList;//alExtensions;
            break;

        case AL_NO_ERROR:
            value=alNoError;
            break;

        case AL_INVALID_NAME:
            value=alErrInvalidName;
            break;

        case AL_INVALID_ENUM:
            value=alErrInvalidEnum;
            break;

        case AL_INVALID_VALUE:
            value=alErrInvalidValue;
            break;

        case AL_INVALID_OPERATION:
            value=alErrInvalidOp;
            break;

        case AL_OUT_OF_MEMORY:
            value=alErrOutOfMemory;
            break;

        default:
            value=NULL;
            alSetError(pContext, AL_INVALID_ENUM);
            break;
    }

    ProcessContext(pContext);

    return value;
}

AL_API ALvoid AL_APIENTRY alDopplerFactor(ALfloat value)
{
    ALCcontext *Context;
    ALboolean updateSources = AL_FALSE;

    Context = GetContextSuspended();
    if(!Context) return;

    if(value >= 0.0f)
    {
        Context->DopplerFactor = value;
        updateSources = AL_TRUE;
    }
    else
        alSetError(Context, AL_INVALID_VALUE);

    // Force updating the sources for these parameters, since even head-
    // relative sources are affected
    if(updateSources)
    {
        ALsource *source = Context->SourceList;
        while(source)
        {
            source->NeedsUpdate = AL_TRUE;
            source = source->next;
        }
    }

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alDopplerVelocity(ALfloat value)
{
    ALCcontext *Context;
    ALboolean updateSources = AL_FALSE;

    Context = GetContextSuspended();
    if(!Context) return;

    if(value > 0.0f)
    {
        Context->DopplerVelocity=value;
        updateSources = AL_TRUE;
    }
    else
        alSetError(Context, AL_INVALID_VALUE);

    if(updateSources)
    {
        ALsource *source = Context->SourceList;
        while(source)
        {
            source->NeedsUpdate = AL_TRUE;
            source = source->next;
        }
    }

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alSpeedOfSound(ALfloat flSpeedOfSound)
{
    ALCcontext *pContext;
    ALboolean updateSources = AL_FALSE;

    pContext = GetContextSuspended();
    if(!pContext) return;

    if(flSpeedOfSound > 0.0f)
    {
        pContext->flSpeedOfSound = flSpeedOfSound;
        updateSources = AL_TRUE;
    }
    else
        alSetError(pContext, AL_INVALID_VALUE);

    if(updateSources)
    {
        ALsource *source = pContext->SourceList;
        while(source)
        {
            source->NeedsUpdate = AL_TRUE;
            source = source->next;
        }
    }

    ProcessContext(pContext);
}

AL_API ALvoid AL_APIENTRY alDistanceModel(ALenum value)
{
    ALCcontext *Context;
    ALboolean updateSources = AL_FALSE;

    Context = GetContextSuspended();
    if(!Context) return;

    switch(value)
    {
        case AL_NONE:
        case AL_INVERSE_DISTANCE:
        case AL_INVERSE_DISTANCE_CLAMPED:
        case AL_LINEAR_DISTANCE:
        case AL_LINEAR_DISTANCE_CLAMPED:
        case AL_EXPONENT_DISTANCE:
        case AL_EXPONENT_DISTANCE_CLAMPED:
            Context->DistanceModel = value;
            updateSources = !Context->SourceDistanceModel;
            break;

        default:
            alSetError(Context, AL_INVALID_VALUE);
            break;
    }

    if(updateSources)
    {
        ALsource *source = Context->SourceList;
        while(source)
        {
            source->NeedsUpdate = AL_TRUE;
            source = source->next;
        }
    }

    ProcessContext(Context);
}
