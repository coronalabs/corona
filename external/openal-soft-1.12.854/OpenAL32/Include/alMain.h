#ifndef AL_MAIN_H
#define AL_MAIN_H

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef HAVE_FENV_H
#include <fenv.h>
#endif

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#if defined(HAVE_STDINT_H)
#include <stdint.h>
typedef int64_t ALint64;
typedef uint64_t ALuint64;
#elif defined(HAVE___INT64)
typedef __int64 ALint64;
typedef unsigned __int64 ALuint64;
#elif (SIZEOF_LONG == 8)
typedef long ALint64;
typedef unsigned long ALuint64;
#elif (SIZEOF_LONG_LONG == 8)
typedef long long ALint64;
typedef unsigned long long ALuint64;
#endif

#ifdef HAVE_GCC_FORMAT
#define PRINTF_STYLE(x, y) __attribute__((format(printf, (x), (y))))
#else
#define PRINTF_STYLE(x, y)
#endif

#ifdef _WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>

typedef DWORD tls_type;
#define tls_create(x) (*(x) = TlsAlloc())
#define tls_delete(x) TlsFree((x))
#define tls_get(x) TlsGetValue((x))
#define tls_set(x, a) TlsSetValue((x), (a))

#else

#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#define IsBadWritePtr(a,b) ((a) == NULL && (b) != 0)

typedef pthread_key_t tls_type;
#define tls_create(x) pthread_key_create((x), NULL)
#define tls_delete(x) pthread_key_delete((x))
#define tls_get(x) pthread_getspecific((x))
#define tls_set(x, a) pthread_setspecific((x), (a))

typedef pthread_mutex_t CRITICAL_SECTION;
static inline void EnterCriticalSection(CRITICAL_SECTION *cs)
{
    int ret;
    ret = pthread_mutex_lock(cs);
    assert(ret == 0);
}
static inline void LeaveCriticalSection(CRITICAL_SECTION *cs)
{
    int ret;
    ret = pthread_mutex_unlock(cs);
    assert(ret == 0);
}
static inline void InitializeCriticalSection(CRITICAL_SECTION *cs)
{
    pthread_mutexattr_t attrib;
    int ret;

    ret = pthread_mutexattr_init(&attrib);
    assert(ret == 0);

    ret = pthread_mutexattr_settype(&attrib, PTHREAD_MUTEX_RECURSIVE);
#ifdef HAVE_PTHREAD_NP_H
    if(ret != 0)
        ret = pthread_mutexattr_setkind_np(&attrib, PTHREAD_MUTEX_RECURSIVE);
#endif
    assert(ret == 0);
    ret = pthread_mutex_init(cs, &attrib);
    assert(ret == 0);

    pthread_mutexattr_destroy(&attrib);
}

static inline void DeleteCriticalSection(CRITICAL_SECTION *cs)
{
    int ret;
    ret = pthread_mutex_destroy(cs);
    assert(ret == 0);
}

/* NOTE: This wrapper isn't quite accurate as it returns an ALuint, as opposed
 * to the expected DWORD. Both are defined as unsigned 32-bit types, however.
 * Additionally, Win32 is supposed to measure the time since Windows started,
 * as opposed to the actual time. */
static inline ALuint timeGetTime(void)
{
    int ret;
#if _POSIX_TIMERS > 0
    struct timespec ts;

    ret = clock_gettime(CLOCK_REALTIME, &ts);
    assert(ret == 0);

    return ts.tv_nsec/1000000 + ts.tv_sec*1000;
#else
    struct timeval tv;

    ret = gettimeofday(&tv, NULL);
    assert(ret == 0);

    return tv.tv_usec/1000 + tv.tv_sec*1000;
#endif
}

static inline void Sleep(ALuint t)
{
    struct timespec tv, rem;
    tv.tv_nsec = (t*1000000)%1000000000;
    tv.tv_sec = t/1000;

    while(nanosleep(&tv, &rem) == -1 && errno == EINTR)
        tv = rem;
}
#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))
#endif

#include "alListener.h"
#include "alu.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SWMIXER_OUTPUT_RATE        44100

#define SPEEDOFSOUNDMETRESPERSEC   (343.3f)
#define AIRABSORBGAINDBHF          (-0.05f)

#define LOWPASSFREQCUTOFF          (5000)

#define DEFAULT_HEAD_DAMPEN        (0.25f)


// Find the next power-of-2 for non-power-of-2 numbers.
static __inline ALuint NextPowerOf2(ALuint value)
{
    ALuint powerOf2 = 1;

    if(value)
    {
        value--;
        while(value)
        {
            value >>= 1;
            powerOf2 <<= 1;
        }
    }
    return powerOf2;
}


typedef struct {
    ALCboolean (*OpenPlayback)(ALCdevice*, const ALCchar*);
    void (*ClosePlayback)(ALCdevice*);
    ALCboolean (*ResetPlayback)(ALCdevice*);
    void (*StopPlayback)(ALCdevice*);

    ALCboolean (*OpenCapture)(ALCdevice*, const ALCchar*);
    void (*CloseCapture)(ALCdevice*);
    void (*StartCapture)(ALCdevice*);
    void (*StopCapture)(ALCdevice*);
    void (*CaptureSamples)(ALCdevice*, void*, ALCuint);
    ALCuint (*AvailableSamples)(ALCdevice*);
} BackendFuncs;

enum {
    DEVICE_PROBE,
    ALL_DEVICE_PROBE,
    CAPTURE_DEVICE_PROBE
};

void alc_alsa_init(BackendFuncs *func_list);
void alc_alsa_deinit(void);
void alc_alsa_probe(int type);
void alc_oss_init(BackendFuncs *func_list);
void alc_oss_deinit(void);
void alc_oss_probe(int type);
void alc_solaris_init(BackendFuncs *func_list);
void alc_solaris_deinit(void);
void alc_solaris_probe(int type);
void alcDSoundInit(BackendFuncs *func_list);
void alcDSoundDeinit(void);
void alcDSoundProbe(int type);
void alcWinMMInit(BackendFuncs *FuncList);
void alcWinMMDeinit(void);
void alcWinMMProbe(int type);
void alc_pa_init(BackendFuncs *func_list);
void alc_pa_deinit(void);
void alc_pa_probe(int type);
void alc_wave_init(BackendFuncs *func_list);
void alc_wave_deinit(void);
void alc_wave_probe(int type);
void alc_pulse_init(BackendFuncs *func_list);
void alc_pulse_deinit(void);
void alc_pulse_probe(int type);


struct ALCdevice_struct
{
    ALCboolean   Connected;
    ALboolean    IsCaptureDevice;

    ALuint       Frequency;
    ALuint       UpdateSize;
    ALuint       NumUpdates;
    ALenum       Format;

    ALCchar      *szDeviceName;

    ALCenum      LastError;

    // Maximum number of sources that can be created
    ALuint       MaxNoOfSources;
    // Maximum number of slots that can be created
    ALuint       AuxiliaryEffectSlotMax;

    ALCuint      NumMonoSources;
    ALCuint      NumStereoSources;
    ALuint       NumAuxSends;

    // Linked List of Buffers for this device
    struct ALbuffer *BufferList;
    ALuint           BufferCount;

    // Linked List of Effects for this device
    struct ALeffect *EffectList;
    ALuint           EffectCount;

    // Linked List of Filters for this device
    struct ALfilter *FilterList;
    ALuint           FilterCount;

    // Linked List of Databuffers for this device
    struct ALdatabuffer *DatabufferList;
    ALuint               DatabufferCount;

    // Stereo-to-binaural filter
    struct bs2b *Bs2b;
    ALCint       Bs2bLevel;

    // Simulated dampening from head occlusion
    ALfloat      HeadDampen;

    // Dry path buffer mix
    float DryBuffer[BUFFERSIZE][OUTPUTCHANNELS];

    Channel DevChannels[OUTPUTCHANNELS];

    // Contexts created on this device
    ALCcontext  **Contexts;
    ALuint        NumContexts;

    BackendFuncs *Funcs;
    void         *ExtraData; // For the backend's use

    ALCdevice *next;
};

#define ALCdevice_OpenPlayback(a,b)      ((a)->Funcs->OpenPlayback((a), (b)))
#define ALCdevice_ClosePlayback(a)       ((a)->Funcs->ClosePlayback((a)))
#define ALCdevice_ResetPlayback(a)       ((a)->Funcs->ResetPlayback((a)))
#define ALCdevice_StopPlayback(a)        ((a)->Funcs->StopPlayback((a)))
#define ALCdevice_OpenCapture(a,b)       ((a)->Funcs->OpenCapture((a), (b)))
#define ALCdevice_CloseCapture(a)        ((a)->Funcs->CloseCapture((a)))
#define ALCdevice_StartCapture(a)        ((a)->Funcs->StartCapture((a)))
#define ALCdevice_StopCapture(a)         ((a)->Funcs->StopCapture((a)))
#define ALCdevice_CaptureSamples(a,b,c)  ((a)->Funcs->CaptureSamples((a), (b), (c)))
#define ALCdevice_AvailableSamples(a)    ((a)->Funcs->AvailableSamples((a)))

struct ALCcontext_struct
{
    ALlistener  Listener;

    struct ALsource *SourceList;
    ALuint           SourceCount;

    struct ALeffectslot *EffectSlotList;
    ALuint               EffectSlotCount;

    struct ALdatabuffer *SampleSource;
    struct ALdatabuffer *SampleSink;

    ALenum      LastError;

    ALboolean   Suspended;

    ALenum      DistanceModel;
    ALboolean   SourceDistanceModel;

    ALfloat     DopplerFactor;
    ALfloat     DopplerVelocity;
    ALfloat     flSpeedOfSound;

    ALfloat     PanningLUT[OUTPUTCHANNELS * LUT_NUM];
    ALint       NumChan;

    ALfloat     ChannelMatrix[OUTPUTCHANNELS][OUTPUTCHANNELS];

    ALCdevice  *Device;
    const ALCchar *ExtensionList;

    ALCcontext *next;
};

extern ALint RTPrioLevel;

ALCvoid ReleaseALC(ALCvoid);

void AppendDeviceList(const ALCchar *name);
void AppendAllDeviceList(const ALCchar *name);
void AppendCaptureDeviceList(const ALCchar *name);

ALCvoid alcSetError(ALCdevice *device, ALenum errorCode);

ALCvoid SuspendContext(ALCcontext *context);
ALCvoid ProcessContext(ALCcontext *context);

ALvoid *StartThread(ALuint (*func)(ALvoid*), ALvoid *ptr);
ALuint StopThread(ALvoid *thread);

ALCcontext *GetContextSuspended(void);

typedef struct RingBuffer RingBuffer;
RingBuffer *CreateRingBuffer(ALsizei frame_size, ALsizei length);
void DestroyRingBuffer(RingBuffer *ring);
ALsizei RingBufferSize(RingBuffer *ring);
void WriteRingBuffer(RingBuffer *ring, const ALubyte *data, ALsizei len);
void ReadRingBuffer(RingBuffer *ring, ALubyte *data, ALsizei len);

void ReadALConfig(void);
void FreeALConfig(void);
int ConfigValueExists(const char *blockName, const char *keyName);
const char *GetConfigValue(const char *blockName, const char *keyName, const char *def);
int GetConfigValueInt(const char *blockName, const char *keyName, int def);
float GetConfigValueFloat(const char *blockName, const char *keyName, float def);
int GetConfigValueBool(const char *blockName, const char *keyName, int def);

void EnableRTPrio(ALint level);

void SetDefaultChannelOrder(ALCdevice *device);
void SetDefaultWFXChannelOrder(ALCdevice *device);

void al_print(const char *fname, unsigned int line, const char *fmt, ...)
             PRINTF_STYLE(3,4);
#define AL_PRINT(...) al_print(__FILE__, __LINE__, __VA_ARGS__)

#define DECL_VERIFIER(name, type, field)                                      \
static type* Verify##name(type *list, ALuint id)                              \
{                                                                             \
    while(list && list->field != id)                                          \
        list = list->next;                                                    \
    return list;                                                              \
}

#ifdef __cplusplus
}
#endif

#endif
