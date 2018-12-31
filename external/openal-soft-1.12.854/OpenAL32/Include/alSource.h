#ifndef _AL_SOURCE_H_
#define _AL_SOURCE_H_

#define MAX_SENDS                 2

#include "alFilter.h"
#include "alu.h"
#include "AL/al.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    POINT_RESAMPLER = 0,
    LINEAR_RESAMPLER,
    COSINE_RESAMPLER,

    RESAMPLER_MAX,
    RESAMPLER_MIN = -1,
    RESAMPLER_DEFAULT = LINEAR_RESAMPLER
} resampler_t;
extern resampler_t DefaultResampler;

typedef struct ALbufferlistitem
{
    struct ALbuffer         *buffer;
    struct ALbufferlistitem *next;
} ALbufferlistitem;

typedef struct ALsource
{
    ALfloat      flPitch;
    ALfloat      flGain;
    ALfloat      flOuterGain;
    ALfloat      flMinGain;
    ALfloat      flMaxGain;
    ALfloat      flInnerAngle;
    ALfloat      flOuterAngle;
    ALfloat      flRefDistance;
    ALfloat      flMaxDistance;
    ALfloat      flRollOffFactor;
    ALfloat      vPosition[3];
    ALfloat      vVelocity[3];
    ALfloat      vOrientation[3];
    ALboolean    bHeadRelative;
    ALboolean    bLooping;
    ALenum       DistanceModel;

    resampler_t  Resampler;

    ALenum       state;
    ALuint       position;
    ALuint       position_fraction;

    struct ALbuffer *Buffer;

    struct ALbufferlistitem *queue; // Linked list of buffers in queue
    ALuint       BuffersInQueue;    // Number of buffers in queue
    ALuint       BuffersPlayed;     // Number of buffers played on this loop

    ALfilter DirectFilter;

    struct {
        struct ALeffectslot *Slot;
        ALfilter WetFilter;
    } Send[MAX_SENDS];

    ALboolean DryGainHFAuto;
    ALboolean WetGainAuto;
    ALboolean WetGainHFAuto;
    ALfloat   OuterGainHF;

    ALfloat AirAbsorptionFactor;
    ALfloat RoomRolloffFactor;
    ALfloat DopplerFactor;

    ALint  lOffset;
    ALint  lOffsetType;

    // Source Type (Static, Streaming, or Undetermined)
    ALint  lSourceType;

    // Current gains, which are ramped while mixed
    ALfloat DryGains[OUTPUTCHANNELS];
    ALfloat WetGains[MAX_SENDS];
    ALboolean FirstStart;

    // Current target parameters used for mixing
    ALboolean NeedsUpdate;
    struct {
        ALfloat DryGains[OUTPUTCHANNELS];
        ALfloat WetGains[MAX_SENDS];
        ALfloat Pitch;

        struct {
            FILTER iirFilter;
            ALfloat history[OUTPUTCHANNELS];
        } Send[MAX_SENDS];

        FILTER iirFilter;
        ALfloat history[OUTPUTCHANNELS*2];
    } Params;

    // Index to itself
    ALuint source;

    struct ALsource *next;
} ALsource;

ALvoid ReleaseALSources(ALCcontext *Context);

#ifdef __cplusplus
}
#endif

#endif
