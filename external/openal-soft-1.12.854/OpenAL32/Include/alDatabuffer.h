#ifndef _AL_DATABUFFER_H_
#define _AL_DATABUFFER_H_

#include "AL/al.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UNMAPPED 0
#define MAPPED   1

typedef struct ALdatabuffer
{
    ALubyte     *data;
    ALintptrEXT size;

    ALenum state;
    ALenum usage;

    /* Index to self */
    ALuint databuffer;

    struct ALdatabuffer *next;
} ALdatabuffer;

ALvoid ReleaseALDatabuffers(ALCdevice *device);

#ifdef __cplusplus
}
#endif

#endif
