#ifndef _AL_FILTER_H_
#define _AL_FILTER_H_

#include "AL/al.h"
#include "alu.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ALfloat coeff;
#ifndef _MSC_VER
    ALfloat history[0];
#else
    ALfloat history[1];
#endif
} FILTER;

static __inline ALfloat lpFilter4P(FILTER *iir, ALuint offset, ALfloat input)
{
    ALfloat *history = &iir->history[offset];
    ALfloat a = iir->coeff;
    ALfloat output = input;

    output = output + (history[0]-output)*a;
    history[0] = output;
    output = output + (history[1]-output)*a;
    history[1] = output;
    output = output + (history[2]-output)*a;
    history[2] = output;
    output = output + (history[3]-output)*a;
    history[3] = output;

    return output;
}

static __inline ALfloat lpFilter2P(FILTER *iir, ALuint offset, ALfloat input)
{
    ALfloat *history = &iir->history[offset];
    ALfloat a = iir->coeff;
    ALfloat output = input;

    output = output + (history[0]-output)*a;
    history[0] = output;
    output = output + (history[1]-output)*a;
    history[1] = output;

    return output;
}

static __inline ALfloat lpFilter1P(FILTER *iir, ALuint offset, ALfloat input)
{
    ALfloat *history = &iir->history[offset];
    ALfloat a = iir->coeff;
    ALfloat output = input;

    output = output + (history[0]-output)*a;
    history[0] = output;

    return output;
}

/* Calculates the low-pass filter coefficient given the pre-scaled gain and
 * cos(w) value. Note that g should be pre-scaled (sqr(gain) for one-pole,
 * sqrt(gain) for four-pole, etc) */
static __inline ALfloat lpCoeffCalc(ALfloat g, ALfloat cw)
{
    ALfloat a = 0.0f;

    /* Be careful with gains < 0.01, as that causes the coefficient
     * head towards 1, which will flatten the signal */
    g = __max(g, 0.01f);
    if(g < 0.9999f) /* 1-epsilon */
        a = (1 - g*cw - aluSqrt(2*g*(1-cw) - g*g*(1 - cw*cw))) /
            (1 - g);

    return a;
}


typedef struct ALfilter
{
    // Filter type (AL_FILTER_NULL, ...)
    ALenum type;

    ALfloat Gain;
    ALfloat GainHF;

    // Index to itself
    ALuint filter;

    struct ALfilter *next;
} ALfilter;


ALvoid ReleaseALFilters(ALCdevice *device);

#ifdef __cplusplus
}
#endif

#endif
