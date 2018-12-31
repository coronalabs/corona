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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alMain.h"
#include "AL/al.h"
#include "AL/alc.h"
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#include <portaudio.h>

static void *pa_handle;
#define MAKE_FUNC(x) static typeof(x) * p##x
MAKE_FUNC(Pa_Initialize);
MAKE_FUNC(Pa_Terminate);
MAKE_FUNC(Pa_GetErrorText);
MAKE_FUNC(Pa_StartStream);
MAKE_FUNC(Pa_StopStream);
MAKE_FUNC(Pa_OpenStream);
MAKE_FUNC(Pa_CloseStream);
MAKE_FUNC(Pa_GetDefaultOutputDevice);
MAKE_FUNC(Pa_GetStreamInfo);
#undef MAKE_FUNC


static const ALCchar pa_device[] = "PortAudio Software";
static const ALCchar pa_capture[] = "PortAudio Capture";
static volatile ALuint load_count;


void *pa_load(void)
{
    if(load_count == 0)
    {
        PaError err;

#ifdef _WIN32
        pa_handle = LoadLibrary("portaudio.dll");
#define LOAD_FUNC(x) do { \
    p##x = (typeof(p##x))GetProcAddress(pa_handle, #x); \
    if(!(p##x)) { \
        AL_PRINT("Could not load %s from portaudio.dll\n", #x); \
        FreeLibrary(pa_handle); \
        pa_handle = NULL; \
        return NULL; \
    } \
} while(0)

#elif defined(HAVE_DLFCN_H)

    const char *str;
#if defined(__APPLE__) && defined(__MACH__)
# define PALIB "libportaudio.2.dylib"
#else
# define PALIB "libportaudio.so.2"
#endif
        pa_handle = dlopen(PALIB, RTLD_NOW);
        dlerror();

#define LOAD_FUNC(f) do { \
    p##f = (typeof(f)*)dlsym(pa_handle, #f); \
    if((str=dlerror()) != NULL) \
    { \
        dlclose(pa_handle); \
        pa_handle = NULL; \
        AL_PRINT("Could not load %s from "PALIB": %s\n", #f, str); \
        return NULL; \
    } \
} while(0)

#else
        str = NULL;
        pa_handle = (void*)0xDEADBEEF;
#define LOAD_FUNC(f) p##f = f
#endif

        if(!pa_handle)
            return NULL;

LOAD_FUNC(Pa_Initialize);
LOAD_FUNC(Pa_Terminate);
LOAD_FUNC(Pa_GetErrorText);
LOAD_FUNC(Pa_StartStream);
LOAD_FUNC(Pa_StopStream);
LOAD_FUNC(Pa_OpenStream);
LOAD_FUNC(Pa_CloseStream);
LOAD_FUNC(Pa_GetDefaultOutputDevice);
LOAD_FUNC(Pa_GetStreamInfo);

#undef LOAD_FUNC

        if((err=pPa_Initialize()) != paNoError)
        {
            AL_PRINT("Pa_Initialize() returned an error: %s\n", pPa_GetErrorText(err));
#ifdef _WIN32
            FreeLibrary(pa_handle);
#elif defined(HAVE_DLFCN_H)
            dlclose(pa_handle);
#endif
            pa_handle = NULL;
            return NULL;
        }
    }
    ++load_count;

    return pa_handle;
}

void pa_unload(void)
{
    if(load_count == 0 || --load_count > 0)
        return;

    pPa_Terminate();
#ifdef _WIN32
    FreeLibrary(pa_handle);
#elif defined(HAVE_DLFCN_H)
    dlclose(pa_handle);
#endif
    pa_handle = NULL;
}


typedef struct {
    PaStream *stream;
    ALuint update_size;

    RingBuffer *ring;
} pa_data;


static int pa_callback(const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo,
                       const PaStreamCallbackFlags statusFlags, void *userData)
{
    ALCdevice *device = (ALCdevice*)userData;

    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;

    aluMixData(device, outputBuffer, framesPerBuffer);
    return 0;
}

static int pa_capture_cb(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo,
                         const PaStreamCallbackFlags statusFlags, void *userData)
{
    ALCdevice *device = (ALCdevice*)userData;
    pa_data *data = (pa_data*)device->ExtraData;

    (void)outputBuffer;
    (void)timeInfo;
    (void)statusFlags;

    WriteRingBuffer(data->ring, inputBuffer, framesPerBuffer);
    return 0;
}


static ALCboolean pa_open_playback(ALCdevice *device, const ALCchar *deviceName)
{
    const PaStreamInfo *streamInfo;
    PaStreamParameters outParams;
    pa_data *data;
    PaError err;

    if(!deviceName)
        deviceName = pa_device;
    else if(strcmp(deviceName, pa_device) != 0)
        return ALC_FALSE;

    if(!pa_load())
        return ALC_FALSE;

    data = (pa_data*)calloc(1, sizeof(pa_data));
    data->update_size = device->UpdateSize;

    device->ExtraData = data;

    outParams.device = GetConfigValueInt("port", "device", -1);
    if(outParams.device < 0)
        outParams.device = pPa_GetDefaultOutputDevice();
    outParams.suggestedLatency = (device->UpdateSize*device->NumUpdates) /
                                 (float)device->Frequency;
    outParams.hostApiSpecificStreamInfo = NULL;

    switch(aluBytesFromFormat(device->Format))
    {
        case 1:
            outParams.sampleFormat = paUInt8;
            break;
        case 2:
            outParams.sampleFormat = paInt16;
            break;
        case 4:
            outParams.sampleFormat = paFloat32;
            break;
        default:
            AL_PRINT("Unknown format: 0x%x\n", device->Format);
            device->ExtraData = NULL;
            free(data);
            pa_unload();
            return ALC_FALSE;
    }
    outParams.channelCount = aluChannelsFromFormat(device->Format);

    SetDefaultChannelOrder(device);

    err = pPa_OpenStream(&data->stream, NULL, &outParams, device->Frequency,
                         device->UpdateSize, paNoFlag, pa_callback, device);
    if(err != paNoError)
    {
        AL_PRINT("Pa_OpenStream() returned an error: %s\n", pPa_GetErrorText(err));
        device->ExtraData = NULL;
        free(data);
        pa_unload();
        return ALC_FALSE;
    }
    streamInfo = pPa_GetStreamInfo(data->stream);

    device->szDeviceName = strdup(deviceName);
    device->Frequency = streamInfo->sampleRate;

    return ALC_TRUE;
}

static void pa_close_playback(ALCdevice *device)
{
    pa_data *data = (pa_data*)device->ExtraData;
    PaError err;

    err = pPa_CloseStream(data->stream);
    if(err != paNoError)
        AL_PRINT("Error closing stream: %s\n", pPa_GetErrorText(err));

    free(data);
    device->ExtraData = NULL;

    pa_unload();
}

static ALCboolean pa_reset_playback(ALCdevice *device)
{
    pa_data *data = (pa_data*)device->ExtraData;
    const PaStreamInfo *streamInfo;
    PaError err;

    streamInfo = pPa_GetStreamInfo(data->stream);
    device->Frequency = streamInfo->sampleRate;
    device->UpdateSize = data->update_size;

    err = pPa_StartStream(data->stream);
    if(err != paNoError)
    {
        AL_PRINT("Pa_StartStream() returned an error: %s\n", pPa_GetErrorText(err));
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void pa_stop_playback(ALCdevice *device)
{
    pa_data *data = (pa_data*)device->ExtraData;
    PaError err;

    err = pPa_StopStream(data->stream);
    if(err != paNoError)
        AL_PRINT("Error stopping stream: %s\n", pPa_GetErrorText(err));
}


static ALCboolean pa_open_capture(ALCdevice *device, const ALCchar *deviceName)
{
    PaStreamParameters inParams;
    ALuint frame_size;
    pa_data *data;
    PaError err;

    if(!deviceName)
        deviceName = pa_capture;
    else if(strcmp(deviceName, pa_capture) != 0)
        return ALC_FALSE;

    if(!pa_load())
        return ALC_FALSE;

    data = (pa_data*)calloc(1, sizeof(pa_data));
    if(data == NULL)
    {
        alcSetError(device, ALC_OUT_OF_MEMORY);
        return ALC_FALSE;
    }

    frame_size = aluChannelsFromFormat(device->Format) *
                 aluBytesFromFormat(device->Format);
    data->ring = CreateRingBuffer(frame_size, device->UpdateSize*device->NumUpdates);
    if(data->ring == NULL)
    {
        alcSetError(device, ALC_OUT_OF_MEMORY);
        goto error;
    }

    inParams.device = GetConfigValueInt("port", "capture", -1);
    if(inParams.device < 0)
        inParams.device = pPa_GetDefaultOutputDevice();
    inParams.suggestedLatency = 0.0f;
    inParams.hostApiSpecificStreamInfo = NULL;

    switch(aluBytesFromFormat(device->Format))
    {
        case 1:
            inParams.sampleFormat = paUInt8;
            break;
        case 2:
            inParams.sampleFormat = paInt16;
            break;
        case 4:
            inParams.sampleFormat = paFloat32;
            break;
        default:
            AL_PRINT("Unknown format: 0x%x\n", device->Format);
            goto error;
    }
    inParams.channelCount = aluChannelsFromFormat(device->Format);

    err = pPa_OpenStream(&data->stream, &inParams, NULL, device->Frequency,
                         paFramesPerBufferUnspecified, paNoFlag, pa_capture_cb, device);
    if(err != paNoError)
    {
        AL_PRINT("Pa_OpenStream() returned an error: %s\n", pPa_GetErrorText(err));
        goto error;
    }

    device->szDeviceName = strdup(deviceName);

    device->ExtraData = data;
    return ALC_TRUE;

error:
    DestroyRingBuffer(data->ring);
    free(data);
    pa_unload();
    return ALC_FALSE;
}

static void pa_close_capture(ALCdevice *device)
{
    pa_data *data = (pa_data*)device->ExtraData;
    PaError err;

    err = pPa_CloseStream(data->stream);
    if(err != paNoError)
        AL_PRINT("Error closing stream: %s\n", pPa_GetErrorText(err));

    free(data);
    device->ExtraData = NULL;

    pa_unload();
}

static void pa_start_capture(ALCdevice *device)
{
    pa_data *data = device->ExtraData;
    PaError err;

    err = pPa_StartStream(data->stream);
    if(err != paNoError)
        AL_PRINT("Error starting stream: %s\n", pPa_GetErrorText(err));
}

static void pa_stop_capture(ALCdevice *device)
{
    pa_data *data = (pa_data*)device->ExtraData;
    PaError err;

    err = pPa_StopStream(data->stream);
    if(err != paNoError)
        AL_PRINT("Error stopping stream: %s\n", pPa_GetErrorText(err));
}

static void pa_capture_samples(ALCdevice *device, ALCvoid *buffer, ALCuint samples)
{
    pa_data *data = device->ExtraData;
    if(samples <= (ALCuint)RingBufferSize(data->ring))
        ReadRingBuffer(data->ring, buffer, samples);
    else
        alcSetError(device, ALC_INVALID_VALUE);
}

static ALCuint pa_available_samples(ALCdevice *device)
{
    pa_data *data = device->ExtraData;
    return RingBufferSize(data->ring);
}


static const BackendFuncs pa_funcs = {
    pa_open_playback,
    pa_close_playback,
    pa_reset_playback,
    pa_stop_playback,
    pa_open_capture,
    pa_close_capture,
    pa_start_capture,
    pa_stop_capture,
    pa_capture_samples,
    pa_available_samples
};

void alc_pa_init(BackendFuncs *func_list)
{
    *func_list = pa_funcs;
}

void alc_pa_deinit(void)
{
}

void alc_pa_probe(int type)
{
    if(!pa_load()) return;

    if(type == DEVICE_PROBE)
        AppendDeviceList(pa_device);
    else if(type == ALL_DEVICE_PROBE)
        AppendAllDeviceList(pa_device);
    else if(type == CAPTURE_DEVICE_PROBE)
        AppendCaptureDeviceList(pa_capture);

    pa_unload();
}
