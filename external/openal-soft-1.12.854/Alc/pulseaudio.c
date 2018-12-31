/**
 * OpenAL cross platform audio library
 * Copyright (C) 2009 by Konstantinos Natsakis <konstantinos.natsakis@gmail.com>
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

#include "alMain.h"
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#include <pulse/pulseaudio.h>

#if PA_API_VERSION == 11
#define PA_STREAM_ADJUST_LATENCY 0x2000U
static inline int PA_STREAM_IS_GOOD(pa_stream_state_t x)
{
    return (x == PA_STREAM_CREATING || x == PA_STREAM_READY);
}
static inline int PA_CONTEXT_IS_GOOD(pa_context_state_t x)
{
    return (x == PA_CONTEXT_CONNECTING || x == PA_CONTEXT_AUTHORIZING ||
            x == PA_CONTEXT_SETTING_NAME || x == PA_CONTEXT_READY);
}
#define PA_STREAM_IS_GOOD PA_STREAM_IS_GOOD
#define PA_CONTEXT_IS_GOOD PA_CONTEXT_IS_GOOD
#elif PA_API_VERSION != 12
#error Invalid PulseAudio API version
#endif

#ifndef PA_CHECK_VERSION
#define PA_CHECK_VERSION(major,minor,micro)                             \
    ((PA_MAJOR > (major)) ||                                            \
     (PA_MAJOR == (major) && PA_MINOR > (minor)) ||                     \
     (PA_MAJOR == (major) && PA_MINOR == (minor) && PA_MICRO >= (micro)))
#endif

static void *pa_handle;
#define MAKE_FUNC(x) static typeof(x) * p##x
MAKE_FUNC(pa_context_unref);
MAKE_FUNC(pa_sample_spec_valid);
MAKE_FUNC(pa_stream_drop);
MAKE_FUNC(pa_strerror);
MAKE_FUNC(pa_context_get_state);
MAKE_FUNC(pa_stream_get_state);
MAKE_FUNC(pa_threaded_mainloop_signal);
MAKE_FUNC(pa_stream_peek);
MAKE_FUNC(pa_threaded_mainloop_wait);
MAKE_FUNC(pa_threaded_mainloop_unlock);
MAKE_FUNC(pa_threaded_mainloop_in_thread);
MAKE_FUNC(pa_context_new);
MAKE_FUNC(pa_threaded_mainloop_stop);
MAKE_FUNC(pa_context_disconnect);
MAKE_FUNC(pa_threaded_mainloop_start);
MAKE_FUNC(pa_threaded_mainloop_get_api);
MAKE_FUNC(pa_context_set_state_callback);
MAKE_FUNC(pa_stream_write);
MAKE_FUNC(pa_xfree);
MAKE_FUNC(pa_stream_connect_record);
MAKE_FUNC(pa_stream_connect_playback);
MAKE_FUNC(pa_stream_readable_size);
MAKE_FUNC(pa_stream_cork);
MAKE_FUNC(pa_path_get_filename);
MAKE_FUNC(pa_get_binary_name);
MAKE_FUNC(pa_threaded_mainloop_free);
MAKE_FUNC(pa_context_errno);
MAKE_FUNC(pa_xmalloc);
MAKE_FUNC(pa_stream_unref);
MAKE_FUNC(pa_threaded_mainloop_accept);
MAKE_FUNC(pa_stream_set_write_callback);
MAKE_FUNC(pa_threaded_mainloop_new);
MAKE_FUNC(pa_context_connect);
MAKE_FUNC(pa_stream_set_buffer_attr);
MAKE_FUNC(pa_stream_get_buffer_attr);
MAKE_FUNC(pa_stream_get_sample_spec);
MAKE_FUNC(pa_stream_set_read_callback);
MAKE_FUNC(pa_stream_set_state_callback);
MAKE_FUNC(pa_stream_new);
MAKE_FUNC(pa_stream_disconnect);
MAKE_FUNC(pa_threaded_mainloop_lock);
MAKE_FUNC(pa_channel_map_init_auto);
MAKE_FUNC(pa_channel_map_parse);
MAKE_FUNC(pa_channel_map_snprint);
MAKE_FUNC(pa_channel_map_equal);
MAKE_FUNC(pa_context_get_server_info);
MAKE_FUNC(pa_context_get_sink_info_by_name);
MAKE_FUNC(pa_operation_get_state);
MAKE_FUNC(pa_operation_unref);
#if PA_CHECK_VERSION(0,9,15)
MAKE_FUNC(pa_channel_map_superset);
MAKE_FUNC(pa_stream_set_buffer_attr_callback);
#endif
#if PA_CHECK_VERSION(0,9,16)
MAKE_FUNC(pa_stream_begin_write);
#endif
#undef MAKE_FUNC

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct {
    ALCuint samples;
    ALCuint frame_size;

    RingBuffer *ring;

    pa_buffer_attr attr;
    pa_sample_spec spec;

    char path_name[PATH_MAX];
    const char *context_name;
    const char *stream_name;

    pa_threaded_mainloop *loop;

    pa_stream *stream;
    pa_context *context;
} pulse_data;

static const ALCchar pulse_device[] = "PulseAudio Software";
static const ALCchar pulse_capture_device[] = "PulseAudio Capture";
static pa_context_flags_t pulse_ctx_flags;
static volatile ALuint load_count;


void *pulse_load(void) //{{{
{
    if(load_count == 0)
    {
#ifdef _WIN32
        pa_handle = LoadLibrary("libpulse-0.dll");
#define LOAD_FUNC(x) do { \
    p##x = (typeof(p##x))GetProcAddress(pa_handle, #x); \
    if(!(p##x)) { \
        AL_PRINT("Could not load %s from libpulse-0.dll\n", #x); \
        FreeLibrary(pa_handle); \
        pa_handle = NULL; \
        return NULL; \
    } \
} while(0)
#define LOAD_OPTIONAL_FUNC(x) do { \
    p##x = (typeof(p##x))GetProcAddress(pa_handle, #x); \
} while(0)

#elif defined (HAVE_DLFCN_H)

        const char *err;
#if defined(__APPLE__) && defined(__MACH__)
        pa_handle = dlopen("libpulse.0.dylib", RTLD_NOW);
#else
        pa_handle = dlopen("libpulse.so.0", RTLD_NOW);
#endif
        dlerror();

#define LOAD_FUNC(x) do { \
    p##x = dlsym(pa_handle, #x); \
    if((err=dlerror()) != NULL) { \
        AL_PRINT("Could not load %s from libpulse: %s\n", #x, err); \
        dlclose(pa_handle); \
        pa_handle = NULL; \
        return NULL; \
    } \
} while(0)
#define LOAD_OPTIONAL_FUNC(x) do { \
    p##x = dlsym(pa_handle, #x); \
    if((err=dlerror()) != NULL) { \
        p##x = NULL; \
    } \
} while(0)

#else

        pa_handle = (void*)0xDEADBEEF;
#define LOAD_FUNC(x) p##x = (x)
#define LOAD_OPTIONAL_FUNC(x) p##x = (x)

#endif
        if(!pa_handle)
            return NULL;

LOAD_FUNC(pa_context_unref);
LOAD_FUNC(pa_sample_spec_valid);
LOAD_FUNC(pa_stream_drop);
LOAD_FUNC(pa_strerror);
LOAD_FUNC(pa_context_get_state);
LOAD_FUNC(pa_stream_get_state);
LOAD_FUNC(pa_threaded_mainloop_signal);
LOAD_FUNC(pa_stream_peek);
LOAD_FUNC(pa_threaded_mainloop_wait);
LOAD_FUNC(pa_threaded_mainloop_unlock);
LOAD_FUNC(pa_threaded_mainloop_in_thread);
LOAD_FUNC(pa_context_new);
LOAD_FUNC(pa_threaded_mainloop_stop);
LOAD_FUNC(pa_context_disconnect);
LOAD_FUNC(pa_threaded_mainloop_start);
LOAD_FUNC(pa_threaded_mainloop_get_api);
LOAD_FUNC(pa_context_set_state_callback);
LOAD_FUNC(pa_stream_write);
LOAD_FUNC(pa_xfree);
LOAD_FUNC(pa_stream_connect_record);
LOAD_FUNC(pa_stream_connect_playback);
LOAD_FUNC(pa_stream_readable_size);
LOAD_FUNC(pa_stream_cork);
LOAD_FUNC(pa_path_get_filename);
LOAD_FUNC(pa_get_binary_name);
LOAD_FUNC(pa_threaded_mainloop_free);
LOAD_FUNC(pa_context_errno);
LOAD_FUNC(pa_xmalloc);
LOAD_FUNC(pa_stream_unref);
LOAD_FUNC(pa_threaded_mainloop_accept);
LOAD_FUNC(pa_stream_set_write_callback);
LOAD_FUNC(pa_threaded_mainloop_new);
LOAD_FUNC(pa_context_connect);
LOAD_FUNC(pa_stream_set_buffer_attr);
LOAD_FUNC(pa_stream_get_buffer_attr);
LOAD_FUNC(pa_stream_get_sample_spec);
LOAD_FUNC(pa_stream_set_read_callback);
LOAD_FUNC(pa_stream_set_state_callback);
LOAD_FUNC(pa_stream_new);
LOAD_FUNC(pa_stream_disconnect);
LOAD_FUNC(pa_threaded_mainloop_lock);
LOAD_FUNC(pa_channel_map_init_auto);
LOAD_FUNC(pa_channel_map_parse);
LOAD_FUNC(pa_channel_map_snprint);
LOAD_FUNC(pa_channel_map_equal);
LOAD_FUNC(pa_context_get_server_info);
LOAD_FUNC(pa_context_get_sink_info_by_name);
LOAD_FUNC(pa_operation_get_state);
LOAD_FUNC(pa_operation_unref);
#if PA_CHECK_VERSION(0,9,15)
LOAD_OPTIONAL_FUNC(pa_channel_map_superset);
LOAD_OPTIONAL_FUNC(pa_stream_set_buffer_attr_callback);
#endif
#if PA_CHECK_VERSION(0,9,16)
LOAD_OPTIONAL_FUNC(pa_stream_begin_write);
#endif

#undef LOAD_OPTIONAL_FUNC
#undef LOAD_FUNC
    }
    ++load_count;

    return pa_handle;
} //}}}

void pulse_unload(void) //{{{
{
    if(load_count == 0 || --load_count > 0)
        return;

#ifdef _WIN32
    FreeLibrary(pa_handle);
#elif defined (HAVE_DLFCN_H)
    dlclose(pa_handle);
#endif
    pa_handle = NULL;
} //}}}


// PulseAudio Event Callbacks //{{{
static void context_state_callback(pa_context *context, void *pdata) //{{{
{
    ALCdevice *Device = pdata;
    pulse_data *data = Device->ExtraData;
    pa_context_state_t state;

    state = ppa_context_get_state(context);
    if(state == PA_CONTEXT_READY || !PA_CONTEXT_IS_GOOD(state))
        ppa_threaded_mainloop_signal(data->loop, 0);
}//}}}

static void stream_state_callback(pa_stream *stream, void *pdata) //{{{
{
    ALCdevice *Device = pdata;
    pulse_data *data = Device->ExtraData;
    pa_stream_state_t state;

    state = ppa_stream_get_state(stream);
    if(state == PA_STREAM_READY || !PA_STREAM_IS_GOOD(state))
        ppa_threaded_mainloop_signal(data->loop, 0);
}//}}}

static void stream_buffer_attr_callback(pa_stream *stream, void *pdata) //{{{
{
    ALCdevice *Device = pdata;
    pulse_data *data = Device->ExtraData;

    SuspendContext(NULL);

    data->attr = *(ppa_stream_get_buffer_attr(stream));
    Device->UpdateSize = 20 * Device->Frequency / 1000;
    Device->NumUpdates = data->attr.tlength/data->frame_size / Device->UpdateSize;
    if(Device->NumUpdates == 0)
        Device->NumUpdates = 1;

    ProcessContext(NULL);
}//}}}

static void context_state_callback2(pa_context *context, void *pdata) //{{{
{
    ALCdevice *Device = pdata;
    pulse_data *data = Device->ExtraData;

    if(ppa_context_get_state(context) == PA_CONTEXT_FAILED)
    {
        AL_PRINT("Received context failure!\n");
        aluHandleDisconnect(Device);
    }
    ppa_threaded_mainloop_signal(data->loop, 0);
}//}}}

static void stream_state_callback2(pa_stream *stream, void *pdata) //{{{
{
    ALCdevice *Device = pdata;
    pulse_data *data = Device->ExtraData;

    if(ppa_stream_get_state(stream) == PA_STREAM_FAILED)
    {
        AL_PRINT("Received stream failure!\n");
        aluHandleDisconnect(Device);
    }
    ppa_threaded_mainloop_signal(data->loop, 0);
}//}}}

static void stream_success_callback(pa_stream *stream, int success, void *pdata) //{{{
{
    ALCdevice *Device = pdata;
    pulse_data *data = Device->ExtraData;
    (void)stream;
    (void)success;

    ppa_threaded_mainloop_signal(data->loop, 0);
}//}}}

static void server_info_callback(pa_context *context, const pa_server_info *info, void *pdata) //{{{
{
    struct {
        pa_threaded_mainloop *loop;
        char *name;
    } *data = pdata;
    (void)context;

    data->name = strdup(info->default_sink_name);
    ppa_threaded_mainloop_signal(data->loop, 0);
}//}}}

static void sink_info_callback(pa_context *context, const pa_sink_info *info, int eol, void *pdata) //{{{
{
    ALCdevice *device = pdata;
    pulse_data *data = device->ExtraData;
    char chanmap_str[256] = "";
    const struct {
        const char *str;
        ALenum format;
    } chanmaps[] = {
        { "front-left,front-right,front-center,lfe,rear-left,rear-right,side-left,side-right",
          AL_FORMAT_71CHN32 },
        { "front-left,front-right,front-center,lfe,rear-center,side-left,side-right",
          AL_FORMAT_61CHN32 },
        { "front-left,front-right,front-center,lfe,rear-left,rear-right",
          AL_FORMAT_51CHN32 },
        { "front-left,front-right,rear-left,rear-right", AL_FORMAT_QUAD32 },
        { "front-left,front-right", AL_FORMAT_STEREO_FLOAT32 },
        { "mono", AL_FORMAT_MONO_FLOAT32 },
        { NULL, 0 }
    };
    int i;
    (void)context;

    if(eol)
    {
        ppa_threaded_mainloop_signal(data->loop, 0);
        return;
    }

    for(i = 0;chanmaps[i].str;i++)
    {
        pa_channel_map map;
        if(!ppa_channel_map_parse(&map, chanmaps[i].str))
            continue;

        if(ppa_channel_map_equal(&info->channel_map, &map)
#if PA_CHECK_VERSION(0,9,15)
           || (ppa_channel_map_superset &&
               ppa_channel_map_superset(&info->channel_map, &map))
#endif
            )
        {
            device->Format = chanmaps[i].format;
            return;
        }
    }

    ppa_channel_map_snprint(chanmap_str, sizeof(chanmap_str), &info->channel_map);
    AL_PRINT("Failed to find format for channel map:\n    %s\n", chanmap_str);
}//}}}
//}}}

// PulseAudio I/O Callbacks //{{{
static void stream_write_callback(pa_stream *stream, size_t len, void *pdata) //{{{
{
    ALCdevice *Device = pdata;
    pulse_data *data = Device->ExtraData;

    while(len > 0)
    {
        size_t newlen = len;
        void *buf;
        pa_free_cb_t free_func = NULL;

#if PA_CHECK_VERSION(0,9,16)
        if(!ppa_stream_begin_write ||
           ppa_stream_begin_write(stream, &buf, &newlen) < 0)
#endif
        {
            buf = ppa_xmalloc(newlen);
            free_func = ppa_xfree;
        }

        aluMixData(Device, buf, newlen/data->frame_size);
        ppa_stream_write(stream, buf, newlen, free_func, 0, PA_SEEK_RELATIVE);
        len -= newlen;
    }
} //}}}
//}}}

static ALCboolean pulse_open(ALCdevice *device, const ALCchar *device_name) //{{{
{
    pulse_data *data = ppa_xmalloc(sizeof(pulse_data));
    pa_context_state_t state;

    memset(data, 0, sizeof(*data));

    if(ppa_get_binary_name(data->path_name, sizeof(data->path_name)))
        data->context_name = ppa_path_get_filename(data->path_name);
    else
        data->context_name = "OpenAL Soft";

    if(!(data->loop = ppa_threaded_mainloop_new()))
    {
        AL_PRINT("pa_threaded_mainloop_new() failed!\n");
        goto out;
    }

    if(ppa_threaded_mainloop_start(data->loop) < 0)
    {
        AL_PRINT("pa_threaded_mainloop_start() failed\n");
        goto out;
    }

    ppa_threaded_mainloop_lock(data->loop);
    device->ExtraData = data;

    data->context = ppa_context_new(ppa_threaded_mainloop_get_api(data->loop), data->context_name);
    if(!data->context)
    {
        AL_PRINT("pa_context_new() failed\n");

        ppa_threaded_mainloop_unlock(data->loop);
        goto out;
    }

    ppa_context_set_state_callback(data->context, context_state_callback, device);

    if(ppa_context_connect(data->context, NULL, pulse_ctx_flags, NULL) < 0)
    {
        AL_PRINT("Context did not connect: %s\n",
                 ppa_strerror(ppa_context_errno(data->context)));

        ppa_context_unref(data->context);
        data->context = NULL;

        ppa_threaded_mainloop_unlock(data->loop);
        goto out;
    }

    while((state=ppa_context_get_state(data->context)) != PA_CONTEXT_READY)
    {
        if(!PA_CONTEXT_IS_GOOD(state))
        {
            int err = ppa_context_errno(data->context);
            if(err != PA_ERR_CONNECTIONREFUSED)
                AL_PRINT("Context did not get ready: %s\n", ppa_strerror(err));

            ppa_context_unref(data->context);
            data->context = NULL;

            ppa_threaded_mainloop_unlock(data->loop);
            goto out;
        }

        ppa_threaded_mainloop_wait(data->loop);
    }
    ppa_context_set_state_callback(data->context, context_state_callback2, device);

    device->szDeviceName = strdup(device_name);

    ppa_threaded_mainloop_unlock(data->loop);
    return ALC_TRUE;

out:
    if(data->loop)
    {
        ppa_threaded_mainloop_stop(data->loop);
        ppa_threaded_mainloop_free(data->loop);
    }

    device->ExtraData = NULL;
    ppa_xfree(data);
    return ALC_FALSE;
} //}}}

static void pulse_close(ALCdevice *device) //{{{
{
    pulse_data *data = device->ExtraData;

    ppa_threaded_mainloop_lock(data->loop);

    if(data->stream)
    {
        ppa_stream_disconnect(data->stream);
        ppa_stream_unref(data->stream);
    }

    ppa_context_disconnect(data->context);
    ppa_context_unref(data->context);

    ppa_threaded_mainloop_unlock(data->loop);

    ppa_threaded_mainloop_stop(data->loop);
    ppa_threaded_mainloop_free(data->loop);

    DestroyRingBuffer(data->ring);

    device->ExtraData = NULL;
    ppa_xfree(data);
} //}}}
//}}}

// OpenAL {{{
static ALCboolean pulse_open_playback(ALCdevice *device, const ALCchar *device_name) //{{{
{
    if(!device_name)
        device_name = pulse_device;
    else if(strcmp(device_name, pulse_device) != 0)
        return ALC_FALSE;

    if(!pulse_load())
        return ALC_FALSE;

    if(pulse_open(device, device_name) != ALC_FALSE)
    {
        ALuint len = GetConfigValueInt("pulse", "buffer-length", 2048);
        if(len != 0)
        {
            device->UpdateSize = len;
            device->NumUpdates = 1;
        }
        return ALC_TRUE;
    }

    pulse_unload();
    return ALC_FALSE;
} //}}}

static void pulse_close_playback(ALCdevice *device) //{{{
{
    pulse_close(device);
    pulse_unload();
} //}}}

static ALCboolean pulse_reset_playback(ALCdevice *device) //{{{
{
    pulse_data *data = device->ExtraData;
    pa_stream_flags_t flags = 0;
    pa_stream_state_t state;
    pa_channel_map chanmap;

    ppa_threaded_mainloop_lock(data->loop);

    if(!ConfigValueExists(NULL, "format"))
    {
        pa_operation *o;
        struct {
            pa_threaded_mainloop *loop;
            char *name;
        } server_data;
        server_data.loop = data->loop;
        server_data.name = NULL;

        o = ppa_context_get_server_info(data->context, server_info_callback, &server_data);
        while(ppa_operation_get_state(o) == PA_OPERATION_RUNNING)
            ppa_threaded_mainloop_wait(data->loop);
        ppa_operation_unref(o);
        if(server_data.name)
        {
            o = ppa_context_get_sink_info_by_name(data->context, server_data.name, sink_info_callback, device);
            while(ppa_operation_get_state(o) == PA_OPERATION_RUNNING)
                ppa_threaded_mainloop_wait(data->loop);
            ppa_operation_unref(o);
            free(server_data.name);
        }
    }
    if(!ConfigValueExists(NULL, "frequency"))
        flags |= PA_STREAM_FIX_RATE;

    data->frame_size = aluBytesFromFormat(device->Format) *
                       aluChannelsFromFormat(device->Format);
    data->stream_name = "Playback Stream";
    data->attr.minreq = -1;
    data->attr.prebuf = -1;
    data->attr.fragsize = -1;
    data->attr.tlength = device->UpdateSize * device->NumUpdates *
                         data->frame_size;
    data->attr.maxlength = data->attr.tlength;

    switch(aluBytesFromFormat(device->Format))
    {
        case 1:
            data->spec.format = PA_SAMPLE_U8;
            break;
        case 2:
            data->spec.format = PA_SAMPLE_S16NE;
            break;
        case 4:
            data->spec.format = PA_SAMPLE_FLOAT32NE;
            break;
        default:
            AL_PRINT("Unknown format: 0x%x\n", device->Format);
            ppa_threaded_mainloop_unlock(data->loop);
            return ALC_FALSE;
    }
    data->spec.rate = device->Frequency;
    data->spec.channels = aluChannelsFromFormat(device->Format);

    if(ppa_sample_spec_valid(&data->spec) == 0)
    {
        AL_PRINT("Invalid sample format\n");
        ppa_threaded_mainloop_unlock(data->loop);
        return ALC_FALSE;
    }

    if(!ppa_channel_map_init_auto(&chanmap, data->spec.channels, PA_CHANNEL_MAP_WAVEEX))
    {
        AL_PRINT("Couldn't build map for channel count (%d)!\n", data->spec.channels);
        ppa_threaded_mainloop_unlock(data->loop);
        return ALC_FALSE;
    }
    SetDefaultWFXChannelOrder(device);

    data->stream = ppa_stream_new(data->context, data->stream_name, &data->spec, &chanmap);
    if(!data->stream)
    {
        AL_PRINT("pa_stream_new() failed: %s\n",
                 ppa_strerror(ppa_context_errno(data->context)));

        ppa_threaded_mainloop_unlock(data->loop);
        return ALC_FALSE;
    }

    ppa_stream_set_state_callback(data->stream, stream_state_callback, device);

    if(ppa_stream_connect_playback(data->stream, NULL, &data->attr, flags, NULL, NULL) < 0)
    {
        AL_PRINT("Stream did not connect: %s\n",
                 ppa_strerror(ppa_context_errno(data->context)));

        ppa_stream_unref(data->stream);
        data->stream = NULL;

        ppa_threaded_mainloop_unlock(data->loop);
        return ALC_FALSE;
    }

    while((state=ppa_stream_get_state(data->stream)) != PA_STREAM_READY)
    {
        if(!PA_STREAM_IS_GOOD(state))
        {
            AL_PRINT("Stream did not get ready: %s\n",
                     ppa_strerror(ppa_context_errno(data->context)));

            ppa_stream_unref(data->stream);
            data->stream = NULL;

            ppa_threaded_mainloop_unlock(data->loop);
            return ALC_FALSE;
        }

        ppa_threaded_mainloop_wait(data->loop);
    }
    ppa_stream_set_state_callback(data->stream, stream_state_callback2, device);

    data->spec = *(ppa_stream_get_sample_spec(data->stream));
    if(device->Frequency != data->spec.rate)
    {
        pa_operation *o;

        /* Server updated our playback rate, so modify the buffer attribs
         * accordingly. */
        data->attr.tlength = (ALuint64)(data->attr.tlength/data->frame_size) *
                             data->spec.rate / device->Frequency * data->frame_size;
        data->attr.maxlength = data->attr.tlength;

        o = ppa_stream_set_buffer_attr(data->stream, &data->attr,
                                       stream_success_callback, device);
        while(ppa_operation_get_state(o) == PA_OPERATION_RUNNING)
            ppa_threaded_mainloop_wait(data->loop);
        ppa_operation_unref(o);

        device->Frequency = data->spec.rate;
    }

    stream_buffer_attr_callback(data->stream, device);
#if PA_CHECK_VERSION(0,9,15)
    if(ppa_stream_set_buffer_attr_callback)
        ppa_stream_set_buffer_attr_callback(data->stream, stream_buffer_attr_callback, device);
#endif

    stream_write_callback(data->stream, data->attr.tlength, device);
    ppa_stream_set_write_callback(data->stream, stream_write_callback, device);

    ppa_threaded_mainloop_unlock(data->loop);
    return ALC_TRUE;
} //}}}

static void pulse_stop_playback(ALCdevice *device) //{{{
{
    pulse_data *data = device->ExtraData;

    if(!data->stream)
        return;

    ppa_threaded_mainloop_lock(data->loop);

    ppa_stream_disconnect(data->stream);
    ppa_stream_unref(data->stream);
    data->stream = NULL;

    ppa_threaded_mainloop_unlock(data->loop);
} //}}}


static ALCboolean pulse_open_capture(ALCdevice *device, const ALCchar *device_name) //{{{
{
    pulse_data *data;
    pa_stream_flags_t flags = 0;
    pa_stream_state_t state;
    pa_channel_map chanmap;

    if(!device_name)
        device_name = pulse_capture_device;
    else if(strcmp(device_name, pulse_capture_device) != 0)
        return ALC_FALSE;

    if(!pulse_load())
        return ALC_FALSE;

    if(pulse_open(device, device_name) == ALC_FALSE)
    {
        pulse_unload();
        return ALC_FALSE;
    }

    data = device->ExtraData;
    ppa_threaded_mainloop_lock(data->loop);

    data->samples = device->UpdateSize * device->NumUpdates;
    data->frame_size = aluBytesFromFormat(device->Format) *
                       aluChannelsFromFormat(device->Format);

    if(!(data->ring = CreateRingBuffer(data->frame_size, data->samples)))
    {
        ppa_threaded_mainloop_unlock(data->loop);
        goto fail;
    }

    data->attr.minreq = -1;
    data->attr.prebuf = -1;
    data->attr.maxlength = data->frame_size * data->samples;
    data->attr.tlength = -1;
    data->attr.fragsize = min(data->frame_size * data->samples,
                              10 * device->Frequency / 1000);
    data->stream_name = "Capture Stream";

    data->spec.rate = device->Frequency;
    data->spec.channels = aluChannelsFromFormat(device->Format);

    switch(aluBytesFromFormat(device->Format))
    {
        case 1:
            data->spec.format = PA_SAMPLE_U8;
            break;
        case 2:
            data->spec.format = PA_SAMPLE_S16NE;
            break;
        case 4:
            data->spec.format = PA_SAMPLE_FLOAT32NE;
            break;
        default:
            AL_PRINT("Unknown format: 0x%x\n", device->Format);
            ppa_threaded_mainloop_unlock(data->loop);
            goto fail;
    }

    if(ppa_sample_spec_valid(&data->spec) == 0)
    {
        AL_PRINT("Invalid sample format\n");
        ppa_threaded_mainloop_unlock(data->loop);
        goto fail;
    }

    if(!ppa_channel_map_init_auto(&chanmap, data->spec.channels, PA_CHANNEL_MAP_WAVEEX))
    {
        AL_PRINT("Couldn't build map for channel count (%d)!\n", data->spec.channels);
        ppa_threaded_mainloop_unlock(data->loop);
        goto fail;
    }

    data->stream = ppa_stream_new(data->context, data->stream_name, &data->spec, &chanmap);
    if(!data->stream)
    {
        AL_PRINT("pa_stream_new() failed: %s\n",
                 ppa_strerror(ppa_context_errno(data->context)));

        ppa_threaded_mainloop_unlock(data->loop);
        goto fail;
    }

    ppa_stream_set_state_callback(data->stream, stream_state_callback, device);

    flags |= PA_STREAM_START_CORKED|PA_STREAM_ADJUST_LATENCY;
    if(ppa_stream_connect_record(data->stream, NULL, &data->attr, flags) < 0)
    {
        AL_PRINT("Stream did not connect: %s\n",
                 ppa_strerror(ppa_context_errno(data->context)));

        ppa_stream_unref(data->stream);
        data->stream = NULL;

        ppa_threaded_mainloop_unlock(data->loop);
        goto fail;
    }

    while((state=ppa_stream_get_state(data->stream)) != PA_STREAM_READY)
    {
        if(!PA_STREAM_IS_GOOD(state))
        {
            AL_PRINT("Stream did not get ready: %s\n",
                     ppa_strerror(ppa_context_errno(data->context)));

            ppa_stream_unref(data->stream);
            data->stream = NULL;

            ppa_threaded_mainloop_unlock(data->loop);
            goto fail;
        }

        ppa_threaded_mainloop_wait(data->loop);
    }
    ppa_stream_set_state_callback(data->stream, stream_state_callback2, device);

    ppa_threaded_mainloop_unlock(data->loop);
    return ALC_TRUE;

fail:
    pulse_close(device);
    pulse_unload();
    return ALC_FALSE;
} //}}}

static void pulse_close_capture(ALCdevice *device) //{{{
{
    pulse_close(device);
    pulse_unload();
} //}}}

static void pulse_start_capture(ALCdevice *device) //{{{
{
    pulse_data *data = device->ExtraData;
    pa_operation *o;

    ppa_threaded_mainloop_lock(data->loop);
    o = ppa_stream_cork(data->stream, 0, stream_success_callback, device);
    while(ppa_operation_get_state(o) == PA_OPERATION_RUNNING)
        ppa_threaded_mainloop_wait(data->loop);
    ppa_operation_unref(o);
    ppa_threaded_mainloop_unlock(data->loop);
} //}}}

static void pulse_stop_capture(ALCdevice *device) //{{{
{
    pulse_data *data = device->ExtraData;
    pa_operation *o;

    ppa_threaded_mainloop_lock(data->loop);
    o = ppa_stream_cork(data->stream, 1, stream_success_callback, device);
    while(ppa_operation_get_state(o) == PA_OPERATION_RUNNING)
        ppa_threaded_mainloop_wait(data->loop);
    ppa_operation_unref(o);
    ppa_threaded_mainloop_unlock(data->loop);
} //}}}

static void pulse_capture_samples(ALCdevice *device, ALCvoid *buffer, ALCuint samples) //{{{
{
    pulse_data *data = device->ExtraData;
    ALCuint available = RingBufferSize(data->ring);
    const void *buf;
    size_t length;

    available *= data->frame_size;
    samples *= data->frame_size;

    ppa_threaded_mainloop_lock(data->loop);
    if(available+ppa_stream_readable_size(data->stream) < samples)
    {
        ppa_threaded_mainloop_unlock(data->loop);
        alcSetError(device, ALC_INVALID_VALUE);
        return;
    }

    available = min(available, samples);
    if(available > 0)
    {
        ReadRingBuffer(data->ring, buffer, available/data->frame_size);
        buffer = (ALubyte*)buffer + available;
        samples -= available;
    }

    /* Capture is done in fragment-sized chunks, so we loop until we get all
     * that's requested */
    while(samples > 0)
    {
        if(ppa_stream_peek(data->stream, &buf, &length) < 0)
        {
            AL_PRINT("pa_stream_peek() failed: %s\n",
                     ppa_strerror(ppa_context_errno(data->context)));
            break;
        }
        available = min(length, samples);

        memcpy(buffer, buf, available);
        buffer = (ALubyte*)buffer + available;
        buf = (const ALubyte*)buf + available;
        samples -= available;
        length -= available;

        /* Any unread data in the fragment will be lost, so save it */
        length /= data->frame_size;
        if(length > 0)
        {
            if(length > data->samples)
                length = data->samples;
            WriteRingBuffer(data->ring, buf, length);
        }

        ppa_stream_drop(data->stream);
    }
    ppa_threaded_mainloop_unlock(data->loop);
} //}}}

static ALCuint pulse_available_samples(ALCdevice *device) //{{{
{
    pulse_data *data = device->ExtraData;
    ALCuint ret;

    ppa_threaded_mainloop_lock(data->loop);
    ret  = RingBufferSize(data->ring);
    ret += ppa_stream_readable_size(data->stream)/data->frame_size;
    ppa_threaded_mainloop_unlock(data->loop);

    return ret;
} //}}}

BackendFuncs pulse_funcs = { //{{{
    pulse_open_playback,
    pulse_close_playback,
    pulse_reset_playback,
    pulse_stop_playback,
    pulse_open_capture,
    pulse_close_capture,
    pulse_start_capture,
    pulse_stop_capture,
    pulse_capture_samples,
    pulse_available_samples
}; //}}}

void alc_pulse_init(BackendFuncs *func_list) //{{{
{
    *func_list = pulse_funcs;

    pulse_ctx_flags = 0;
    if(!GetConfigValueBool("pulse", "spawn-server", 0))
        pulse_ctx_flags |= PA_CONTEXT_NOAUTOSPAWN;
} //}}}

void alc_pulse_deinit(void) //{{{
{
} //}}}

void alc_pulse_probe(int type) //{{{
{
    if(!pulse_load()) return;

    if(type == DEVICE_PROBE)
        AppendDeviceList(pulse_device);
    else if(type == ALL_DEVICE_PROBE)
        AppendAllDeviceList(pulse_device);
    else if(type == CAPTURE_DEVICE_PROBE)
        AppendCaptureDeviceList(pulse_capture_device);

    pulse_unload();
} //}}}
//}}}
