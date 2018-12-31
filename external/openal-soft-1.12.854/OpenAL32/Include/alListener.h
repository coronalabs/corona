#ifndef _AL_LISTENER_H_
#define _AL_LISTENER_H_

#include "AL/al.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALlistener_struct
{
    ALfloat Position[3];
    ALfloat Velocity[3];
    ALfloat Forward[3];
    ALfloat Up[3];
    ALfloat Gain;
    ALfloat MetersPerUnit;
} ALlistener;

#ifdef __cplusplus
}
#endif

#endif
