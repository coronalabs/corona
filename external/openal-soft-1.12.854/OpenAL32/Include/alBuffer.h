#ifndef _AL_BUFFER_H_
#define _AL_BUFFER_H_

#include "AL/al.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUFFER_PADDING 2

typedef struct ALbuffer
{
    ALfloat *data;
    ALsizei  size;

    ALenum   format;
    ALenum   eOriginalFormat;
    ALsizei  frequency;

    ALuint   refcount; // Number of sources using this buffer (deletion can only occur when this is 0)

    // Index to itself
    ALuint buffer;

    struct ALbuffer *next;
} ALbuffer;

ALvoid ReleaseALBuffers(ALCdevice *device);

#ifdef __cplusplus
}
#endif

#endif
