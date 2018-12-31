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
#include <memory.h>
#include "alMain.h"
#include "AL/al.h"
#include "AL/alc.h"


typedef struct {
    FILE *f;
    long DataStart;

    ALvoid *buffer;
    ALuint size;

    volatile int killNow;
    ALvoid *thread;
} wave_data;


static const ALCchar waveDevice[] = "Wave File Writer";

static const ALubyte SUBTYPE_PCM[] = {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa,
    0x00, 0x38, 0x9b, 0x71
};
static const ALubyte SUBTYPE_FLOAT[] = {
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa,
    0x00, 0x38, 0x9b, 0x71
};

static const ALuint channel_masks[] = {
    0, /* invalid */
    0x4, /* Mono */
    0x1 | 0x2, /* Stereo */
    0, /* 3 channel */
    0x1 | 0x2 | 0x10 | 0x20, /* Quad */
    0, /* 5 channel */
    0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20, /* 5.1 */
    0x1 | 0x2 | 0x4 | 0x8 | 0x100 | 0x200 | 0x400, /* 6.1 */
    0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20 | 0x200 | 0x400, /* 7.1 */
};

static ALuint WaveProc(ALvoid *ptr)
{
    ALCdevice *pDevice = (ALCdevice*)ptr;
    wave_data *data = (wave_data*)pDevice->ExtraData;
    ALuint frameSize;
    ALuint now, last;
    size_t fs;
    ALuint avail;
    union {
        short s;
        char b[sizeof(short)];
    } uSB;

    uSB.s = 1;
    frameSize = aluBytesFromFormat(pDevice->Format) *
                aluChannelsFromFormat(pDevice->Format);

    last = timeGetTime()<<8;
    while(!data->killNow && pDevice->Connected)
    {
        now = timeGetTime()<<8;

        avail = (ALuint64)(now-last) * pDevice->Frequency / (1000<<8);
        if(avail < pDevice->UpdateSize)
        {
            Sleep(1);
            continue;
        }

        while(avail >= pDevice->UpdateSize)
        {
            aluMixData(pDevice, data->buffer, pDevice->UpdateSize);

            if(uSB.b[0] != 1)
            {
                ALubyte *bytes = data->buffer;
                ALuint i;

                if(aluBytesFromFormat(pDevice->Format) == 1)
                {
                    for(i = 0;i < data->size;i++)
                        fputc(bytes[i], data->f);
                }
                else if(aluBytesFromFormat(pDevice->Format) == 2)
                {
                    for(i = 0;i < data->size;i++)
                        fputc(bytes[i^1], data->f);
                }
                else if(aluBytesFromFormat(pDevice->Format) == 4)
                {
                    for(i = 0;i < data->size;i++)
                        fputc(bytes[i^3], data->f);
                }
            }
            else
                fs = fwrite(data->buffer, frameSize, pDevice->UpdateSize,
                            data->f);
            if(ferror(data->f))
            {
                AL_PRINT("Error writing to file\n");
                aluHandleDisconnect(pDevice);
                break;
            }

            avail -= pDevice->UpdateSize;
            last += (ALuint64)pDevice->UpdateSize * (1000<<8) / pDevice->Frequency;
        }
    }

    return 0;
}

static ALCboolean wave_open_playback(ALCdevice *device, const ALCchar *deviceName)
{
    wave_data *data;
    const char *fname;

    fname = GetConfigValue("wave", "file", "");
    if(!fname[0])
        return ALC_FALSE;

    if(!deviceName)
        deviceName = waveDevice;
    else if(strcmp(deviceName, waveDevice) != 0)
        return ALC_FALSE;

    data = (wave_data*)calloc(1, sizeof(wave_data));

    data->f = fopen(fname, "wb");
    if(!data->f)
    {
        free(data);
        AL_PRINT("Could not open file '%s': %s\n", fname, strerror(errno));
        return ALC_FALSE;
    }

    device->szDeviceName = strdup(deviceName);
    device->ExtraData = data;
    return ALC_TRUE;
}

static void wave_close_playback(ALCdevice *device)
{
    wave_data *data = (wave_data*)device->ExtraData;

    fclose(data->f);
    free(data);
    device->ExtraData = NULL;
}

static ALCboolean wave_reset_playback(ALCdevice *device)
{
    wave_data *data = (wave_data*)device->ExtraData;
    ALuint channels, bits, i;
    size_t val;

    fseek(data->f, 0, SEEK_SET);
    clearerr(data->f);

    bits = aluBytesFromFormat(device->Format) * 8;
    channels = aluChannelsFromFormat(device->Format);

    /* 7.1 max */
    if(channels > 8)
    {
        if(bits == 8)
            device->Format = AL_FORMAT_71CHN8;
        else if(bits == 16)
            device->Format = AL_FORMAT_71CHN16;
        else
        {
            device->Format = AL_FORMAT_71CHN32;
            bits = 32;
        }
        channels = 8;
    }

    fprintf(data->f, "RIFF");
    fputc(0xFF, data->f); // 'RIFF' header len; filled in at close
    fputc(0xFF, data->f);
    fputc(0xFF, data->f);
    fputc(0xFF, data->f);

    fprintf(data->f, "WAVE");

    fprintf(data->f, "fmt ");
    fputc(40, data->f); // 'fmt ' header len; 40 bytes for EXTENSIBLE
    fputc(0, data->f);
    fputc(0, data->f);
    fputc(0, data->f);
    // 16-bit val, format type id (extensible: 0xFFFE)
    fputc(0xFE, data->f);
    fputc(0xFF, data->f);
    // 16-bit val, channel count
    fputc(channels&0xff, data->f);
    fputc((channels>>8)&0xff, data->f);
    // 32-bit val, frequency
    fputc(device->Frequency&0xff, data->f);
    fputc((device->Frequency>>8)&0xff, data->f);
    fputc((device->Frequency>>16)&0xff, data->f);
    fputc((device->Frequency>>24)&0xff, data->f);
    // 32-bit val, bytes per second
    i = device->Frequency * channels * bits / 8;
    fputc(i&0xff, data->f);
    fputc((i>>8)&0xff, data->f);
    fputc((i>>16)&0xff, data->f);
    fputc((i>>24)&0xff, data->f);
    // 16-bit val, frame size
    i = channels * bits / 8;
    fputc(i&0xff, data->f);
    fputc((i>>8)&0xff, data->f);
    // 16-bit val, bits per sample
    fputc(bits&0xff, data->f);
    fputc((bits>>8)&0xff, data->f);
    // 16-bit val, extra byte count
    fputc(22, data->f);
    fputc(0, data->f);
    // 16-bit val, valid bits per sample
    fputc(bits&0xff, data->f);
    fputc((bits>>8)&0xff, data->f);
    // 32-bit val, channel mask
    i = channel_masks[channels];
    fputc(i&0xff, data->f);
    fputc((i>>8)&0xff, data->f);
    fputc((i>>16)&0xff, data->f);
    fputc((i>>24)&0xff, data->f);
    // 16 byte GUID, sub-type format
    val = fwrite(((bits==32) ? SUBTYPE_FLOAT : SUBTYPE_PCM), 1, 16, data->f);

    fprintf(data->f, "data");
    fputc(0xFF, data->f); // 'data' header len; filled in at close
    fputc(0xFF, data->f);
    fputc(0xFF, data->f);
    fputc(0xFF, data->f);

    if(ferror(data->f))
    {
        AL_PRINT("Error writing header: %s\n", strerror(errno));
        return ALC_FALSE;
    }

    data->DataStart = ftell(data->f);

    data->size = device->UpdateSize * channels * bits / 8;
    data->buffer = malloc(data->size);
    if(!data->buffer)
    {
        AL_PRINT("buffer malloc failed\n");
        return ALC_FALSE;
    }

    SetDefaultWFXChannelOrder(device);

    data->thread = StartThread(WaveProc, device);
    if(data->thread == NULL)
    {
        free(data->buffer);
        data->buffer = NULL;
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void wave_stop_playback(ALCdevice *device)
{
    wave_data *data = (wave_data*)device->ExtraData;
    ALuint dataLen;
    long size;

    if(!data->thread)
        return;

    data->killNow = 1;
    StopThread(data->thread);
    data->thread = NULL;

    data->killNow = 0;

    free(data->buffer);
    data->buffer = NULL;

    size = ftell(data->f);
    if(size > 0)
    {
        dataLen = size - data->DataStart;
        if(fseek(data->f, data->DataStart-4, SEEK_SET) == 0)
        {
            fputc(dataLen&0xff, data->f); // 'data' header len
            fputc((dataLen>>8)&0xff, data->f);
            fputc((dataLen>>16)&0xff, data->f);
            fputc((dataLen>>24)&0xff, data->f);
        }
        if(fseek(data->f, 4, SEEK_SET) == 0)
        {
            size -= 8;
            fputc(size&0xff, data->f); // 'WAVE' header len
            fputc((size>>8)&0xff, data->f);
            fputc((size>>16)&0xff, data->f);
            fputc((size>>24)&0xff, data->f);
        }
    }
}


static ALCboolean wave_open_capture(ALCdevice *pDevice, const ALCchar *deviceName)
{
    (void)pDevice;
    (void)deviceName;
    return ALC_FALSE;
}


BackendFuncs wave_funcs = {
    wave_open_playback,
    wave_close_playback,
    wave_reset_playback,
    wave_stop_playback,
    wave_open_capture,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

void alc_wave_init(BackendFuncs *func_list)
{
    *func_list = wave_funcs;
}

void alc_wave_deinit(void)
{
}

void alc_wave_probe(int type)
{
    if(!ConfigValueExists("wave", "file"))
        return;

    if(type == DEVICE_PROBE)
        AppendDeviceList(waveDevice);
    else if(type == ALL_DEVICE_PROBE)
        AppendAllDeviceList(waveDevice);
}
