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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <ctype.h>

#include "alMain.h"
#include "alSource.h"
#include "AL/al.h"
#include "AL/alc.h"
#include "alThunk.h"
#include "alSource.h"
#include "alBuffer.h"
#include "alAuxEffectSlot.h"
#include "alDatabuffer.h"
#include "bs2b.h"
#include "alu.h"


#define EmptyFuncs { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
typedef struct BackendInfo {
    const char *name;
    void (*Init)(BackendFuncs*);
    void (*Deinit)(void);
    void (*Probe)(int);
    BackendFuncs Funcs;
} BackendInfo;
static BackendInfo BackendList[] = {
#ifdef HAVE_PULSEAUDIO
    { "pulse", alc_pulse_init, alc_pulse_deinit, alc_pulse_probe, EmptyFuncs },
#endif
#ifdef HAVE_ALSA
    { "alsa", alc_alsa_init, alc_alsa_deinit, alc_alsa_probe, EmptyFuncs },
#endif
#ifdef HAVE_OSS
    { "oss", alc_oss_init, alc_oss_deinit, alc_oss_probe, EmptyFuncs },
#endif
#ifdef HAVE_SOLARIS
    { "solaris", alc_solaris_init, alc_solaris_deinit, alc_solaris_probe, EmptyFuncs },
#endif
#ifdef HAVE_DSOUND
    { "dsound", alcDSoundInit, alcDSoundDeinit, alcDSoundProbe, EmptyFuncs },
#endif
#ifdef HAVE_WINMM
    { "winmm", alcWinMMInit, alcWinMMDeinit, alcWinMMProbe, EmptyFuncs },
#endif
#ifdef HAVE_PORTAUDIO
    { "port", alc_pa_init, alc_pa_deinit, alc_pa_probe, EmptyFuncs },
#endif

    { "wave", alc_wave_init, alc_wave_deinit, alc_wave_probe, EmptyFuncs },

    { NULL, NULL, NULL, NULL, EmptyFuncs }
};
#undef EmptyFuncs

///////////////////////////////////////////////////////

#define ALC_EFX_MAJOR_VERSION                              0x20001
#define ALC_EFX_MINOR_VERSION                              0x20002
#define ALC_MAX_AUXILIARY_SENDS                            0x20003

///////////////////////////////////////////////////////
// STRING and EXTENSIONS

typedef struct ALCfunction {
    const ALCchar *funcName;
    ALCvoid *address;
} ALCfunction;

typedef struct ALCenums {
    const ALCchar *enumName;
    ALCenum value;
} ALCenums;


static const ALCfunction alcFunctions[] = {
    { "alcCreateContext",           (ALCvoid *) alcCreateContext         },
    { "alcMakeContextCurrent",      (ALCvoid *) alcMakeContextCurrent    },
    { "alcProcessContext",          (ALCvoid *) alcProcessContext        },
    { "alcSuspendContext",          (ALCvoid *) alcSuspendContext        },
    { "alcDestroyContext",          (ALCvoid *) alcDestroyContext        },
    { "alcGetCurrentContext",       (ALCvoid *) alcGetCurrentContext     },
    { "alcGetContextsDevice",       (ALCvoid *) alcGetContextsDevice     },
    { "alcOpenDevice",              (ALCvoid *) alcOpenDevice            },
    { "alcCloseDevice",             (ALCvoid *) alcCloseDevice           },
    { "alcGetError",                (ALCvoid *) alcGetError              },
    { "alcIsExtensionPresent",      (ALCvoid *) alcIsExtensionPresent    },
    { "alcGetProcAddress",          (ALCvoid *) alcGetProcAddress        },
    { "alcGetEnumValue",            (ALCvoid *) alcGetEnumValue          },
    { "alcGetString",               (ALCvoid *) alcGetString             },
    { "alcGetIntegerv",             (ALCvoid *) alcGetIntegerv           },
    { "alcCaptureOpenDevice",       (ALCvoid *) alcCaptureOpenDevice     },
    { "alcCaptureCloseDevice",      (ALCvoid *) alcCaptureCloseDevice    },
    { "alcCaptureStart",            (ALCvoid *) alcCaptureStart          },
    { "alcCaptureStop",             (ALCvoid *) alcCaptureStop           },
    { "alcCaptureSamples",          (ALCvoid *) alcCaptureSamples        },

    { "alcSetThreadContext",        (ALCvoid *) alcSetThreadContext      },
    { "alcGetThreadContext",        (ALCvoid *) alcGetThreadContext      },

    { NULL,                         (ALCvoid *) NULL                     }
};

static const ALCenums enumeration[] = {
    // Types
    { "ALC_INVALID",                          ALC_INVALID                         },
    { "ALC_FALSE",                            ALC_FALSE                           },
    { "ALC_TRUE",                             ALC_TRUE                            },

    // ALC Properties
    { "ALC_MAJOR_VERSION",                    ALC_MAJOR_VERSION                   },
    { "ALC_MINOR_VERSION",                    ALC_MINOR_VERSION                   },
    { "ALC_ATTRIBUTES_SIZE",                  ALC_ATTRIBUTES_SIZE                 },
    { "ALC_ALL_ATTRIBUTES",                   ALC_ALL_ATTRIBUTES                  },
    { "ALC_DEFAULT_DEVICE_SPECIFIER",         ALC_DEFAULT_DEVICE_SPECIFIER        },
    { "ALC_DEVICE_SPECIFIER",                 ALC_DEVICE_SPECIFIER                },
    { "ALC_ALL_DEVICES_SPECIFIER",            ALC_ALL_DEVICES_SPECIFIER           },
    { "ALC_DEFAULT_ALL_DEVICES_SPECIFIER",    ALC_DEFAULT_ALL_DEVICES_SPECIFIER   },
    { "ALC_EXTENSIONS",                       ALC_EXTENSIONS                      },
    { "ALC_FREQUENCY",                        ALC_FREQUENCY                       },
    { "ALC_REFRESH",                          ALC_REFRESH                         },
    { "ALC_SYNC",                             ALC_SYNC                            },
    { "ALC_MONO_SOURCES",                     ALC_MONO_SOURCES                    },
    { "ALC_STEREO_SOURCES",                   ALC_STEREO_SOURCES                  },
    { "ALC_CAPTURE_DEVICE_SPECIFIER",         ALC_CAPTURE_DEVICE_SPECIFIER        },
    { "ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER", ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER},
    { "ALC_CAPTURE_SAMPLES",                  ALC_CAPTURE_SAMPLES                 },

    // EFX Properties
    { "ALC_EFX_MAJOR_VERSION",                ALC_EFX_MAJOR_VERSION               },
    { "ALC_EFX_MINOR_VERSION",                ALC_EFX_MINOR_VERSION               },
    { "ALC_MAX_AUXILIARY_SENDS",              ALC_MAX_AUXILIARY_SENDS             },

    // ALC Error Message
    { "ALC_NO_ERROR",                         ALC_NO_ERROR                        },
    { "ALC_INVALID_DEVICE",                   ALC_INVALID_DEVICE                  },
    { "ALC_INVALID_CONTEXT",                  ALC_INVALID_CONTEXT                 },
    { "ALC_INVALID_ENUM",                     ALC_INVALID_ENUM                    },
    { "ALC_INVALID_VALUE",                    ALC_INVALID_VALUE                   },
    { "ALC_OUT_OF_MEMORY",                    ALC_OUT_OF_MEMORY                   },
    { NULL,                                   (ALCenum)0 }
};
// Error strings
static const ALCchar alcNoError[] = "No Error";
static const ALCchar alcErrInvalidDevice[] = "Invalid Device";
static const ALCchar alcErrInvalidContext[] = "Invalid Context";
static const ALCchar alcErrInvalidEnum[] = "Invalid Enum";
static const ALCchar alcErrInvalidValue[] = "Invalid Value";
static const ALCchar alcErrOutOfMemory[] = "Out of Memory";

/* Device lists. Sizes only include the first ending null character, not the
 * second */
static ALCchar *alcDeviceList;
static size_t alcDeviceListSize;
static ALCchar *alcAllDeviceList;
static size_t alcAllDeviceListSize;
static ALCchar *alcCaptureDeviceList;
static size_t alcCaptureDeviceListSize;
// Default is always the first in the list
static ALCchar *alcDefaultDeviceSpecifier;
static ALCchar *alcDefaultAllDeviceSpecifier;
static ALCchar *alcCaptureDefaultDeviceSpecifier;


static const ALCchar alcNoDeviceExtList[] =
    "ALC_ENUMERATE_ALL_EXT ALC_ENUMERATION_EXT ALC_EXT_CAPTURE "
    "ALC_EXT_thread_local_context";
static const ALCchar alcExtensionList[] =
    "ALC_ENUMERATE_ALL_EXT ALC_ENUMERATION_EXT ALC_EXT_CAPTURE "
    "ALC_EXT_disconnect ALC_EXT_EFX ALC_EXT_thread_local_context";
static const ALCint alcMajorVersion = 1;
static const ALCint alcMinorVersion = 1;

static const ALCint alcEFXMajorVersion = 1;
static const ALCint alcEFXMinorVersion = 0;

///////////////////////////////////////////////////////


///////////////////////////////////////////////////////
// Global Variables

static ALCdevice *g_pDeviceList = NULL;
static ALCuint    g_ulDeviceCount = 0;

static CRITICAL_SECTION g_csMutex;

// Context List
static ALCcontext *g_pContextList = NULL;
static ALCuint     g_ulContextCount = 0;

// Thread-local current context
static tls_type LocalContext;
// Process-wide current context
static ALCcontext *GlobalContext;

// Context Error
static ALCenum g_eLastContextError = ALC_NO_ERROR;

// Default context extensions
static const ALchar alExtList[] =
    "AL_EXTX_buffer_sub_data AL_EXT_DOUBLE AL_EXT_EXPONENT_DISTANCE "
    "AL_EXT_FLOAT32 AL_EXT_IMA4 AL_EXT_LINEAR_DISTANCE AL_EXT_MCFORMATS "
    "AL_EXT_MULAW AL_EXT_MULAW_MCFORMATS AL_EXT_OFFSET "
    "AL_EXTX_sample_buffer_object AL_EXT_source_distance_model "
    "AL_LOKI_quadriphonic";

// Mixing Priority Level
ALint RTPrioLevel;

// Resampler Quality
resampler_t DefaultResampler;

///////////////////////////////////////////////////////


///////////////////////////////////////////////////////
// ALC Related helper functions
#ifdef _WIN32
static void alc_init(void);
static void alc_deinit(void);

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
    (void)lpReserved;

    // Perform actions based on the reason for calling.
    switch(ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            alc_init();
            break;

        case DLL_PROCESS_DETACH:
            alc_deinit();
            break;
    }
    return TRUE;
}
#else
#ifdef HAVE_GCC_DESTRUCTOR
static void alc_init(void) __attribute__((constructor));
static void alc_deinit(void) __attribute__((destructor));
#endif
#endif

static void alc_init(void)
{
    int i;
    const char *devs, *str;

    InitializeCriticalSection(&g_csMutex);
    ALTHUNK_INIT();
    ReadALConfig();

    tls_create(&LocalContext);

    RTPrioLevel = GetConfigValueInt(NULL, "rt-prio", 0);

    DefaultResampler = GetConfigValueInt(NULL, "resampler", RESAMPLER_DEFAULT);
    if(DefaultResampler >= RESAMPLER_MAX || DefaultResampler <= RESAMPLER_MIN)
        DefaultResampler = RESAMPLER_DEFAULT;

    devs = GetConfigValue(NULL, "drivers", "");
    if(devs[0])
    {
        int n;
        size_t len;
        const char *next = devs;
        int endlist;

        i = 0;
        do {
            devs = next;
            next = strchr(devs, ',');

            if(!devs[0] || devs[0] == ',')
            {
                endlist = 0;
                continue;
            }
            endlist = 1;

            len = (next ? ((size_t)(next-devs)) : strlen(devs));
            for(n = i;BackendList[n].Init;n++)
            {
                if(len == strlen(BackendList[n].name) &&
                   strncmp(BackendList[n].name, devs, len) == 0)
                {
                    BackendInfo Bkp = BackendList[n];
                    while(n > i)
                    {
                        BackendList[n] = BackendList[n-1];
                        --n;
                    }
                    BackendList[n] = Bkp;

                    i++;
                    break;
                }
            }
        } while(next++);

        if(endlist)
        {
            BackendList[i].name = NULL;
            BackendList[i].Init = NULL;
            BackendList[i].Deinit = NULL;
            BackendList[i].Probe = NULL;
        }
    }

    for(i = 0;BackendList[i].Init;i++)
        BackendList[i].Init(&BackendList[i].Funcs);

    DuplicateStereo = GetConfigValueBool(NULL, "stereodup", 0);

    str = GetConfigValue(NULL, "excludefx", "");
    if(str[0])
    {
        const struct {
            const char *name;
            int type;
        } EffectList[] = {
            { "eaxreverb", EAXREVERB },
            { "reverb", REVERB },
            { "echo", ECHO },
            { NULL, 0 }
        };
        int n;
        size_t len;
        const char *next = str;

        do {
            str = next;
            next = strchr(str, ',');

            if(!str[0] || next == str)
                continue;

            len = (next ? ((size_t)(next-str)) : strlen(str));
            for(n = 0;EffectList[n].name;n++)
            {
                if(len == strlen(EffectList[n].name) &&
                   strncmp(EffectList[n].name, str, len) == 0)
                    DisabledEffects[EffectList[n].type] = AL_TRUE;
            }
        } while(next++);
    }
}

static void alc_deinit(void)
{
    int i;

    ReleaseALC();

    for(i = 0;BackendList[i].Deinit;i++)
        BackendList[i].Deinit();

    tls_delete(LocalContext);

    FreeALConfig();
    ALTHUNK_EXIT();
    DeleteCriticalSection(&g_csMutex);
}


static void ProbeDeviceList()
{
    ALint i;

    free(alcDeviceList); alcDeviceList = NULL;
    alcDeviceListSize = 0;

    for(i = 0;BackendList[i].Probe;i++)
        BackendList[i].Probe(DEVICE_PROBE);
}

static void ProbeAllDeviceList()
{
    ALint i;

    free(alcAllDeviceList); alcAllDeviceList = NULL;
    alcAllDeviceListSize = 0;

    for(i = 0;BackendList[i].Probe;i++)
        BackendList[i].Probe(ALL_DEVICE_PROBE);
}

static void ProbeCaptureDeviceList()
{
    ALint i;

    free(alcCaptureDeviceList); alcCaptureDeviceList = NULL;
    alcCaptureDeviceListSize = 0;

    for(i = 0;BackendList[i].Probe;i++)
        BackendList[i].Probe(CAPTURE_DEVICE_PROBE);
}


#define DECL_APPEND_LIST_FUNC(type)                                          \
void Append##type##List(const ALCchar *name)                                 \
{                                                                            \
    size_t len = strlen(name);                                               \
    void *temp;                                                              \
                                                                             \
    if(len == 0)                                                             \
        return;                                                              \
                                                                             \
    temp = realloc(alc##type##List, alc##type##ListSize + len + 2);          \
    if(!temp)                                                                \
    {                                                                        \
        AL_PRINT("Realloc failed to add %s!\n", name);                       \
        return;                                                              \
    }                                                                        \
    alc##type##List = temp;                                                  \
    sprintf(alc##type##List+alc##type##ListSize, "%s", name);                \
    alc##type##ListSize += len+1;                                            \
    alc##type##List[alc##type##ListSize] = 0;                                \
}

DECL_APPEND_LIST_FUNC(Device)
DECL_APPEND_LIST_FUNC(AllDevice)
DECL_APPEND_LIST_FUNC(CaptureDevice)


void al_print(const char *fname, unsigned int line, const char *fmt, ...)
{
    const char *fn;
    char str[256];
    int i;

    fn = strrchr(fname, '/');
    if(!fn) fn = strrchr(fname, '\\');;
    if(!fn) fn = fname;
    else fn += 1;

    i = snprintf(str, sizeof(str), "AL lib: %s:%d: ", fn, line);
    if(i < (int)sizeof(str) && i > 0)
    {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(str+i, sizeof(str)-i, fmt, ap);
        va_end(ap);
    }
    str[sizeof(str)-1] = 0;

    fprintf(stderr, "%s", str);
}

void EnableRTPrio(ALint level)
{
    ALboolean failed;

#ifdef _WIN32
    if(level > 0)
        failed = !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    else
        failed = !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#elif defined(HAVE_PTHREAD_SETSCHEDPARAM)
    struct sched_param param;

    if(level > 0)
    {
        /* Use the minimum real-time priority possible for now (on Linux this
         * should be 1 for SCHED_RR) */
        param.sched_priority = sched_get_priority_min(SCHED_RR);
        failed = !!pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    }
    else
    {
        param.sched_priority = 0;
        failed = !!pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
    }
#else
    /* Real-time priority not available */
    failed = !!level;
#endif
    if(failed)
        AL_PRINT("Failed to set priority level for thread\n");
}


/*
    IsDevice

    Check pDevice is a valid Device pointer
*/
static ALCboolean IsDevice(ALCdevice *pDevice)
{
    ALCdevice *pTempDevice;

    SuspendContext(NULL);

    pTempDevice = g_pDeviceList;
    while(pTempDevice && pTempDevice != pDevice)
        pTempDevice = pTempDevice->next;

    ProcessContext(NULL);

    return (pTempDevice ? ALC_TRUE : ALC_FALSE);
}

/*
    IsContext

    Check pContext is a valid Context pointer
*/
static ALCboolean IsContext(ALCcontext *pContext)
{
    ALCcontext *pTempContext;

    SuspendContext(NULL);

    pTempContext = g_pContextList;
    while (pTempContext && pTempContext != pContext)
        pTempContext = pTempContext->next;

    ProcessContext(NULL);

    return (pTempContext ? ALC_TRUE : ALC_FALSE);
}


/*
    alcSetError

    Store latest ALC Error
*/
ALCvoid alcSetError(ALCdevice *device, ALenum errorCode)
{
    if(IsDevice(device))
        device->LastError = errorCode;
    else
        g_eLastContextError = errorCode;
}


/*
    SuspendContext

    Thread-safe entry
*/
ALCvoid SuspendContext(ALCcontext *pContext)
{
    (void)pContext;
    EnterCriticalSection(&g_csMutex);
}


/*
    ProcessContext

    Thread-safe exit
*/
ALCvoid ProcessContext(ALCcontext *pContext)
{
    (void)pContext;
    LeaveCriticalSection(&g_csMutex);
}


/*
    GetContextSuspended

    Returns the currently active Context, in a locked state
*/
ALCcontext *GetContextSuspended(void)
{
    ALCcontext *pContext = NULL;

    SuspendContext(NULL);

    pContext = tls_get(LocalContext);
    if(pContext && !IsContext(pContext))
    {
        tls_set(LocalContext, NULL);
        pContext = NULL;
    }
    if(!pContext)
        pContext = GlobalContext;

    if(pContext)
        SuspendContext(pContext);

    ProcessContext(NULL);

    return pContext;
}


/*
    InitContext

    Initialize Context variables
*/
static ALvoid InitContext(ALCcontext *pContext)
{
    //Initialise listener
    pContext->Listener.Gain = 1.0f;
    pContext->Listener.MetersPerUnit = 1.0f;
    pContext->Listener.Position[0] = 0.0f;
    pContext->Listener.Position[1] = 0.0f;
    pContext->Listener.Position[2] = 0.0f;
    pContext->Listener.Velocity[0] = 0.0f;
    pContext->Listener.Velocity[1] = 0.0f;
    pContext->Listener.Velocity[2] = 0.0f;
    pContext->Listener.Forward[0] = 0.0f;
    pContext->Listener.Forward[1] = 0.0f;
    pContext->Listener.Forward[2] = -1.0f;
    pContext->Listener.Up[0] = 0.0f;
    pContext->Listener.Up[1] = 1.0f;
    pContext->Listener.Up[2] = 0.0f;

    //Validate pContext
    pContext->LastError = AL_NO_ERROR;
    pContext->Suspended = AL_FALSE;

    //Set globals
    pContext->DistanceModel = AL_INVERSE_DISTANCE_CLAMPED;
    pContext->SourceDistanceModel = AL_FALSE;
    pContext->DopplerFactor = 1.0f;
    pContext->DopplerVelocity = 1.0f;
    pContext->flSpeedOfSound = SPEEDOFSOUNDMETRESPERSEC;

    pContext->ExtensionList = alExtList;
}


/*
    ExitContext

    Clean up Context, destroy any remaining Sources
*/
static ALCvoid ExitContext(ALCcontext *pContext)
{
    //Invalidate context
    pContext->LastError = AL_NO_ERROR;
}

///////////////////////////////////////////////////////


///////////////////////////////////////////////////////
// ALC Functions calls


// This should probably move to another c file but for now ...
ALC_API ALCdevice* ALC_APIENTRY alcCaptureOpenDevice(const ALCchar *deviceName, ALCuint frequency, ALCenum format, ALCsizei SampleSize)
{
    ALCboolean DeviceFound = ALC_FALSE;
    ALCdevice *device = NULL;
    ALCint i;

    if(SampleSize <= 0)
    {
        alcSetError(NULL, ALC_INVALID_VALUE);
        return NULL;
    }

    if(deviceName && !deviceName[0])
        deviceName = NULL;

    device = calloc(1, sizeof(ALCdevice));
    if(!device)
    {
        alcSetError(NULL, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    //Validate device
    device->Connected = ALC_TRUE;
    device->IsCaptureDevice = AL_TRUE;

    device->szDeviceName = NULL;

    device->Frequency = frequency;
    device->Format = format;
    device->UpdateSize = SampleSize;
    device->NumUpdates = 1;

    SuspendContext(NULL);
    for(i = 0;BackendList[i].Init;i++)
    {
        device->Funcs = &BackendList[i].Funcs;
        if(ALCdevice_OpenCapture(device, deviceName))
        {
            device->next = g_pDeviceList;
            g_pDeviceList = device;
            g_ulDeviceCount++;

            DeviceFound = ALC_TRUE;
            break;
        }
    }
    ProcessContext(NULL);

    if(!DeviceFound)
    {
        alcSetError(NULL, ALC_INVALID_VALUE);
        free(device);
        device = NULL;
    }

    return device;
}

ALC_API ALCboolean ALC_APIENTRY alcCaptureCloseDevice(ALCdevice *pDevice)
{
    ALCdevice **list;

    if(!IsDevice(pDevice) || !pDevice->IsCaptureDevice)
    {
        alcSetError(pDevice, ALC_INVALID_DEVICE);
        return ALC_FALSE;
    }

    SuspendContext(NULL);

    list = &g_pDeviceList;
    while(*list != pDevice)
        list = &(*list)->next;

    *list = (*list)->next;
    g_ulDeviceCount--;

    ProcessContext(NULL);

    ALCdevice_CloseCapture(pDevice);

    free(pDevice->szDeviceName);
    pDevice->szDeviceName = NULL;

    free(pDevice);

    return ALC_TRUE;
}

ALC_API void ALC_APIENTRY alcCaptureStart(ALCdevice *device)
{
    if(!IsDevice(device) || !device->IsCaptureDevice)
    {
        alcSetError(device, ALC_INVALID_DEVICE);
        return;
    }
    SuspendContext(NULL);
    ALCdevice_StartCapture(device);
    ProcessContext(NULL);
}

ALC_API void ALC_APIENTRY alcCaptureStop(ALCdevice *device)
{
    if(!IsDevice(device) || !device->IsCaptureDevice)
    {
        alcSetError(device, ALC_INVALID_DEVICE);
        return;
    }
    SuspendContext(NULL);
    ALCdevice_StopCapture(device);
    ProcessContext(NULL);
}

ALC_API void ALC_APIENTRY alcCaptureSamples(ALCdevice *device, ALCvoid *buffer, ALCsizei samples)
{
    if(!IsDevice(device) || !device->IsCaptureDevice)
    {
        alcSetError(device, ALC_INVALID_DEVICE);
        return;
    }
    SuspendContext(NULL);
    ALCdevice_CaptureSamples(device, buffer, samples);
    ProcessContext(NULL);
}

/*
    alcGetError

    Return last ALC generated error code
*/
ALC_API ALCenum ALC_APIENTRY alcGetError(ALCdevice *device)
{
    ALCenum errorCode = ALC_NO_ERROR;

    if(IsDevice(device))
    {
        errorCode = device->LastError;
        device->LastError = ALC_NO_ERROR;
    }
    else
    {
        errorCode = g_eLastContextError;
        g_eLastContextError = ALC_NO_ERROR;
    }
    return errorCode;
}


/*
    alcSuspendContext

    Not functional
*/
ALC_API ALCvoid ALC_APIENTRY alcSuspendContext(ALCcontext *pContext)
{
    SuspendContext(NULL);
    if(IsContext(pContext))
        pContext->Suspended = AL_TRUE;
    ProcessContext(NULL);
}


/*
    alcProcessContext

    Not functional
*/
ALC_API ALCvoid ALC_APIENTRY alcProcessContext(ALCcontext *pContext)
{
    SuspendContext(NULL);
    if(IsContext(pContext))
        pContext->Suspended = AL_FALSE;
    ProcessContext(NULL);
}


/*
    alcGetString

    Returns information about the Device, and error strings
*/
ALC_API const ALCchar* ALC_APIENTRY alcGetString(ALCdevice *pDevice,ALCenum param)
{
    const ALCchar *value = NULL;

    switch (param)
    {
    case ALC_NO_ERROR:
        value = alcNoError;
        break;

    case ALC_INVALID_ENUM:
        value = alcErrInvalidEnum;
        break;

    case ALC_INVALID_VALUE:
        value = alcErrInvalidValue;
        break;

    case ALC_INVALID_DEVICE:
        value = alcErrInvalidDevice;
        break;

    case ALC_INVALID_CONTEXT:
        value = alcErrInvalidContext;
        break;

    case ALC_OUT_OF_MEMORY:
        value = alcErrOutOfMemory;
        break;

    case ALC_DEVICE_SPECIFIER:
        if(IsDevice(pDevice))
            value = pDevice->szDeviceName;
        else
        {
            ProbeDeviceList();
            value = alcDeviceList;
        }
        break;

    case ALC_ALL_DEVICES_SPECIFIER:
        ProbeAllDeviceList();
        value = alcAllDeviceList;
        break;

    case ALC_CAPTURE_DEVICE_SPECIFIER:
        if(IsDevice(pDevice))
            value = pDevice->szDeviceName;
        else
        {
            ProbeCaptureDeviceList();
            value = alcCaptureDeviceList;
        }
        break;

    /* Default devices are always first in the list */
    case ALC_DEFAULT_DEVICE_SPECIFIER:
        if(!alcDeviceList)
            ProbeDeviceList();

        free(alcDefaultDeviceSpecifier);
        alcDefaultDeviceSpecifier = strdup(alcDeviceList ? alcDeviceList : "");
        if(!alcDefaultDeviceSpecifier)
            alcSetError(pDevice, ALC_OUT_OF_MEMORY);
        value = alcDefaultDeviceSpecifier;
        break;

    case ALC_DEFAULT_ALL_DEVICES_SPECIFIER:
        if(!alcAllDeviceList)
            ProbeAllDeviceList();

        free(alcDefaultAllDeviceSpecifier);
        alcDefaultAllDeviceSpecifier = strdup(alcAllDeviceList ?
                                              alcAllDeviceList : "");
        if(!alcDefaultAllDeviceSpecifier)
            alcSetError(pDevice, ALC_OUT_OF_MEMORY);
        value = alcDefaultAllDeviceSpecifier;
        break;

    case ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER:
        if(!alcCaptureDeviceList)
            ProbeCaptureDeviceList();

        free(alcCaptureDefaultDeviceSpecifier);
        alcCaptureDefaultDeviceSpecifier = strdup(alcCaptureDeviceList ?
                                                  alcCaptureDeviceList : "");
        if(!alcCaptureDefaultDeviceSpecifier)
            alcSetError(pDevice, ALC_OUT_OF_MEMORY);
        value = alcCaptureDefaultDeviceSpecifier;
        break;

    case ALC_EXTENSIONS:
        if(IsDevice(pDevice))
            value = alcExtensionList;
        else
            value = alcNoDeviceExtList;
        break;

    default:
        alcSetError(pDevice, ALC_INVALID_ENUM);
        break;
    }

    return value;
}


/*
    alcGetIntegerv

    Returns information about the Device and the version of Open AL
*/
ALC_API ALCvoid ALC_APIENTRY alcGetIntegerv(ALCdevice *device,ALCenum param,ALsizei size,ALCint *data)
{
    if(size == 0 || data == NULL)
    {
        alcSetError(device, ALC_INVALID_VALUE);
        return;
    }

    if(IsDevice(device) && device->IsCaptureDevice)
    {
        SuspendContext(NULL);

        // Capture device
        switch (param)
        {
        case ALC_CAPTURE_SAMPLES:
            *data = ALCdevice_AvailableSamples(device);
            break;

        case ALC_CONNECTED:
            *data = device->Connected;
            break;

        default:
            alcSetError(device, ALC_INVALID_ENUM);
            break;
        }

        ProcessContext(NULL);
        return;
    }

    // Playback Device
    switch (param)
    {
        case ALC_MAJOR_VERSION:
            *data = alcMajorVersion;
            break;

        case ALC_MINOR_VERSION:
            *data = alcMinorVersion;
            break;

        case ALC_EFX_MAJOR_VERSION:
            *data = alcEFXMajorVersion;
            break;

        case ALC_EFX_MINOR_VERSION:
            *data = alcEFXMinorVersion;
            break;

        case ALC_MAX_AUXILIARY_SENDS:
            if(!IsDevice(device))
                alcSetError(device, ALC_INVALID_DEVICE);
            else
                *data = device->NumAuxSends;
            break;

        case ALC_ATTRIBUTES_SIZE:
            if(!IsDevice(device))
                alcSetError(device, ALC_INVALID_DEVICE);
            else
                *data = 13;
            break;

        case ALC_ALL_ATTRIBUTES:
            if(!IsDevice(device))
                alcSetError(device, ALC_INVALID_DEVICE);
            else if (size < 13)
                alcSetError(device, ALC_INVALID_VALUE);
            else
            {
                int i = 0;

                SuspendContext(NULL);
                data[i++] = ALC_FREQUENCY;
                data[i++] = device->Frequency;

                data[i++] = ALC_REFRESH;
                data[i++] = device->Frequency / device->UpdateSize;

                data[i++] = ALC_SYNC;
                data[i++] = ALC_FALSE;

                data[i++] = ALC_MONO_SOURCES;
                data[i++] = device->NumMonoSources;

                data[i++] = ALC_STEREO_SOURCES;
                data[i++] = device->NumStereoSources;

                data[i++] = ALC_MAX_AUXILIARY_SENDS;
                data[i++] = device->NumAuxSends;

                data[i++] = 0;
                ProcessContext(NULL);
            }
            break;

        case ALC_FREQUENCY:
            if(!IsDevice(device))
                alcSetError(device, ALC_INVALID_DEVICE);
            else
                *data = device->Frequency;
            break;

        case ALC_REFRESH:
            if(!IsDevice(device))
                alcSetError(device, ALC_INVALID_DEVICE);
            else
                *data = device->Frequency / device->UpdateSize;
            break;

        case ALC_SYNC:
            if(!IsDevice(device))
                alcSetError(device, ALC_INVALID_DEVICE);
            else
                *data = ALC_FALSE;
            break;

        case ALC_MONO_SOURCES:
            if(!IsDevice(device))
                alcSetError(device, ALC_INVALID_DEVICE);
            else
                *data = device->NumMonoSources;
            break;

        case ALC_STEREO_SOURCES:
            if(!IsDevice(device))
                alcSetError(device, ALC_INVALID_DEVICE);
            else
                *data = device->NumStereoSources;
            break;

        case ALC_CONNECTED:
            if(!IsDevice(device))
                alcSetError(device, ALC_INVALID_DEVICE);
            else
                *data = device->Connected;
            break;

        default:
            alcSetError(device, ALC_INVALID_ENUM);
            break;
    }
}


/*
    alcIsExtensionPresent

    Determines if there is support for a particular extension
*/
ALC_API ALCboolean ALC_APIENTRY alcIsExtensionPresent(ALCdevice *device, const ALCchar *extName)
{
    ALCboolean bResult = ALC_FALSE;
    const char *ptr;
    size_t len;

    if(!extName)
    {
        alcSetError(device, ALC_INVALID_VALUE);
        return ALC_FALSE;
    }

    len = strlen(extName);
    ptr = (IsDevice(device) ? alcExtensionList : alcNoDeviceExtList);
    while(ptr && *ptr)
    {
        if(strncasecmp(ptr, extName, len) == 0 &&
           (ptr[len] == '\0' || isspace(ptr[len])))
        {
            bResult = ALC_TRUE;
            break;
        }
        if((ptr=strchr(ptr, ' ')) != NULL)
        {
            do {
                ++ptr;
            } while(isspace(*ptr));
        }
    }

    return bResult;
}


/*
    alcGetProcAddress

    Retrieves the function address for a particular extension function
*/
ALC_API ALCvoid* ALC_APIENTRY alcGetProcAddress(ALCdevice *device, const ALCchar *funcName)
{
    ALsizei i = 0;

    if(!funcName)
    {
        alcSetError(device, ALC_INVALID_VALUE);
        return NULL;
    }

    while(alcFunctions[i].funcName && strcmp(alcFunctions[i].funcName,funcName) != 0)
        i++;
    return alcFunctions[i].address;
}


/*
    alcGetEnumValue

    Get the value for a particular ALC Enumerated Value
*/
ALC_API ALCenum ALC_APIENTRY alcGetEnumValue(ALCdevice *device, const ALCchar *enumName)
{
    ALsizei i = 0;

    if(!enumName)
    {
        alcSetError(device, ALC_INVALID_VALUE);
        return (ALCenum)0;
    }

    while(enumeration[i].enumName && strcmp(enumeration[i].enumName,enumName) != 0)
        i++;
    return enumeration[i].value;
}


/*
    alcCreateContext

    Create and attach a Context to a particular Device.
*/
ALC_API ALCcontext* ALC_APIENTRY alcCreateContext(ALCdevice *device, const ALCint *attrList)
{
    ALCcontext *ALContext;
    ALuint attrIdx;
    void *temp;
    ALuint i;

    SuspendContext(NULL);

    if(!IsDevice(device) || device->IsCaptureDevice || !device->Connected)
    {
        alcSetError(device, ALC_INVALID_DEVICE);
        ProcessContext(NULL);
        return NULL;
    }

    // Reset Context Last Error code
    device->LastError = ALC_NO_ERROR;

    // If a context is already running on the device, stop playback so the
    // device attributes can be updated
    if(device->NumContexts > 0)
    {
        ProcessContext(NULL);
        ALCdevice_StopPlayback(device);
        SuspendContext(NULL);
    }

    // Check for attributes
    if(attrList)
    {
        ALCuint freq = device->Frequency;
        ALCuint numMono = device->NumMonoSources;
        ALCuint numStereo = device->NumStereoSources;
        ALCuint numSends = device->NumAuxSends;

        attrIdx = 0;
        while(attrList[attrIdx])
        {
            if(attrList[attrIdx] == ALC_FREQUENCY &&
               !ConfigValueExists(NULL, "frequency"))
            {
                freq = attrList[attrIdx + 1];
                if(freq < 8000)
                    freq = 8000;
            }

            if(attrList[attrIdx] == ALC_STEREO_SOURCES)
            {
                numStereo = attrList[attrIdx + 1];
                if(numStereo > device->MaxNoOfSources)
                    numStereo = device->MaxNoOfSources;

                numMono = device->MaxNoOfSources - numStereo;
            }

            if(attrList[attrIdx] == ALC_MAX_AUXILIARY_SENDS &&
               !ConfigValueExists(NULL, "sends"))
            {
                numSends = attrList[attrIdx + 1];
                if(numSends > MAX_SENDS)
                    numSends = MAX_SENDS;
            }

            attrIdx += 2;
        }

        device->UpdateSize = (ALuint64)device->UpdateSize * freq /
                             device->Frequency;

        device->Frequency = freq;
        device->NumMonoSources = numMono;
        device->NumStereoSources = numStereo;
        device->NumAuxSends = numSends;
    }

    if(ALCdevice_ResetPlayback(device) == ALC_FALSE)
    {
        alcSetError(device, ALC_INVALID_DEVICE);
        aluHandleDisconnect(device);
        ProcessContext(NULL);
        return NULL;
    }

    for(i = 0;i < device->NumContexts;i++)
    {
        ALCcontext *context = device->Contexts[i];
        ALeffectslot *slot;
        ALsource *source;

        SuspendContext(context);
        for(slot = context->EffectSlotList;slot != NULL;slot = slot->next)
        {
            if(!slot->EffectState)
                continue;

            if(ALEffect_DeviceUpdate(slot->EffectState, device) == AL_FALSE)
            {
                alcSetError(device, ALC_INVALID_DEVICE);
                aluHandleDisconnect(device);
                ProcessContext(context);
                ProcessContext(NULL);
                ALCdevice_StopPlayback(device);
                return NULL;
            }
            ALEffect_Update(slot->EffectState, context, &slot->effect);
        }

        for(source = context->SourceList;source != NULL;source = source->next)
        {
            ALuint s = device->NumAuxSends;
            while(s < MAX_SENDS)
            {
                if(source->Send[s].Slot)
                    source->Send[s].Slot->refcount--;
                source->Send[s].Slot = NULL;
                source->Send[s].WetFilter.type = 0;
                source->Send[s].WetFilter.filter = 0;
                s++;
            }
            source->NeedsUpdate = AL_TRUE;
        }
        aluInitPanning(context);
        ProcessContext(context);
    }

    if(device->Bs2bLevel > 0 && device->Bs2bLevel <= 6)
    {
        if(!device->Bs2b)
        {
            device->Bs2b = calloc(1, sizeof(*device->Bs2b));
            bs2b_clear(device->Bs2b);
        }
        bs2b_set_srate(device->Bs2b, device->Frequency);
        bs2b_set_level(device->Bs2b, device->Bs2bLevel);
    }
    else
    {
        free(device->Bs2b);
        device->Bs2b = NULL;
    }

    temp = realloc(device->Contexts, (device->NumContexts+1) * sizeof(*device->Contexts));
    if(!temp)
    {
        alcSetError(device, ALC_OUT_OF_MEMORY);
        ProcessContext(NULL);
        return NULL;
    }
    device->Contexts = temp;

    ALContext = calloc(1, sizeof(ALCcontext));
    if(!ALContext)
    {
        alcSetError(device, ALC_OUT_OF_MEMORY);
        ProcessContext(NULL);
        return NULL;
    }

    device->Contexts[device->NumContexts++] = ALContext;
    ALContext->Device = device;

    InitContext(ALContext);
    aluInitPanning(ALContext);

    ALContext->next = g_pContextList;
    g_pContextList = ALContext;
    g_ulContextCount++;

    ProcessContext(NULL);

    return ALContext;
}


/*
    alcDestroyContext

    Remove a Context
*/
ALC_API ALCvoid ALC_APIENTRY alcDestroyContext(ALCcontext *context)
{
    ALCdevice *Device;
    ALCcontext **list;
    ALuint i;

    if(!IsContext(context))
    {
        alcSetError(NULL, ALC_INVALID_CONTEXT);
        return;
    }

    Device = context->Device;

    if(Device->NumContexts == 1)
        ALCdevice_StopPlayback(Device);

    SuspendContext(NULL);

    if(context == GlobalContext)
        GlobalContext = NULL;

    for(i = 0;i < Device->NumContexts-1;i++)
    {
        if(Device->Contexts[i] == context)
        {
            Device->Contexts[i] = Device->Contexts[Device->NumContexts-1];
            break;
        }
    }
    Device->NumContexts--;

    // Lock context
    SuspendContext(context);

    if(context->SourceCount > 0)
    {
#ifdef _DEBUG
        AL_PRINT("alcDestroyContext(): deleting %d Source(s)\n", context->SourceCount);
#endif
        ReleaseALSources(context);
    }
    if(context->EffectSlotCount > 0)
    {
#ifdef _DEBUG
        AL_PRINT("alcDestroyContext(): deleting %d AuxiliaryEffectSlot(s)\n", context->EffectSlotCount);
#endif
        ReleaseALAuxiliaryEffectSlots(context);
    }

    list = &g_pContextList;
    while(*list != context)
        list = &(*list)->next;

    *list = (*list)->next;
    g_ulContextCount--;

    // Unlock context
    ProcessContext(context);
    ProcessContext(NULL);

    ExitContext(context);

    // Free memory (MUST do this after ProcessContext)
    memset(context, 0, sizeof(ALCcontext));
    free(context);
}


/*
    alcGetCurrentContext

    Returns the currently active Context
*/
ALC_API ALCcontext* ALC_APIENTRY alcGetCurrentContext(ALCvoid)
{
    ALCcontext *pContext;

    if((pContext=GetContextSuspended()) != NULL)
        ProcessContext(pContext);

    return pContext;
}

/*
    alcGetThreadContext

    Returns the currently active thread-local Context
*/
ALC_API ALCcontext* ALC_APIENTRY alcGetThreadContext(void)
{
    ALCcontext *pContext = NULL;

    SuspendContext(NULL);

    pContext = tls_get(LocalContext);
    if(pContext && !IsContext(pContext))
    {
        tls_set(LocalContext, NULL);
        pContext = NULL;
    }

    ProcessContext(NULL);

    return pContext;
}


/*
    alcGetContextsDevice

    Returns the Device that a particular Context is attached to
*/
ALC_API ALCdevice* ALC_APIENTRY alcGetContextsDevice(ALCcontext *pContext)
{
    ALCdevice *pDevice = NULL;

    SuspendContext(NULL);
    if(IsContext(pContext))
        pDevice = pContext->Device;
    else
        alcSetError(NULL, ALC_INVALID_CONTEXT);
    ProcessContext(NULL);

    return pDevice;
}


/*
    alcMakeContextCurrent

    Makes the given Context the active Context
*/
ALC_API ALCboolean ALC_APIENTRY alcMakeContextCurrent(ALCcontext *context)
{
    ALboolean bReturn = AL_TRUE;

    SuspendContext(NULL);

    // context must be a valid Context or NULL
    if(context == NULL || IsContext(context))
    {
        GlobalContext = context;
        tls_set(LocalContext, NULL);
    }
    else
    {
        alcSetError(NULL, ALC_INVALID_CONTEXT);
        bReturn = AL_FALSE;
    }

    ProcessContext(NULL);

    return bReturn;
}

/*
    alcSetThreadContext

    Makes the given Context the active Context for the current thread
*/
ALC_API ALCboolean ALC_APIENTRY alcSetThreadContext(ALCcontext *context)
{
    ALboolean bReturn = AL_TRUE;

    SuspendContext(NULL);

    // context must be a valid Context or NULL
    if(context == NULL || IsContext(context))
        tls_set(LocalContext, context);
    else
    {
        alcSetError(NULL, ALC_INVALID_CONTEXT);
        bReturn = AL_FALSE;
    }

    ProcessContext(NULL);

    return bReturn;
}


// Sets the default channel order used by most non-WaveFormatEx-based APIs
void SetDefaultChannelOrder(ALCdevice *device)
{
    switch(aluChannelsFromFormat(device->Format))
    {
    case 1: device->DevChannels[0] = FRONT_CENTER; break;

    case 2: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT; break;

    case 4: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT;
            device->DevChannels[2] = BACK_LEFT;
            device->DevChannels[3] = BACK_RIGHT; break;

    case 6: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT;
            device->DevChannels[2] = BACK_LEFT;
            device->DevChannels[3] = BACK_RIGHT;
            device->DevChannels[4] = FRONT_CENTER;
            device->DevChannels[5] = LFE; break;

    case 7: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT;
            device->DevChannels[2] = FRONT_CENTER;
            device->DevChannels[3] = LFE;
            device->DevChannels[4] = BACK_CENTER;
            device->DevChannels[5] = SIDE_LEFT;
            device->DevChannels[6] = SIDE_RIGHT; break;

    case 8: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT;
            device->DevChannels[2] = BACK_LEFT;
            device->DevChannels[3] = BACK_RIGHT;
            device->DevChannels[4] = FRONT_CENTER;
            device->DevChannels[5] = LFE;
            device->DevChannels[6] = SIDE_LEFT;
            device->DevChannels[7] = SIDE_RIGHT; break;
    }
}
// Sets the default order used by WaveFormatEx
void SetDefaultWFXChannelOrder(ALCdevice *device)
{
    switch(aluChannelsFromFormat(device->Format))
    {
    case 1: device->DevChannels[0] = FRONT_CENTER; break;

    case 2: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT; break;

    case 4: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT;
            device->DevChannels[2] = BACK_LEFT;
            device->DevChannels[3] = BACK_RIGHT; break;

    case 6: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT;
            device->DevChannels[2] = FRONT_CENTER;
            device->DevChannels[3] = LFE;
            device->DevChannels[4] = BACK_LEFT;
            device->DevChannels[5] = BACK_RIGHT; break;

    case 7: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT;
            device->DevChannels[2] = FRONT_CENTER;
            device->DevChannels[3] = LFE;
            device->DevChannels[4] = BACK_CENTER;
            device->DevChannels[5] = SIDE_LEFT;
            device->DevChannels[6] = SIDE_RIGHT; break;

    case 8: device->DevChannels[0] = FRONT_LEFT;
            device->DevChannels[1] = FRONT_RIGHT;
            device->DevChannels[2] = FRONT_CENTER;
            device->DevChannels[3] = LFE;
            device->DevChannels[4] = BACK_LEFT;
            device->DevChannels[5] = BACK_RIGHT;
            device->DevChannels[6] = SIDE_LEFT;
            device->DevChannels[7] = SIDE_RIGHT; break;
    }
}

static ALenum GetFormatFromString(const char *str)
{
    if(strcasecmp(str, "AL_FORMAT_MONO32") == 0)    return AL_FORMAT_MONO_FLOAT32;
    if(strcasecmp(str, "AL_FORMAT_STEREO32") == 0)  return AL_FORMAT_STEREO_FLOAT32;
    if(strcasecmp(str, "AL_FORMAT_QUAD32") == 0)    return AL_FORMAT_QUAD32;
    if(strcasecmp(str, "AL_FORMAT_51CHN32") == 0)   return AL_FORMAT_51CHN32;
    if(strcasecmp(str, "AL_FORMAT_61CHN32") == 0)   return AL_FORMAT_61CHN32;
    if(strcasecmp(str, "AL_FORMAT_71CHN32") == 0)   return AL_FORMAT_71CHN32;

    if(strcasecmp(str, "AL_FORMAT_MONO16") == 0)    return AL_FORMAT_MONO16;
    if(strcasecmp(str, "AL_FORMAT_STEREO16") == 0)  return AL_FORMAT_STEREO16;
    if(strcasecmp(str, "AL_FORMAT_QUAD16") == 0)    return AL_FORMAT_QUAD16;
    if(strcasecmp(str, "AL_FORMAT_51CHN16") == 0)   return AL_FORMAT_51CHN16;
    if(strcasecmp(str, "AL_FORMAT_61CHN16") == 0)   return AL_FORMAT_61CHN16;
    if(strcasecmp(str, "AL_FORMAT_71CHN16") == 0)   return AL_FORMAT_71CHN16;

    if(strcasecmp(str, "AL_FORMAT_MONO8") == 0)    return AL_FORMAT_MONO8;
    if(strcasecmp(str, "AL_FORMAT_STEREO8") == 0)  return AL_FORMAT_STEREO8;
    if(strcasecmp(str, "AL_FORMAT_QUAD8") == 0)    return AL_FORMAT_QUAD8;
    if(strcasecmp(str, "AL_FORMAT_51CHN8") == 0)   return AL_FORMAT_51CHN8;
    if(strcasecmp(str, "AL_FORMAT_61CHN8") == 0)   return AL_FORMAT_61CHN8;
    if(strcasecmp(str, "AL_FORMAT_71CHN8") == 0)   return AL_FORMAT_71CHN8;

    AL_PRINT("Unknown format: \"%s\"\n", str);
    return AL_FORMAT_STEREO16;
}

/*
    alcOpenDevice

    Open the Device specified.
*/
ALC_API ALCdevice* ALC_APIENTRY alcOpenDevice(const ALCchar *deviceName)
{
    ALboolean bDeviceFound = AL_FALSE;
    const ALCchar *fmt;
    ALCdevice *device;
    ALint i;

    if(deviceName && !deviceName[0])
        deviceName = NULL;

    device = calloc(1, sizeof(ALCdevice));
    if(!device)
    {
        alcSetError(NULL, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    //Validate device
    device->Connected = ALC_TRUE;
    device->IsCaptureDevice = AL_FALSE;
    device->LastError = ALC_NO_ERROR;

    device->Bs2b = NULL;
    device->szDeviceName = NULL;

    device->Contexts = NULL;
    device->NumContexts = 0;

    //Set output format
    device->Frequency = GetConfigValueInt(NULL, "frequency", SWMIXER_OUTPUT_RATE);
    if(device->Frequency < 8000)
        device->Frequency = 8000;

    fmt = GetConfigValue(NULL, "format", "AL_FORMAT_STEREO16");
    device->Format = GetFormatFromString(fmt);

    device->NumUpdates = GetConfigValueInt(NULL, "periods", 4);
    if(device->NumUpdates < 2)
        device->NumUpdates = 4;

    i = GetConfigValueInt(NULL, "refresh", 4096);
    if(i <= 0) i = 4096;

    device->UpdateSize = GetConfigValueInt(NULL, "period_size", i/device->NumUpdates);
    if(device->UpdateSize <= 0)
        device->UpdateSize = i/device->NumUpdates;

    device->MaxNoOfSources = GetConfigValueInt(NULL, "sources", 256);
    if((ALint)device->MaxNoOfSources <= 0)
        device->MaxNoOfSources = 256;

    device->AuxiliaryEffectSlotMax = GetConfigValueInt(NULL, "slots", 4);
    if((ALint)device->AuxiliaryEffectSlotMax <= 0)
        device->AuxiliaryEffectSlotMax = 4;

    device->NumStereoSources = 1;
    device->NumMonoSources = device->MaxNoOfSources - device->NumStereoSources;

    device->NumAuxSends = GetConfigValueInt(NULL, "sends", MAX_SENDS);
    if(device->NumAuxSends > MAX_SENDS)
        device->NumAuxSends = MAX_SENDS;

    device->Bs2bLevel = GetConfigValueInt(NULL, "cf_level", 0);

    if(aluChannelsFromFormat(device->Format) <= 2)
    {
        device->HeadDampen = GetConfigValueFloat(NULL, "head_dampen", DEFAULT_HEAD_DAMPEN);
        device->HeadDampen = __min(device->HeadDampen, 1.0f);
        device->HeadDampen = __max(device->HeadDampen, 0.0f);
    }
    else
        device->HeadDampen = 0.0f;

    // Find a playback device to open
    SuspendContext(NULL);
    for(i = 0;BackendList[i].Init;i++)
    {
        device->Funcs = &BackendList[i].Funcs;
        if(ALCdevice_OpenPlayback(device, deviceName))
        {
            device->next = g_pDeviceList;
            g_pDeviceList = device;
            g_ulDeviceCount++;

            bDeviceFound = AL_TRUE;
            break;
        }
    }
    ProcessContext(NULL);

    if(!bDeviceFound)
    {
        // No suitable output device found
        alcSetError(NULL, ALC_INVALID_VALUE);
        free(device);
        device = NULL;
    }

    return device;
}


/*
    alcCloseDevice

    Close the specified Device
*/
ALC_API ALCboolean ALC_APIENTRY alcCloseDevice(ALCdevice *pDevice)
{
    ALCdevice **list;

    if(!IsDevice(pDevice) || pDevice->IsCaptureDevice)
    {
        alcSetError(pDevice, ALC_INVALID_DEVICE);
        return ALC_FALSE;
    }

    SuspendContext(NULL);

    list = &g_pDeviceList;
    while(*list != pDevice)
        list = &(*list)->next;

    *list = (*list)->next;
    g_ulDeviceCount--;

    ProcessContext(NULL);

    if(pDevice->NumContexts > 0)
    {
#ifdef _DEBUG
        AL_PRINT("alcCloseDevice(): destroying %u Context(s)\n", pDevice->NumContexts);
#endif
        while(pDevice->NumContexts > 0)
            alcDestroyContext(pDevice->Contexts[0]);
    }
    ALCdevice_ClosePlayback(pDevice);

    if(pDevice->BufferCount > 0)
    {
#ifdef _DEBUG
        AL_PRINT("alcCloseDevice(): deleting %d Buffer(s)\n", pDevice->BufferCount);
#endif
        ReleaseALBuffers(pDevice);
    }
    if(pDevice->EffectCount > 0)
    {
#ifdef _DEBUG
        AL_PRINT("alcCloseDevice(): deleting %d Effect(s)\n", pDevice->EffectCount);
#endif
        ReleaseALEffects(pDevice);
    }
    if(pDevice->FilterCount > 0)
    {
#ifdef _DEBUG
        AL_PRINT("alcCloseDevice(): deleting %d Filter(s)\n", pDevice->FilterCount);
#endif
        ReleaseALFilters(pDevice);
    }
    if(pDevice->DatabufferCount > 0)
    {
#ifdef _DEBUG
        AL_PRINT("alcCloseDevice(): deleting %d Databuffer(s)\n", pDevice->DatabufferCount);
#endif
        ReleaseALDatabuffers(pDevice);
    }

    free(pDevice->Bs2b);
    pDevice->Bs2b = NULL;

    free(pDevice->szDeviceName);
    pDevice->szDeviceName = NULL;

    free(pDevice->Contexts);
    pDevice->Contexts = NULL;

    //Release device structure
    memset(pDevice, 0, sizeof(ALCdevice));
    free(pDevice);

    return ALC_TRUE;
}


ALCvoid ReleaseALC(ALCvoid)
{
    free(alcDeviceList); alcDeviceList = NULL;
    alcDeviceListSize = 0;
    free(alcAllDeviceList); alcAllDeviceList = NULL;
    alcAllDeviceListSize = 0;
    free(alcCaptureDeviceList); alcCaptureDeviceList = NULL;
    alcCaptureDeviceListSize = 0;

    free(alcDefaultDeviceSpecifier);
    alcDefaultDeviceSpecifier = NULL;
    free(alcDefaultAllDeviceSpecifier);
    alcDefaultAllDeviceSpecifier = NULL;
    free(alcCaptureDefaultDeviceSpecifier);
    alcCaptureDefaultDeviceSpecifier = NULL;

#ifdef _DEBUG
    if(g_ulDeviceCount > 0)
        AL_PRINT("exit(): closing %u Device%s\n", g_ulDeviceCount, (g_ulDeviceCount>1)?"s":"");
#endif

    while(g_pDeviceList)
    {
        if(g_pDeviceList->IsCaptureDevice)
            alcCaptureCloseDevice(g_pDeviceList);
        else
            alcCloseDevice(g_pDeviceList);
    }
}

///////////////////////////////////////////////////////
