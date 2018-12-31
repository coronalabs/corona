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

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include "alMain.h"
#include "AL/al.h"
#include "AL/alc.h"

#include <sys/audioio.h>


static const ALCchar solaris_device[] = "Solaris Software";

typedef struct {
    int fd;
    volatile int killNow;
    ALvoid *thread;

    ALubyte *mix_data;
    int data_size;
} solaris_data;


static ALuint SolarisProc(ALvoid *ptr)
{
    ALCdevice *pDevice = (ALCdevice*)ptr;
    solaris_data *data = (solaris_data*)pDevice->ExtraData;
    int remaining = 0;
    ALint frameSize;
    int wrote;

    EnableRTPrio(RTPrioLevel);

    frameSize = aluChannelsFromFormat(pDevice->Format) *
                aluBytesFromFormat(pDevice->Format);

    while(!data->killNow && pDevice->Connected)
    {
        ALint len = data->data_size;
        ALubyte *WritePtr = data->mix_data;

        aluMixData(pDevice, WritePtr, len/frameSize);
        while(len > 0 && !data->killNow)
        {
            wrote = write(data->fd, WritePtr, len);
            if(wrote < 0)
            {
                if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                {
                    AL_PRINT("write failed: %s\n", strerror(errno));
                    aluHandleDisconnect(pDevice);
                    break;
                }

                Sleep(1);
                continue;
            }

            len -= wrote;
            WritePtr += wrote;
        }
    }

    return 0;
}


static ALCboolean solaris_open_playback(ALCdevice *device, const ALCchar *deviceName)
{
    char driver[64];
    solaris_data *data;

    strncpy(driver, GetConfigValue("solaris", "device", "/dev/audio"), sizeof(driver)-1);
    driver[sizeof(driver)-1] = 0;
    if(!deviceName)
        deviceName = solaris_device;
    else if(strcmp(deviceName, solaris_device) != 0)
        return ALC_FALSE;

    data = (solaris_data*)calloc(1, sizeof(solaris_data));
    data->killNow = 0;

    data->fd = open(driver, O_WRONLY);
    if(data->fd == -1)
    {
        free(data);
        if(errno != ENOENT)
            AL_PRINT("Could not open %s: %s\n", driver, strerror(errno));
        return ALC_FALSE;
    }

    device->szDeviceName = strdup(deviceName);
    device->ExtraData = data;
    return ALC_TRUE;
}

static void solaris_close_playback(ALCdevice *device)
{
    solaris_data *data = (solaris_data*)device->ExtraData;

    close(data->fd);
    free(data);
    device->ExtraData = NULL;
}

static ALCboolean solaris_reset_playback(ALCdevice *device)
{
    solaris_data *data = (solaris_data*)device->ExtraData;
    audio_info_t info;
    ALuint frameSize;
    int numChannels;

    AUDIO_INITINFO(&info);

    switch(aluBytesFromFormat(device->Format))
    {
        case 1:
            info.play.precision = 8;
            info.play.encoding = AUDIO_ENCODING_LINEAR8;
            break;
        case 4:
            switch(numChannels)
            {
                case 1: device->Format = AL_FORMAT_MONO16; break;
                case 2: device->Format = AL_FORMAT_STEREO16; break;
                case 4: device->Format = AL_FORMAT_QUAD16; break;
                case 6: device->Format = AL_FORMAT_51CHN16; break;
                case 7: device->Format = AL_FORMAT_61CHN16; break;
                case 8: device->Format = AL_FORMAT_71CHN16; break;
            }
            /* fall-through */
        case 2:
            info.play.precision = 16;
            info.play.encoding = AUDIO_ENCODING_LINEAR;
            break;
        default:
            AL_PRINT("Unknown format: 0x%x\n", device->Format);
            return ALC_FALSE;
    }

    numChannels = aluChannelsFromFormat(device->Format);
    info.play.sample_rate = device->Frequency;
    info.play.channels = numChannels;

    frameSize = numChannels * aluBytesFromFormat(device->Format);
    info.play.buffer_size = device->UpdateSize*device->NumUpdates * frameSize;

    if(ioctl(data->fd, AUDIO_SETINFO, &info) < 0)
    {
        AL_PRINT("ioctl failed: %s\n", strerror(errno));
        return ALC_FALSE;
    }

    if(aluChannelsFromFormat(device->Format) != info.play.channels)
    {
        AL_PRINT("Could not set %d channels, got %d instead\n", aluChannelsFromFormat(device->Format), info.play.channels);
        return ALC_FALSE;
    }

    if(!((info.play.precision == 8 && aluBytesFromFormat(device->Format) == 1) ||
         (info.play.precision == 16 && aluBytesFromFormat(device->Format) == 2)))
    {
        AL_PRINT("Could not set %d-bit output, got %d\n", aluBytesFromFormat(device->Format)*8, info.play.precision);
        return ALC_FALSE;
    }

    device->Frequency = info.play.sample_rate;
    device->UpdateSize = (info.play.buffer_size/device->NumUpdates) + 1;

    data->data_size = device->UpdateSize * frameSize;
    data->mix_data = calloc(1, data->data_size);

    SetDefaultChannelOrder(device);

    data->thread = StartThread(SolarisProc, device);
    if(data->thread == NULL)
    {
        free(data->mix_data);
        data->mix_data = NULL;
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void solaris_stop_playback(ALCdevice *device)
{
    solaris_data *data = (solaris_data*)device->ExtraData;

    if(!data->thread)
        return;

    data->killNow = 1;
    StopThread(data->thread);
    data->thread = NULL;

    data->killNow = 0;

    free(data->mix_data);
    data->mix_data = NULL;
}


static ALCboolean solaris_open_capture(ALCdevice *device, const ALCchar *deviceName, ALCuint frequency, ALCenum format, ALCsizei SampleSize)
{
    (void)device;
    (void)deviceName;
    (void)frequency;
    (void)format;
    (void)SampleSize;
    return ALC_FALSE;
}

static void solaris_close_capture(ALCdevice *device)
{
    (void)device;
}

static void solaris_start_capture(ALCdevice *pDevice)
{
    (void)pDevice;
}

static void solaris_stop_capture(ALCdevice *pDevice)
{
    (void)pDevice;
}

static void solaris_capture_samples(ALCdevice *pDevice, ALCvoid *pBuffer, ALCuint lSamples)
{
    (void)pDevice;
    (void)pBuffer;
    (void)lSamples;
}

static ALCuint solaris_available_samples(ALCdevice *pDevice)
{
    (void)pDevice;
    return 0;
}


BackendFuncs solaris_funcs = {
    solaris_open_playback,
    solaris_close_playback,
    solaris_reset_playback,
    solaris_stop_playback,
    solaris_open_capture,
    solaris_close_capture,
    solaris_start_capture,
    solaris_stop_capture,
    solaris_capture_samples,
    solaris_available_samples
};

void alc_solaris_init(BackendFuncs *func_list)
{
    *func_list = solaris_funcs;
}

void alc_solaris_deinit(void)
{
}

void alc_solaris_probe(int type)
{
#ifdef HAVE_STAT
    struct stat buf;
    if(stat(GetConfigValue("solaris", "device", "/dev/audio"), &buf) != 0)
        return;
#endif

    if(type == DEVICE_PROBE)
        AppendDeviceList(solaris_device);
    else if(type == ALL_DEVICE_PROBE)
        AppendAllDeviceList(solaris_device);
}
