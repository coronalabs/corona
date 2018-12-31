#ifndef _ALU_H_
#define _ALU_H_

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include <math.h>
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif

#ifdef HAVE_POWF
#define aluPow(x,y) ((ALfloat)powf((float)(x),(float)(y)))
#else
#define aluPow(x,y) ((ALfloat)pow((double)(x),(double)(y)))
#endif

#ifdef HAVE_SQRTF
#define aluSqrt(x) ((ALfloat)sqrtf((float)(x)))
#else
#define aluSqrt(x) ((ALfloat)sqrt((double)(x)))
#endif

#ifdef HAVE_ACOSF
#define aluAcos(x) ((ALfloat)acosf((float)(x)))
#else
#define aluAcos(x) ((ALfloat)acos((double)(x)))
#endif

#ifdef HAVE_ATANF
#define aluAtan(x) ((ALfloat)atanf((float)(x)))
#else
#define aluAtan(x) ((ALfloat)atan((double)(x)))
#endif

#ifdef HAVE_FABSF
#define aluFabs(x) ((ALfloat)fabsf((float)(x)))
#else
#define aluFabs(x) ((ALfloat)fabs((double)(x)))
#endif

// fixes for mingw32.
#if defined(max) && !defined(__max)
#define __max max
#endif
#if defined(min) && !defined(__min)
#define __min min
#endif

#define QUADRANT_NUM  128
#define LUT_NUM       (4 * QUADRANT_NUM)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FRONT_LEFT = 0,
    FRONT_RIGHT,
    FRONT_CENTER,
    LFE,
    BACK_LEFT,
    BACK_RIGHT,
    BACK_CENTER,
    SIDE_LEFT,
    SIDE_RIGHT,

    OUTPUTCHANNELS
} Channel;

#define BUFFERSIZE 16384

extern ALboolean DuplicateStereo;

/* NOTE: The AL_FORMAT_REAR* enums aren't handled here be cause they're
 *       converted to AL_FORMAT_QUAD* when loaded */
static __inline ALuint aluBytesFromFormat(ALenum format)
{
    switch(format)
    {
        case AL_FORMAT_MONO8:
        case AL_FORMAT_STEREO8:
        case AL_FORMAT_QUAD8_LOKI:
        case AL_FORMAT_QUAD8:
        case AL_FORMAT_51CHN8:
        case AL_FORMAT_61CHN8:
        case AL_FORMAT_71CHN8:
            return 1;

        case AL_FORMAT_MONO16:
        case AL_FORMAT_STEREO16:
        case AL_FORMAT_QUAD16_LOKI:
        case AL_FORMAT_QUAD16:
        case AL_FORMAT_51CHN16:
        case AL_FORMAT_61CHN16:
        case AL_FORMAT_71CHN16:
            return 2;

        case AL_FORMAT_MONO_FLOAT32:
        case AL_FORMAT_STEREO_FLOAT32:
        case AL_FORMAT_QUAD32:
        case AL_FORMAT_51CHN32:
        case AL_FORMAT_61CHN32:
        case AL_FORMAT_71CHN32:
            return 4;

        case AL_FORMAT_MONO_DOUBLE_EXT:
        case AL_FORMAT_STEREO_DOUBLE_EXT:
            return 8;

        default:
            return 0;
    }
}
static __inline ALuint aluChannelsFromFormat(ALenum format)
{
    switch(format)
    {
        case AL_FORMAT_MONO8:
        case AL_FORMAT_MONO16:
        case AL_FORMAT_MONO_FLOAT32:
        case AL_FORMAT_MONO_DOUBLE_EXT:
            return 1;

        case AL_FORMAT_STEREO8:
        case AL_FORMAT_STEREO16:
        case AL_FORMAT_STEREO_FLOAT32:
        case AL_FORMAT_STEREO_DOUBLE_EXT:
            return 2;

        case AL_FORMAT_QUAD8_LOKI:
        case AL_FORMAT_QUAD16_LOKI:
        case AL_FORMAT_QUAD8:
        case AL_FORMAT_QUAD16:
        case AL_FORMAT_QUAD32:
            return 4;

        case AL_FORMAT_51CHN8:
        case AL_FORMAT_51CHN16:
        case AL_FORMAT_51CHN32:
            return 6;

        case AL_FORMAT_61CHN8:
        case AL_FORMAT_61CHN16:
        case AL_FORMAT_61CHN32:
            return 7;

        case AL_FORMAT_71CHN8:
        case AL_FORMAT_71CHN16:
        case AL_FORMAT_71CHN32:
            return 8;

        default:
            return 0;
    }
}

static __inline ALint aluCart2LUTpos(ALfloat re, ALfloat im)
{
    ALint pos = 0;
    ALfloat denom = aluFabs(re) + aluFabs(im);
    if(denom > 0.0f)
        pos = (ALint)(QUADRANT_NUM*aluFabs(im) / denom + 0.5);

    if(re < 0.0)
        pos = 2 * QUADRANT_NUM - pos;
    if(im < 0.0)
        pos = LUT_NUM - pos;
    return pos%LUT_NUM;
}

ALvoid aluInitPanning(ALCcontext *Context);
ALvoid aluMixData(ALCdevice *device, ALvoid *buffer, ALsizei size);
ALvoid aluHandleDisconnect(ALCdevice *device);

#ifdef __cplusplus
}
#endif

#endif

