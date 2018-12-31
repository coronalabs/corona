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

#include <sys/soundcard.h>

/*
 * The OSS documentation talks about SOUND_MIXER_READ, but the header
 * only contains MIXER_READ. Play safe. Same for WRITE.
 */
#ifndef SOUND_MIXER_READ
#define SOUND_MIXER_READ MIXER_READ
#endif
#ifndef SOUND_MIXER_WRITE
#define SOUND_MIXER_WRITE MIXER_WRITE
#endif

static const ALCchar oss_device[] = "OSS Software";
static const ALCchar oss_device_capture[] = "OSS Capture";

typedef struct {
    int fd;
    volatile int killNow;
    ALvoid *thread;

    ALubyte *mix_data;
    int data_size;

    RingBuffer *ring;
    int doCapture;
} oss_data;


static int log2i(ALCuint x)
{
    int y = 0;
    while (x > 1)
    {
        x >>= 1;
        y++;
    }
    return y;
}


static ALuint OSSProc(ALvoid *ptr)
{
    ALCdevice *pDevice = (ALCdevice*)ptr;
    oss_data *data = (oss_data*)pDevice->ExtraData;
    ALint frameSize;
    ssize_t wrote;

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

static ALuint OSSCaptureProc(ALvoid *ptr)
{
    ALCdevice *pDevice = (ALCdevice*)ptr;
    oss_data *data = (oss_data*)pDevice->ExtraData;
    int frameSize;
    int amt;

    EnableRTPrio(RTPrioLevel);

    frameSize  = aluBytesFromFormat(pDevice->Format);
    frameSize *= aluChannelsFromFormat(pDevice->Format);

    while(!data->killNow)
    {
        amt = read(data->fd, data->mix_data, data->data_size);
        if(amt < 0)
        {
            AL_PRINT("read failed: %s\n", strerror(errno));
            aluHandleDisconnect(pDevice);
            break;
        }
        if(amt == 0)
        {
            Sleep(1);
            continue;
        }
        if(data->doCapture)
            WriteRingBuffer(data->ring, data->mix_data, amt/frameSize);
    }

    return 0;
}

static ALCboolean oss_open_playback(ALCdevice *device, const ALCchar *deviceName)
{
    char driver[64];
    oss_data *data;

    strncpy(driver, GetConfigValue("oss", "device", "/dev/dsp"), sizeof(driver)-1);
    driver[sizeof(driver)-1] = 0;
    if(!deviceName)
        deviceName = oss_device;
    else if(strcmp(deviceName, oss_device) != 0)
        return ALC_FALSE;

    data = (oss_data*)calloc(1, sizeof(oss_data));
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

static void oss_close_playback(ALCdevice *device)
{
    oss_data *data = (oss_data*)device->ExtraData;

    close(data->fd);
    free(data);
    device->ExtraData = NULL;
}

static ALCboolean oss_reset_playback(ALCdevice *device)
{
    oss_data *data = (oss_data*)device->ExtraData;
    int numFragmentsLogSize;
    int log2FragmentSize;
    unsigned int periods;
    audio_buf_info info;
    ALuint frameSize;
    int numChannels;
    int ossFormat;
    int ossSpeed;
    char *err;
    int i;

    switch(aluBytesFromFormat(device->Format))
    {
        case 1:
            ossFormat = AFMT_U8;
            break;
        case 4:
            switch(aluChannelsFromFormat(device->Format))
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
            ossFormat = AFMT_S16_NE;
            break;
        default:
            AL_PRINT("Unknown format: 0x%x\n", device->Format);
            return ALC_FALSE;
    }

    periods = device->NumUpdates;
    numChannels = aluChannelsFromFormat(device->Format);
    frameSize = numChannels * aluBytesFromFormat(device->Format);

    ossSpeed = device->Frequency;
    log2FragmentSize = log2i(device->UpdateSize * frameSize);

    /* according to the OSS spec, 16 bytes are the minimum */
    if (log2FragmentSize < 4)
        log2FragmentSize = 4;
    /* Subtract one period since the temp mixing buffer counts as one. Still
     * need at least two on the card, though. */
    if(periods > 2) periods--;
    numFragmentsLogSize = (periods << 16) | log2FragmentSize;

#define ok(func, str) (i=(func),((i<0)?(err=(str)),0:1))
    /* Don't fail if SETFRAGMENT fails. We can handle just about anything
     * that's reported back via GETOSPACE */
    ioctl(data->fd, SNDCTL_DSP_SETFRAGMENT, &numFragmentsLogSize);
    if (!(ok(ioctl(data->fd, SNDCTL_DSP_SETFMT, &ossFormat), "set format") &&
          ok(ioctl(data->fd, SNDCTL_DSP_CHANNELS, &numChannels), "set channels") &&
          ok(ioctl(data->fd, SNDCTL_DSP_SPEED, &ossSpeed), "set speed") &&
          ok(ioctl(data->fd, SNDCTL_DSP_GETOSPACE, &info), "get space")))
    {
        AL_PRINT("%s failed: %s\n", err, strerror(errno));
        return ALC_FALSE;
    }
#undef ok

    if((int)aluChannelsFromFormat(device->Format) != numChannels)
    {
        AL_PRINT("Could not set %d channels, got %d instead\n", aluChannelsFromFormat(device->Format), numChannels);
        return ALC_FALSE;
    }

    if(!((ossFormat == AFMT_U8 && aluBytesFromFormat(device->Format) == 1) ||
         (ossFormat == AFMT_S16_NE && aluBytesFromFormat(device->Format) == 2)))
    {
        AL_PRINT("Could not set %d-bit output, got format %#x\n", aluBytesFromFormat(device->Format)*8, ossFormat);
        return ALC_FALSE;
    }

    device->Frequency = ossSpeed;
    device->UpdateSize = info.fragsize / frameSize;
    device->NumUpdates = info.fragments + 1;

    data->data_size = device->UpdateSize * frameSize;
    data->mix_data = calloc(1, data->data_size);

    SetDefaultChannelOrder(device);

    data->thread = StartThread(OSSProc, device);
    if(data->thread == NULL)
    {
        free(data->mix_data);
        data->mix_data = NULL;
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void oss_stop_playback(ALCdevice *device)
{
    oss_data *data = (oss_data*)device->ExtraData;

    if(!data->thread)
        return;

    data->killNow = 1;
    StopThread(data->thread);
    data->thread = NULL;

    data->killNow = 0;
    if(ioctl(data->fd, SNDCTL_DSP_RESET) != 0)
        AL_PRINT("Error resetting device: %s\n", strerror(errno));

    free(data->mix_data);
    data->mix_data = NULL;
}


static ALCboolean oss_open_capture(ALCdevice *device, const ALCchar *deviceName)
{
    int numFragmentsLogSize;
    int log2FragmentSize;
    unsigned int periods;
    audio_buf_info info;
    ALuint frameSize;
    int numChannels;
    char driver[64];
    oss_data *data;
    int ossFormat;
    int ossSpeed;
    char *err;
    int i;

    strncpy(driver, GetConfigValue("oss", "capture", "/dev/dsp"), sizeof(driver)-1);
    driver[sizeof(driver)-1] = 0;
    if(!deviceName)
        deviceName = oss_device_capture;
    else if(strcmp(deviceName, oss_device_capture) != 0)
        return ALC_FALSE;

    data = (oss_data*)calloc(1, sizeof(oss_data));
    data->killNow = 0;

    data->fd = open(driver, O_RDONLY);
    if(data->fd == -1)
    {
        free(data);
        if(errno != ENOENT)
            AL_PRINT("Could not open %s: %s\n", driver, strerror(errno));
        return ALC_FALSE;
    }

    switch(aluBytesFromFormat(device->Format))
    {
        case 1:
            ossFormat = AFMT_U8;
            break;
        case 2:
            ossFormat = AFMT_S16_NE;
            break;
        default:
            AL_PRINT("Unknown format: 0x%x\n", device->Format);
            close(data->fd);
            free(data);
            return ALC_FALSE;
    }

    periods = 4;
    numChannels = aluChannelsFromFormat(device->Format);
    frameSize = numChannels * aluBytesFromFormat(device->Format);
    ossSpeed = device->Frequency;
    log2FragmentSize = log2i(device->UpdateSize * device->NumUpdates *
                             frameSize / periods);

    /* according to the OSS spec, 16 bytes are the minimum */
    if (log2FragmentSize < 4)
        log2FragmentSize = 4;
    numFragmentsLogSize = (periods << 16) | log2FragmentSize;

#define ok(func, str) (i=(func),((i<0)?(err=(str)),0:1))
    if (!(ok(ioctl(data->fd, SNDCTL_DSP_SETFRAGMENT, &numFragmentsLogSize), "set fragment") &&
          ok(ioctl(data->fd, SNDCTL_DSP_SETFMT, &ossFormat), "set format") &&
          ok(ioctl(data->fd, SNDCTL_DSP_CHANNELS, &numChannels), "set channels") &&
          ok(ioctl(data->fd, SNDCTL_DSP_SPEED, &ossSpeed), "set speed") &&
          ok(ioctl(data->fd, SNDCTL_DSP_GETISPACE, &info), "get space")))
    {
        AL_PRINT("%s failed: %s\n", err, strerror(errno));
        close(data->fd);
        free(data);
        return ALC_FALSE;
    }
#undef ok

    if((int)aluChannelsFromFormat(device->Format) != numChannels)
    {
        AL_PRINT("Could not set %d channels, got %d instead\n", aluChannelsFromFormat(device->Format), numChannels);
        close(data->fd);
        free(data);
        return ALC_FALSE;
    }

    if(!((ossFormat == AFMT_U8 && aluBytesFromFormat(device->Format) == 1) ||
         (ossFormat == AFMT_S16_NE && aluBytesFromFormat(device->Format) == 2)))
    {
        AL_PRINT("Could not set %d-bit input, got format %#x\n", aluBytesFromFormat(device->Format)*8, ossFormat);
        close(data->fd);
        free(data);
        return ALC_FALSE;
    }

    data->ring = CreateRingBuffer(frameSize, device->UpdateSize * device->NumUpdates);
    if(!data->ring)
    {
        AL_PRINT("ring buffer create failed\n");
        close(data->fd);
        free(data);
        return ALC_FALSE;
    }

    data->data_size = info.fragsize;
    data->mix_data = calloc(1, data->data_size);

    device->ExtraData = data;
    data->thread = StartThread(OSSCaptureProc, device);
    if(data->thread == NULL)
    {
        device->ExtraData = NULL;
        free(data->mix_data);
        free(data);
        return ALC_FALSE;
    }

    device->szDeviceName = strdup(deviceName);
    return ALC_TRUE;
}

static void oss_close_capture(ALCdevice *device)
{
    oss_data *data = (oss_data*)device->ExtraData;
    data->killNow = 1;
    StopThread(data->thread);

    close(data->fd);

    DestroyRingBuffer(data->ring);

    free(data->mix_data);
    free(data);
    device->ExtraData = NULL;
}

static void oss_start_capture(ALCdevice *pDevice)
{
    oss_data *data = (oss_data*)pDevice->ExtraData;
    data->doCapture = 1;
}

static void oss_stop_capture(ALCdevice *pDevice)
{
    oss_data *data = (oss_data*)pDevice->ExtraData;
    data->doCapture = 0;
}

static void oss_capture_samples(ALCdevice *pDevice, ALCvoid *pBuffer, ALCuint lSamples)
{
    oss_data *data = (oss_data*)pDevice->ExtraData;
    if(lSamples <= (ALCuint)RingBufferSize(data->ring))
        ReadRingBuffer(data->ring, pBuffer, lSamples);
    else
        alcSetError(pDevice, ALC_INVALID_VALUE);
}

static ALCuint oss_available_samples(ALCdevice *pDevice)
{
    oss_data *data = (oss_data*)pDevice->ExtraData;
    return RingBufferSize(data->ring);
}


BackendFuncs oss_funcs = {
    oss_open_playback,
    oss_close_playback,
    oss_reset_playback,
    oss_stop_playback,
    oss_open_capture,
    oss_close_capture,
    oss_start_capture,
    oss_stop_capture,
    oss_capture_samples,
    oss_available_samples
};

void alc_oss_init(BackendFuncs *func_list)
{
    *func_list = oss_funcs;
}

void alc_oss_deinit(void)
{
}

void alc_oss_probe(int type)
{
    if(type == DEVICE_PROBE)
    {
#ifdef HAVE_STAT
        struct stat buf;
        if(stat(GetConfigValue("oss", "device", "/dev/dsp"), &buf) == 0)
#endif
            AppendDeviceList(oss_device);
    }
    else if(type == ALL_DEVICE_PROBE)
    {
#ifdef HAVE_STAT
        struct stat buf;
        if(stat(GetConfigValue("oss", "device", "/dev/dsp"), &buf) == 0)
#endif
            AppendAllDeviceList(oss_device);
    }
    else if(type == CAPTURE_DEVICE_PROBE)
    {
#ifdef HAVE_STAT
        struct stat buf;
        if(stat(GetConfigValue("oss", "capture", "/dev/dsp"), &buf) == 0)
#endif
            AppendCaptureDeviceList(oss_device_capture);
    }
}
